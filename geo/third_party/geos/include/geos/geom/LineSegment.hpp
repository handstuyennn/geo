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

#pragma once

#include <array>
#include <cassert>
#include <functional> // for std::hash
#include <geos/algorithm/Distance.hpp>
#include <geos/algorithm/Orientation.hpp>
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp> // for composition
#include <iostream>                 // for ostream
#include <memory>                   // for unique_ptr

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
class GeometryFactory;
class LineString;
} // namespace geom
} // namespace geos

namespace geos {
namespace geom { // geos::geom

/**
 * Represents a line segment defined by two Coordinate.
 * Provides methods to compute various geometric properties
 * and relationships of line segments.
 *
 * This class is designed to be easily mutable (to the extent of
 * having its contained points public).
 * This supports a common pattern of reusing a single LineSegment
 * object as a way of computing segment properties on the
 * segments defined by arrays or lists of {@link Coordinate}s.
 *
 * TODO: have this class keep pointers rather then real Coordinates ?
 */
class GEOS_DLL LineSegment {
public:
	Coordinate p0; /// Segment start
	Coordinate p1; /// Segment end

	/// Checks if two LineSegment are equal (2D only check)
	friend bool operator==(const LineSegment &a, const LineSegment &b) {
		return a.p0 == b.p0 && a.p1 == b.p1;
	};

	LineSegment(const Coordinate &c0, const Coordinate &c1) : p0(c0), p1(c1) {};

	LineSegment(double x0, double y0, double x1, double y1) : p0(x0, y0), p1(x1, y1) {};

	LineSegment() {};

	void setCoordinates(const Coordinate &c0, const Coordinate &c1) {
		p0 = c0;
		p1 = c1;
	};

	void setCoordinates(const LineSegment &ls) {
		setCoordinates(ls.p0, ls.p1);
	};

	/** \brief
	 * Compute the projection factor for the projection of the point p
	 * onto this LineSegment.
	 *
	 * The projection factor is the constant r
	 * by which the vector for this segment must be multiplied to
	 * equal the vector for the projection of p on the line
	 * defined by this segment.
	 *
	 * The projection factor returned will be in the range
	 * (-inf, +inf)
	 *
	 * @param p the point to compute the factor for
	 *
	 * @return the projection factor for the point
	 *
	 */
	double projectionFactor(const CoordinateXY &p) const;

	/// Computes the distance between this line segment and another one.
	double distance(const LineSegment &ls) const {
		return algorithm::Distance::segmentToSegment(p0, p1, ls.p0, ls.p1);
	};

	/// Computes the distance between this line segment and a point.
	double distance(const CoordinateXY &p) const {
		return algorithm::Distance::pointToSegment(p, p0, p1);
	};

	/// gets the minimum X ordinate value
	double minX() const {
		return std::min(p0.x, p1.x);
	};

	/// gets the maximum X ordinate value
	double maxX() const {
		return std::max(p0.x, p1.x);
	};

	/// gets the minimum Y ordinate value
	double minY() const {
		return std::min(p0.y, p1.y);
	};

	/// gets the maximum Y ordinate value
	double maxY() const {
		return std::max(p0.y, p1.y);
	};

	/// Computes the length of the line segment.
	double getLength() const {
		return p0.distance(p1);
	};

	/** \brief
	 * Computes the Coordinate that lies a given
	 * fraction along the line defined by this segment.
	 *
	 * A fraction of <code>0.0</code> returns the start point of
	 * the segment; a fraction of <code>1.0</code> returns the end
	 * point of the segment.
	 * If the fraction is < 0.0 or > 1.0 the point returned
	 * will lie before the start or beyond the end of the segment.
	 *
	 * @param segmentLengthFraction the fraction of the segment length
	 *        along the line
	 * @param ret will be set to the point at that distance
	 */
	void pointAlong(double segmentLengthFraction, Coordinate &ret) const {
		ret = Coordinate(p0.x + segmentLengthFraction * (p1.x - p0.x), p0.y + segmentLengthFraction * (p1.y - p0.y));
	};

	/**
	 * Determines the orientation of a LineSegment relative to this segment.
	 * The concept of orientation is specified as follows:
	 * Given two line segments A and L,
	 * <ul>
	 * <li>A is to the left of a segment L if A lies wholly in the
	 * closed half-plane lying to the left of L
	 * <li>A is to the right of a segment L if A lies wholly in the
	 * closed half-plane lying to the right of L
	 * <li>otherwise, A has indeterminate orientation relative to L.
	 *     This happens if A is collinear with L or if A crosses
	 *     the line determined by L.
	 * </ul>
	 *
	 * @param seg the LineSegment to compare
	 *
	 * @return 1 if seg is to the left of this segment
	 * @return -1 if seg is to the right of this segment
	 * @return 0 if seg has indeterminate orientation relative
	 *         to this segment
	 */
	int orientationIndex(const LineSegment &seg) const;

	// TODO deprecate this
	int orientationIndex(const LineSegment *seg) const {
		assert(seg);
		return orientationIndex(*seg);
	};

	/** \brief
	 * Compares this object with the specified object for order.
	 *
	 * Uses the standard lexicographic ordering for the points in the LineSegment.
	 *
	 * @param  other  the LineSegment with which this LineSegment
	 *            is being compared
	 * @return a negative integer, zero, or a positive integer as this
	 *         LineSegment is less than, equal to, or greater than the
	 *         specified LineSegment
	 */
	int compareTo(const LineSegment &other) const;

	/**
	 * Computes an intersection point between two segments,
	 * if there is one.
	 * There may be 0, 1 or many intersection points between two segments.
	 * If there are 0, null is returned. If there is 1 or more, a single
	 * one is returned (chosen at the discretion of the algorithm).
	 * If more information is required about the details of the
	 * intersection, the LineIntersector class should be used.
	 *
	 * @param line
	 * @return intersection if found, setNull() otherwise
	 */
	Coordinate intersection(const LineSegment &line) const;
};

// std::ostream& operator<< (std::ostream& o, const LineSegment& l);

} // namespace geom
} // namespace geos
