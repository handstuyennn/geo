/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/overlay/PolygonBuilder.java rev. 1.20 (JTS-1.10)
 *
 **********************************************************************/

#include <cassert>
#include <geos/algorithm/PointLocation.hpp>
#include <geos/geom/CoordinateArraySequence.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geomgraph/DirectedEdgeStar.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/geomgraph/NodeMap.hpp>
#include <geos/operation/overlay/MaximalEdgeRing.hpp>
#include <geos/operation/overlay/MinimalEdgeRing.hpp>
#include <geos/operation/overlay/OverlayOp.hpp>
#include <geos/operation/overlay/PolygonBuilder.hpp>
#include <geos/operation/polygonize/EdgeRing.hpp>
#include <geos/util.hpp>
#include <geos/util/GEOSException.hpp>
#include <geos/util/TopologyException.hpp>
#include <vector>
#include <typeinfo>

using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay {   // geos.operation.overlay

PolygonBuilder::PolygonBuilder(const GeometryFactory *newGeometryFactory) : geometryFactory(newGeometryFactory) {
}

PolygonBuilder::~PolygonBuilder() {
	for (std::size_t i = 0, n = shellList.size(); i < n; ++i) {
		delete shellList[i];
	}
}

/*public*/
std::vector<Geometry *> *PolygonBuilder::getPolygons() {
	std::vector<Geometry *> *resultPolyList = computePolygons(shellList);
	return resultPolyList;
}

/*private*/
std::vector<Geometry *> *PolygonBuilder::computePolygons(std::vector<EdgeRing *> &newShellList) {
	std::vector<Geometry *> *resultPolyList = new std::vector<Geometry *>();

	// add Polygons for all shells
	for (std::size_t i = 0, n = newShellList.size(); i < n; i++) {
		EdgeRing *er = newShellList[i];
		Polygon *poly = er->toPolygon(geometryFactory).release();
		resultPolyList->push_back(poly);
	}
	return resultPolyList;
}

/*public*/
void PolygonBuilder::add(PlanarGraph *graph)
// throw(TopologyException *)
{
	const std::vector<EdgeEnd *> *eeptr = graph->getEdgeEnds();
	assert(eeptr);
	const std::vector<EdgeEnd *> &ee = *eeptr;

	std::size_t eeSize = ee.size();

	std::vector<DirectedEdge *> dirEdges(eeSize);
	for (std::size_t i = 0; i < eeSize; ++i) {
		DirectedEdge *de = detail::down_cast<DirectedEdge *>(ee[i]);
		dirEdges[i] = de;
	}

	const auto &nodeMap = graph->getNodeMap()->nodeMap;
	std::vector<Node *> nodes;
	nodes.reserve(nodeMap.size());
	for (const auto &nodeIt : nodeMap) {
		Node *node = nodeIt.second.get();
		nodes.push_back(node);
	}

	add(&dirEdges, &nodes); // might throw a TopologyException *
}

/*public*/
void PolygonBuilder::add(const std::vector<DirectedEdge *> *dirEdges, const std::vector<Node *> *nodes)
// throw(TopologyException *)
{
	PlanarGraph::linkResultDirectedEdges(nodes->begin(), nodes->end());

	std::vector<MaximalEdgeRing *> maxEdgeRings;
	buildMaximalEdgeRings(dirEdges, maxEdgeRings);

	std::vector<EdgeRing *> freeHoleList;
	std::vector<MaximalEdgeRing *> edgeRings;
	buildMinimalEdgeRings(maxEdgeRings, shellList, freeHoleList, edgeRings);

	sortShellsAndHoles(edgeRings, shellList, freeHoleList);

	std::vector<FastPIPRing> indexedshellist;
	for (auto const &shell : shellList) {
		FastPIPRing pipRing {shell, new geos::algorithm::locate::IndexedPointInAreaLocator(*shell->getLinearRing())};
		indexedshellist.push_back(pipRing);
	}
	placeFreeHoles(indexedshellist, freeHoleList);
	// Assert: every hole on freeHoleList has a shell assigned to it

	for (auto const &shell : indexedshellist) {
		delete shell.pipLocator;
	}
}

/*private*/
void PolygonBuilder::placeFreeHoles(std::vector<FastPIPRing> &newShellList, std::vector<EdgeRing *> &freeHoleList) {
	for (std::vector<EdgeRing *>::iterator it = freeHoleList.begin(), itEnd = freeHoleList.end(); it != itEnd; ++it) {
		EdgeRing *hole = *it;
		// only place this hole if it doesn't yet have a shell
		if (hole->getShell() == nullptr) {
			EdgeRing *shell = findEdgeRingContaining(hole, newShellList);
			if (shell == nullptr) {
				// assert(shell!=NULL); // unable to assign hole to a shell
				throw util::TopologyException("unable to assign hole to a shell");
			}

			hole->setShell(shell);
		}
	}
}

/*private*/
void PolygonBuilder::sortShellsAndHoles(std::vector<MaximalEdgeRing *> &edgeRings,
                                        std::vector<EdgeRing *> &newShellList, std::vector<EdgeRing *> &freeHoleList) {
	for (std::size_t i = 0, n = edgeRings.size(); i < n; i++) {
		EdgeRing *er = edgeRings[i];
		// er->setInResult();
		if (er->isHole()) {
			freeHoleList.push_back(er);
		} else {
			newShellList.push_back(er);
		}
	}
}

/*private*/
void PolygonBuilder::buildMinimalEdgeRings(std::vector<MaximalEdgeRing *> &maxEdgeRings,
                                           std::vector<EdgeRing *> &newShellList, std::vector<EdgeRing *> &freeHoleList,
                                           std::vector<MaximalEdgeRing *> &edgeRings) {
	for (std::size_t i = 0, n = maxEdgeRings.size(); i < n; ++i) {
		MaximalEdgeRing *er = maxEdgeRings[i];
		if (er->getMaxNodeDegree() > 2) {
			er->linkDirectedEdgesForMinimalEdgeRings();
			std::vector<MinimalEdgeRing *> minEdgeRings;
			er->buildMinimalRings(minEdgeRings);
			// at this point we can go ahead and attempt to place
			// holes, if this EdgeRing is a polygon
			EdgeRing *shell = findShell(&minEdgeRings);
			if (shell != nullptr) {
				placePolygonHoles(shell, &minEdgeRings);
				newShellList.push_back(shell);
			} else {
				freeHoleList.insert(freeHoleList.end(), minEdgeRings.begin(), minEdgeRings.end());
			}
			delete er;
		} else {
			edgeRings.push_back(er);
		}
	}
}

/*private*/
void PolygonBuilder::buildMaximalEdgeRings(const std::vector<DirectedEdge *> *dirEdges,
                                           std::vector<MaximalEdgeRing *> &maxEdgeRings) {
	std::vector<MaximalEdgeRing *>::size_type oldSize = maxEdgeRings.size();

	for (std::size_t i = 0, n = dirEdges->size(); i < n; i++) {
		DirectedEdge *de = (*dirEdges)[i];
		if (de->isInResult() && de->getLabel().isArea()) {
			// if this edge has not yet been processed
			if (de->getEdgeRing() == nullptr) {
				MaximalEdgeRing *er;
				try {
					// MaximalEdgeRing constructor may throw
					er = new MaximalEdgeRing(de, geometryFactory);
				} catch (util::GEOSException &) {
					// cleanup if that happens (see stmlf-cases-20061020.xml)
					for (std::size_t p_i = oldSize, p_n = maxEdgeRings.size(); p_i < p_n; p_i++) {
						delete maxEdgeRings[p_i];
					}
					throw;
				}
				maxEdgeRings.push_back(er);
				er->setInResult();
			}
		}
	}
}

/*private*/
EdgeRing *PolygonBuilder::findEdgeRingContaining(EdgeRing *testEr, std::vector<FastPIPRing> &newShellList) {
	LinearRing *testRing = testEr->getLinearRing();
	const Envelope *testEnv = testRing->getEnvelopeInternal();
	EdgeRing *minShell = nullptr;
	const Envelope *minShellEnv = nullptr;

	for (auto const &tryShell : newShellList) {
		LinearRing *tryShellRing = tryShell.edgeRing->getLinearRing();
		const Envelope *tryShellEnv = tryShellRing->getEnvelopeInternal();
		// the hole envelope cannot equal the shell envelope
		// (also guards against testing rings against themselves)
		if (tryShellEnv->equals(testEnv)) {
			continue;
		}
		// hole must be contained in shell
		if (!tryShellEnv->contains(testEnv)) {
			continue;
		}

		const CoordinateSequence *tsrcs = tryShellRing->getCoordinatesRO();
		const Coordinate &testPt = operation::polygonize::EdgeRing::ptNotInList(testRing->getCoordinatesRO(), tsrcs);

		bool isContained = false;
		if (tryShell.pipLocator->locate(&testPt) != Location::EXTERIOR) {
			isContained = true;
		}

		// check if this new containing ring is smaller than
		// the current minimum ring
		if (isContained) {
			if (minShell == nullptr || minShellEnv->contains(tryShellEnv)) {
				minShell = tryShell.edgeRing;
				minShellEnv = minShell->getLinearRing()->getEnvelopeInternal();
			}
		}
	}
	return minShell;
}

/*private*/
EdgeRing *PolygonBuilder::findShell(std::vector<MinimalEdgeRing *> *minEdgeRings) {
	int shellCount = 0;
	EdgeRing *shell = nullptr;

	for (std::size_t i = 0, n = minEdgeRings->size(); i < n; ++i) {
		EdgeRing *er = (*minEdgeRings)[i];
		if (!er->isHole()) {
			shell = er;
			++shellCount;
		}
	}

	if (shellCount > 1) {
		throw util::TopologyException("found two shells in MinimalEdgeRing list");
	}

	return shell;
}

/*private*/
void PolygonBuilder::placePolygonHoles(EdgeRing *shell, std::vector<MinimalEdgeRing *> *minEdgeRings) {
	for (std::size_t i = 0, n = minEdgeRings->size(); i < n; ++i) {
		MinimalEdgeRing *er = (*minEdgeRings)[i];
		if (er->isHole()) {
			er->setShell(shell);
		}
	}
}

} // namespace overlay
} // namespace operation
} // namespace geos
