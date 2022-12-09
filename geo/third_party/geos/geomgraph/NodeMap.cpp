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
 * Last port: geomgraph/NodeMap.java rev. 1.3 (JTS-1.10)
 *
 **********************************************************************/

#include <cassert>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geomgraph/EdgeEnd.hpp>
#include <geos/geomgraph/Label.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/geomgraph/NodeFactory.hpp>
#include <geos/geomgraph/NodeMap.hpp>
#include <vector>

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

NodeMap::NodeMap(const NodeFactory &newNodeFact) : nodeFact(newNodeFact) {
}

NodeMap::~NodeMap() = default;

// first arg cannot be const because
// it is liable to label-merging ... --strk;
Node *NodeMap::addNode(Node *n) {
	assert(n);
	Coordinate *c = const_cast<Coordinate *>(&n->getCoordinate());
	Node *node = find(*c);
	if (node == nullptr) {
		nodeMap[c] = std::unique_ptr<Node>(n);
		return nodeMap[c].get();
	}

	node->mergeLabel(*n);
	return node;
}

void NodeMap::add(EdgeEnd *e) {
	Coordinate &p = e->getCoordinate();
	Node *n = addNode(p);
	n->add(e);
}

void NodeMap::add(std::unique_ptr<EdgeEnd> &&e) {
	add(e.get());
	e.release();
}

/*
 * @return the node if found; null otherwise
 */
Node *NodeMap::find(const Coordinate &coord) const {
	Coordinate *c = const_cast<Coordinate *>(&coord);

	const auto &found = nodeMap.find(c);

	if (found == nodeMap.end()) {
		return nullptr;
	} else {
		return found->second.get();
	}
}

Node *NodeMap::addNode(const Coordinate &coord) {
	Node *node = find(coord);
	if (node == nullptr) {
		node = nodeFact.createNode(coord);
		Coordinate *c = const_cast<Coordinate *>(&(node->getCoordinate()));
		nodeMap[c] = std::unique_ptr<Node>(node);
		node = nodeMap[c].get();
	} else {
		node->addZ(coord.z);
	}
	return node;
}

void NodeMap::getBoundaryNodes(uint8_t geomIndex, std::vector<Node *> &bdyNodes) const {
	for (const auto &it : nodeMap) {
		Node *node = it.second.get();
		if (node->getLabel().getLocation(geomIndex) == Location::BOUNDARY) {
			bdyNodes.push_back(node);
		}
	}
}

} // namespace geomgraph
} // namespace geos
