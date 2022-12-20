/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geomgraph/Node.java r411 (JTS-1.12+)
 *
 **********************************************************************/

#include <algorithm>
#include <cmath>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geomgraph/DirectedEdge.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/EdgeEndStar.hpp>
#include <geos/geomgraph/Label.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/util.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public*/
Node::Node(const Coordinate &newCoord, EdgeEndStar *newEdges)
    : GraphComponent(Label(0, Location::NONE)), coord(newCoord), edges(newEdges)

{
	testInvariant();
}

/*public*/
Node::~Node() {
	testInvariant();
	delete edges;
}

/*public*/
EdgeEndStar *Node::getEdges() {
	testInvariant();

	return edges;
}

void Node::add(EdgeEnd *e) {
	assert(e);
	// Assert: start pt of e is equal to node point
	if (!e->getCoordinate().equals2D(coord)) {
		std::stringstream ss;
		ss << "EdgeEnd with coordinate " << e->getCoordinate() << " invalid for node " << coord;
		throw util::IllegalArgumentException(ss.str());
	}

	// It seems it's legal for edges to be NULL
	// we'd not be honouring the promise of adding
	// an EdgeEnd in this case, though ...
	assert(edges);
	// if (edges==NULL) return;

	edges->insert(e);
	e->setNode(this);
	testInvariant();
}

/*public*/
void Node::mergeLabel(const Node &n) {
	assert(!n.label.isNull());
	mergeLabel(n.label);
	testInvariant();
}

/*public*/
void Node::mergeLabel(const Label &label2) {
	for (uint8_t i = 0; i < 2; i++) {
		Location loc = computeMergedLocation(label2, i);
		Location thisLoc = label.getLocation(i);
		if (thisLoc == Location::NONE) {
			label.setLocation(i, loc);
		}
	}
	testInvariant();
}

/*public*/
bool Node::isIsolated() const {
	testInvariant();

	return (label.getGeometryCount() == 1);
}

/*public*/
const Coordinate &Node::getCoordinate() const {
	testInvariant();
	return coord;
}

/*public*/
void Node::setLabel(uint8_t argIndex, Location onLocation) {
	if (label.isNull()) {
		label = Label(argIndex, onLocation);
	} else {
		label.setLocation(argIndex, onLocation);
	}

	testInvariant();
}

/*public*/
void Node::setLabelBoundary(uint8_t argIndex) {
	Location loc = label.getLocation(argIndex);
	// flip the loc
	Location newLoc;
	switch (loc) {
	case Location::BOUNDARY:
		newLoc = Location::INTERIOR;
		break;
	case Location::INTERIOR:
		newLoc = Location::BOUNDARY;
		break;
	default:
		newLoc = Location::BOUNDARY;
		break;
	}
	label.setLocation(argIndex, newLoc);

	testInvariant();
}

/*public*/
void Node::addZ(double z) {
	if (std::isnan(z)) {
		return;
	}
	if (find(zvals.begin(), zvals.end(), z) != zvals.end()) {
		return;
	}
	zvals.push_back(z);
	ztot += z;
	coord.z = ztot / static_cast<double>(zvals.size());
}

/*public*/
bool Node::isIncidentEdgeInResult() const {
	testInvariant();

	if (!edges) {
		return false;
	}

	EdgeEndStar::iterator it = edges->begin();
	EdgeEndStar::iterator endIt = edges->end();
	for (; it != endIt; ++it) {
		assert(*it);
		DirectedEdge *de = detail::down_cast<DirectedEdge *>(*it);
		if (de->getEdge()->isInResult()) {
			return true;
		}
	}
	return false;
}

/*public*/
Location Node::computeMergedLocation(const Label &label2, uint8_t eltIndex) {
	Location loc = label.getLocation(eltIndex);
	if (!label2.isNull(eltIndex)) {
		Location nLoc = label2.getLocation(eltIndex);
		if (loc != Location::BOUNDARY) {
			loc = nLoc;
		}
	}

	testInvariant();

	return loc;
}

} // namespace geomgraph
} // namespace geos
