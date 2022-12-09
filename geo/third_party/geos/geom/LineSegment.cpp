/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geom/LineSegment.java r18 (JTS-1.11)
 *
 **********************************************************************/

#include <algorithm> // for max
#include <cmath>
#include <geos/algorithm/Intersection.hpp>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/constants.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LineSegment.hpp>
#include <geos/geom/LineString.hpp> // for toGeometry
#include <sstream>

namespace geos {
namespace geom { // geos::geom

/*public*/
double LineSegment::projectionFactor(const CoordinateXY &p) const {
	if (p == p0) {
		return 0.0;
	}
	if (p == p1) {
		return 1.0;
	}
	// Otherwise, use comp.graphics.algorithms Frequently Asked Questions method
	/*(1)     	      AC dot AB
	               r = ---------
	                     ||AB||^2
	            r has the following meaning:
	            r=0 P = A
	            r=1 P = B
	            r<0 P is on the backward extension of AB
	            r>1 P is on the forward extension of AB
	            0<r<1 P is interior to AB
	    */
	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;
	double len2 = dx * dx + dy * dy;
	double r = ((p.x - p0.x) * dx + (p.y - p0.y) * dy) / len2;
	return r;
}

/*public*/
int LineSegment::compareTo(const LineSegment &other) const {
	int comp0 = p0.compareTo(other.p0);
	if (comp0 != 0) {
		return comp0;
	}
	return p1.compareTo(other.p1);
}

/*public*/
int LineSegment::orientationIndex(const LineSegment &seg) const {
	int orient0 = algorithm::Orientation::index(p0, p1, seg.p0);
	int orient1 = algorithm::Orientation::index(p0, p1, seg.p1);
	// this handles the case where the points are L or collinear
	if (orient0 >= 0 && orient1 >= 0) {
		return std::max(orient0, orient1);
	}
	// this handles the case where the points are R or collinear
	if (orient0 <= 0 && orient1 <= 0) {
		return std::min(orient0, orient1);
	}
	// points lie on opposite sides ==> indeterminate orientation
	return 0;
}

Coordinate LineSegment::intersection(const LineSegment &line) const {
	algorithm::LineIntersector li;
	li.computeIntersection(p0, p1, line.p0, line.p1);
	if (li.hasIntersection()) {
		return li.getIntersection(0);
	}
	Coordinate rv;
	rv.setNull();
	return rv;
}

} // namespace geom
} // namespace geos
