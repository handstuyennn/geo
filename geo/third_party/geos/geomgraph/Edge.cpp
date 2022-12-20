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
 * Last port: geomgraph/Edge.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#include <cassert>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateArraySequence.hpp> // FIXME: shouldn't use
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/IntersectionMatrix.hpp>
#include <geos/geom/Position.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/Label.hpp>
#include <geos/geomgraph/index/MonotoneChainEdge.hpp>
#include <geos/util.hpp>
#include <sstream>
#include <string>

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

using namespace geos::geomgraph::index;
using namespace geos::algorithm;

/**
 * Updates an IM from the label for an edge.
 * Handles edges from both L and A geometrys.
 */
void Edge::updateIM(const Label &lbl, IntersectionMatrix &im) {
	im.setAtLeastIfValid(lbl.getLocation(0, Position::ON), lbl.getLocation(1, Position::ON), 1);
	if (lbl.isArea()) {

		im.setAtLeastIfValid(lbl.getLocation(0, Position::LEFT), lbl.getLocation(1, Position::LEFT), 2);

		im.setAtLeastIfValid(lbl.getLocation(0, Position::RIGHT), lbl.getLocation(1, Position::RIGHT), 2);
	}
}

/*public*/
Edge::~Edge() = default;

/*public*/
Edge::Edge(CoordinateSequence *newPts, const Label &newLabel)
    : GraphComponent(newLabel), mce(nullptr), env(newPts->getEnvelope()), depth(), depthDelta(0), isIsolatedVar(true),
      pts(newPts), eiList(this) {
	testInvariant();
}

/*public*/
Edge::Edge(CoordinateSequence *newPts)
    : GraphComponent(), mce(nullptr), env(newPts->getEnvelope()), depth(), depthDelta(0), isIsolatedVar(true),
      pts(newPts), eiList(this) {
	testInvariant();
}

const Envelope *Edge::getEnvelope() {
	return &env;
}

Edge *Edge::getCollapsedEdge() {
	testInvariant();
	CoordinateSequence *newPts = new CoordinateArraySequence(2);
	newPts->setAt(pts->getAt(0), 0);
	newPts->setAt(pts->getAt(1), 1);
	return new Edge(newPts, Label::toLineLabel(label));
}

/*public*/
bool Edge::isCollapsed() const {
	testInvariant();
	if (!label.isArea()) {
		return false;
	}
	if (getNumPoints() != 3) {
		return false;
	}
	if (pts->getAt(0) == pts->getAt(2)) {
		return true;
	}
	return false;
}

/*public*/
bool Edge::isPointwiseEqual(const Edge *e) const {
	testInvariant();
	auto npts = getNumPoints();
	auto enpts = e->getNumPoints();
	if (npts != enpts) {
		return false;
	}
	for (unsigned int i = 0; i < npts; ++i) {
		if (!pts->getAt(i).equals2D(e->pts->getAt(i))) {
			return false;
		}
	}
	return true;
}

/*public*/
MonotoneChainEdge *Edge::getMonotoneChainEdge() {
	testInvariant();
	if (mce == nullptr) {
		mce = detail::make_unique<MonotoneChainEdge>(this);
	}
	return mce.get();
}

/*public*/
void Edge::addIntersections(LineIntersector *li, std::size_t segmentIndex, std::size_t geomIndex) {
	for (std::size_t i = 0; i < li->getIntersectionNum(); ++i) {
		addIntersection(li, segmentIndex, geomIndex, i);
	}

	testInvariant();
}

/*public*/
void Edge::addIntersection(LineIntersector *li, std::size_t segmentIndex, std::size_t geomIndex, std::size_t intIndex) {
	const Coordinate &intPt = li->getIntersection(intIndex);
	auto normalizedSegmentIndex = segmentIndex;
	double dist = li->getEdgeDistance(geomIndex, intIndex);

	// normalize the intersection point location
	auto nextSegIndex = normalizedSegmentIndex + 1;
	auto npts = getNumPoints();
	if (nextSegIndex < npts) {
		const Coordinate &nextPt = pts->getAt(nextSegIndex);
		// Normalize segment index if intPt falls on vertex
		// The check for point equality is 2D only - Z values are ignored
		if (intPt.equals2D(nextPt)) {
			normalizedSegmentIndex = nextSegIndex;
			dist = 0.0;
		}
	}

	/*
	 * Add the intersection point to edge intersection list.
	 */
	eiList.add(intPt, normalizedSegmentIndex, dist);

	testInvariant();
}

} // namespace geomgraph
} // namespace geos
