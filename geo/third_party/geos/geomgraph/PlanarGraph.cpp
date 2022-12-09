/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/PlanarGraph.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#include <cassert>
#include <geos/algorithm/Orientation.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geom/Quadrant.hpp>
#include <geos/geomgraph/DirectedEdge.hpp>
#include <geos/geomgraph/DirectedEdgeStar.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/EdgeEndStar.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/geomgraph/NodeFactory.hpp>
#include <geos/geomgraph/NodeMap.hpp>
#include <geos/geomgraph/PlanarGraph.hpp>
#include <geos/util.hpp>
#include <sstream>
#include <string>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public*/
PlanarGraph::PlanarGraph(const NodeFactory &nodeFact)
    : edges(new std::vector<Edge *>()), nodes(new NodeMap(nodeFact)), edgeEndList(new std::vector<EdgeEnd *>()) {
}

/*public*/
PlanarGraph::PlanarGraph()
    : edges(new std::vector<Edge *>()), nodes(new NodeMap(NodeFactory::instance())),
      edgeEndList(new std::vector<EdgeEnd *>()) {
}

/*public*/
PlanarGraph::~PlanarGraph() {
	delete nodes;
#if 1 // FIXME: PlanarGraph should *not* own edges!
	for (std::size_t i = 0, n = edges->size(); i < n; i++) {
		delete (*edges)[i];
	}
#endif
	delete edges;

	for (std::size_t i = 0, n = edgeEndList->size(); i < n; i++) {
		delete (*edgeEndList)[i];
	}
	delete edgeEndList;
}

/*public*/
bool PlanarGraph::isBoundaryNode(uint8_t geomIndex, const Coordinate &coord) {
	assert(nodes);

	Node *node = nodes->find(coord);
	if (node == nullptr) {
		return false;
	}

	const Label &label = node->getLabel();
	if (!label.isNull() && label.getLocation(geomIndex) == Location::BOUNDARY) {
		return true;
	}

	return false;
}

/*public*/
void PlanarGraph::add(EdgeEnd *e) {
	// It is critical to add the edge to the edgeEndList first,
	// then it is safe to follow with any potentially throwing operations.
	assert(edgeEndList);
	edgeEndList->push_back(e);

	assert(e);
	assert(nodes);
	nodes->add(e);
}

/*public*/
std::vector<EdgeEnd *> *PlanarGraph::getEdgeEnds() {
	return edgeEndList;
}

NodeMap *PlanarGraph::getNodeMap() {
	return nodes;
}

// arg cannot be const, NodeMap::addNode will
// occasionally label-merge first arg.
/*public*/
Node *PlanarGraph::addNode(Node *node) {
	assert(nodes);
	return nodes->addNode(node);
}

/*public*/
Node *PlanarGraph::addNode(const Coordinate &coord) {
	return nodes->addNode(coord);
}

/*public*/
void PlanarGraph::addEdges(const std::vector<Edge *> &edgesToAdd) {
	// create all the nodes for the edges
	for (std::vector<Edge *>::const_iterator it = edgesToAdd.begin(), endIt = edgesToAdd.end(); it != endIt; ++it) {
		Edge *e = *it;
		assert(e);
		edges->push_back(e);

		// PlanarGraph destructor will delete all DirectedEdges
		// in edgeEndList, which is where these are added
		// by the ::add(EdgeEnd) call
		auto de1 = detail::make_unique<DirectedEdge>(e, true);
		auto de2 = detail::make_unique<DirectedEdge>(e, false);
		de1->setSym(de2.get());
		de2->setSym(de1.get());

		// First, ::add takes the ownership, then follows with operations that may throw.
		add(de1.release());
		add(de2.release());
	}
}

/*public*/
Edge *PlanarGraph::findEdge(const Coordinate &p0, const Coordinate &p1) {
	for (std::size_t i = 0, n = edges->size(); i < n; ++i) {
		Edge *e = (*edges)[i];
		assert(e);

		const CoordinateSequence *eCoord = e->getCoordinates();
		assert(eCoord);

		if (p0 == eCoord->getAt(0) && p1 == eCoord->getAt(1)) {
			return e;
		}
	}
	return nullptr;
}

/*public*/
void PlanarGraph::getNodes(std::vector<Node *> &values) {
	assert(nodes);
	for (const auto &elem : nodes->nodeMap) {
		assert(elem.second.get());
		values.push_back(elem.second.get());
	}
}

} // namespace geomgraph
} // namespace geos