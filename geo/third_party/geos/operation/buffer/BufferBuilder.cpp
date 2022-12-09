/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2008-2010 Safe Software Inc.
 * Copyright (C) 2005-2007 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferBuilder.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <cassert>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geom/MultiLineString.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geom/Position.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/Label.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/geomgraph/PlanarGraph.hpp>
#include <geos/noding/IntersectionAdder.hpp>
#include <geos/noding/MCIndexNoder.hpp>
#include <geos/noding/NodedSegmentString.hpp>
#include <geos/noding/SegmentString.hpp>
#include <geos/operation/buffer/BufferBuilder.hpp>
#include <geos/operation/buffer/BufferCurveSetBuilder.hpp>
#include <geos/operation/buffer/BufferSubgraph.hpp>
#include <geos/operation/buffer/SubgraphDepthLocater.hpp>
#include <geos/operation/overlay/OverlayNodeFactory.hpp>
#include <geos/operation/overlay/OverlayOp.hpp>
#include <geos/operation/overlay/PolygonBuilder.hpp>
#include <geos/operation/valid/RepeatedPointRemover.hpp>
#include <geos/util/GEOSException.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <geos/util/Interrupt.hpp>
#include <iomanip>
#include <iostream>
#include <vector>

//
using namespace geos::geom;
using namespace geos::geomgraph;
using namespace geos::noding;
using namespace geos::algorithm;
using namespace geos::operation::overlay;

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

int BufferBuilder::depthDelta(const Label &label) {
	Location lLoc = label.getLocation(0, Position::LEFT);
	Location rLoc = label.getLocation(0, Position::RIGHT);
	if (lLoc == Location::INTERIOR && rLoc == Location::EXTERIOR) {
		return 1;
	} else if (lLoc == Location::EXTERIOR && rLoc == Location::INTERIOR) {
		return -1;
	}
	return 0;
}

BufferBuilder::~BufferBuilder() {
	delete li; // could be NULL
	delete intersectionAdder;
}

/* private */
void BufferBuilder::computeNodedEdges(SegmentString::NonConstVect &bufferSegStrList,
                                      const PrecisionModel *precisionModel) // throw(GEOSException)
{
	Noder *noder = getNoder(precisionModel);

	noder->computeNodes(&bufferSegStrList);

	SegmentString::NonConstVect *nodedSegStrings = noder->getNodedSubstrings();

	for (SegmentString::NonConstVect::iterator i = nodedSegStrings->begin(), e = nodedSegStrings->end(); i != e; ++i) {
		SegmentString *segStr = *i;
		const Label *oldLabel = static_cast<const Label *>(segStr->getData());

		auto cs = operation::valid::RepeatedPointRemover::removeRepeatedPoints(segStr->getCoordinates());
		delete segStr;
		if (cs->size() < 2) {
			// don't insert collapsed edges
			// we need to take care of the memory here as cs is a new sequence
			continue;
		}

		// Edge takes ownership of the CoordinateSequence
		Edge *edge = new Edge(cs.release(), *oldLabel);

		// will take care of the Edge ownership
		insertUniqueEdge(edge);
	}

	delete nodedSegStrings;

	if (noder != workingNoder) {
		delete noder;
	}
}

/*private*/
void BufferBuilder::insertUniqueEdge(Edge *e) {
	//<FIX> MD 8 Oct 03  speed up identical edge lookup
	// fast lookup
	Edge *existingEdge = edgeList.findEqualEdge(e);
	// If an identical edge already exists, simply update its label
	if (existingEdge != nullptr) {
		Label &existingLabel = existingEdge->getLabel();
		Label labelToMerge = e->getLabel();

		// check if new edge is in reverse direction to existing edge
		// if so, must flip the label before merging it
		if (!existingEdge->isPointwiseEqual(e)) {
			labelToMerge = e->getLabel();
			labelToMerge.flip();
		}

		existingLabel.merge(labelToMerge);

		// compute new depth delta of sum of edges
		int mergeDelta = depthDelta(labelToMerge);
		int existingDelta = existingEdge->getDepthDelta();
		int newDelta = existingDelta + mergeDelta;
		existingEdge->setDepthDelta(newDelta);

		// we have memory release responsibility
		delete e;

	} else { // no matching existing edge was found

		// add this new edge to the list of edges in this graph
		edgeList.add(e);

		e->setDepthDelta(depthDelta(e->getLabel()));
	}
}

/*private*/
Noder *BufferBuilder::getNoder(const PrecisionModel *pm) {
	// this doesn't change workingNoder precisionModel!
	if (workingNoder != nullptr) {
		return workingNoder;
	}

	// otherwise use a fast (but non-robust) noder

	if (li) { // reuse existing IntersectionAdder and LineIntersector
		li->setPrecisionModel(pm);
		assert(intersectionAdder != nullptr);
	} else {
		li = new LineIntersector(pm);
		intersectionAdder = new IntersectionAdder(*li);
	}

	MCIndexNoder *noder = new MCIndexNoder(intersectionAdder);
	return noder;
}

/*public*/
std::unique_ptr<Geometry> BufferBuilder::buffer(const Geometry *g, double distance)
// throw(GEOSException *)
{
	const PrecisionModel *precisionModel = workingPrecisionModel;
	if (precisionModel == nullptr) {
		precisionModel = g->getPrecisionModel();
	}

	assert(precisionModel);
	assert(g);

	// factory must be the same as the one used by the input
	geomFact = g->getFactory();

	{
		// This scope is here to force release of resources owned by
		// BufferCurveSetBuilder when we're doing with it
		BufferCurveSetBuilder curveSetBuilder(*g, distance, precisionModel, bufParams);
		curveSetBuilder.setInvertOrientation(isInvertOrientation);

		GEOS_CHECK_FOR_INTERRUPTS();

		std::vector<SegmentString *> &bufferSegStrList = curveSetBuilder.getCurves();

		// short-circuit test
		if (bufferSegStrList.empty()) {
			return createEmptyResultGeometry();
		}

		computeNodedEdges(bufferSegStrList, precisionModel);

		GEOS_CHECK_FOR_INTERRUPTS();

	} // bufferSegStrList and contents are released here

	std::unique_ptr<Geometry> resultGeom(nullptr);
	std::unique_ptr<std::vector<Geometry *>> resultPolyList;
	std::vector<BufferSubgraph *> subgraphList;

	try {
		PlanarGraph graph(OverlayNodeFactory::instance());
		graph.addEdges(edgeList.getEdges());

		GEOS_CHECK_FOR_INTERRUPTS();

		createSubgraphs(&graph, subgraphList);

		GEOS_CHECK_FOR_INTERRUPTS();

		{
			// scope for earlier PolygonBuilder cleanup
			PolygonBuilder polyBuilder(geomFact);
			buildSubgraphs(subgraphList, polyBuilder);

			resultPolyList.reset(polyBuilder.getPolygons());
		}

		// Get rid of the subgraphs, shouldn't be needed anymore
		for (std::size_t i = 0, n = subgraphList.size(); i < n; i++) {
			delete subgraphList[i];
		}
		subgraphList.clear();

		// just in case ...
		if (resultPolyList->empty()) {
			return createEmptyResultGeometry();
		}

		// resultPolyList ownership transferred here
		resultGeom.reset(geomFact->buildGeometry(resultPolyList.release()));

	} catch (const util::GEOSException & /* exc */) {

		// In case they're still around
		for (std::size_t i = 0, n = subgraphList.size(); i < n; i++) {
			delete subgraphList[i];
		}
		subgraphList.clear();

		throw;
	}

	return resultGeom;
}

/*private*/
void BufferBuilder::buildSubgraphs(const std::vector<BufferSubgraph *> &subgraphList, PolygonBuilder &polyBuilder) {
	std::vector<BufferSubgraph *> processedGraphs;
	for (std::size_t i = 0, n = subgraphList.size(); i < n; i++) {
		BufferSubgraph *subgraph = subgraphList[i];
		Coordinate *p = subgraph->getRightmostCoordinate();
		assert(p);

		SubgraphDepthLocater locater(&processedGraphs);
		int outsideDepth = locater.getDepth(*p);
		subgraph->computeDepth(outsideDepth);
		subgraph->findResultEdges();
		processedGraphs.push_back(subgraph);
		polyBuilder.add(subgraph->getDirectedEdges(), subgraph->getNodes());
	}
}

/*private*/
std::unique_ptr<geom::Geometry> BufferBuilder::createEmptyResultGeometry() const {
	return geomFact->createPolygon();
}

bool BufferSubgraphGT(BufferSubgraph *first, BufferSubgraph *second) {
	if (first->compareTo(second) > 0) {
		return true;
	} else {
		return false;
	}
}

/*private*/
void BufferBuilder::createSubgraphs(PlanarGraph *graph, std::vector<BufferSubgraph *> &subgraphList) {
	std::vector<Node *> nodes;
	graph->getNodes(nodes);
	for (std::size_t i = 0, n = nodes.size(); i < n; i++) {
		Node *node = nodes[i];
		if (!node->isVisited()) {
			BufferSubgraph *subgraph = new BufferSubgraph();
			subgraph->create(node);
			subgraphList.push_back(subgraph);
		}
	}

	/*
	 * Sort the subgraphs in descending order of their rightmost coordinate
	 * This ensures that when the Polygons for the subgraphs are built,
	 * subgraphs for shells will have been built before the subgraphs for
	 * any holes they contain
	 */
	std::sort(subgraphList.begin(), subgraphList.end(), BufferSubgraphGT);
}

} // namespace buffer
} // namespace operation
} // namespace geos
