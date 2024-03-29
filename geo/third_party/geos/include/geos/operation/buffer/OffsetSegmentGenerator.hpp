/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/OffsetSegmentGenerator.java r378 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/LineIntersector.hpp> // for composition
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>                      // for composition
#include <geos/geom/LineSegment.hpp>                     // for composition
#include <geos/operation/buffer/BufferParameters.hpp>    // for composition
#include <geos/operation/buffer/OffsetSegmentString.hpp> // for composition
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
class PrecisionModel;
} // namespace geom
} // namespace geos

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

/**
 * Generates segments which form an offset curve.
 * Supports all end cap and join options
 * provided for buffering.
 * Implements various heuristics to
 * produce smoother, simpler curves which are
 * still within a reasonable tolerance of the
 * true curve.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL OffsetSegmentGenerator {

public:
	/*
	 * @param nBufParams buffer parameters, this object will
	 *                   keep a reference to the passed parameters
	 *                   so caller must make sure the object is
	 *                   kept alive for the whole lifetime of
	 *                   the buffer builder.
	 */
	OffsetSegmentGenerator(const geom::PrecisionModel *newPrecisionModel, const BufferParameters &bufParams,
	                       double distance);

	/// Get coordinates by taking ownership of them
	///
	/// After this call, the coordinates reference in
	/// this object are dropped. Calling twice will
	/// segfault...
	///
	/// FIXME: refactor memory management of this
	///
	void getCoordinates(std::vector<geom::CoordinateSequence *> &to) {
		to.push_back(segList.getCoordinates());
	}

	/// Add last offset point
	void addLastSegment() {
		segList.addPt(offset1.p1);
	}

	void initSideSegments(const geom::Coordinate &nS1, const geom::Coordinate &nS2, int nSide);

	void addNextSegment(const geom::Coordinate &p, bool addStartPoint);

	/// \brief
	/// Add an end cap around point p1, terminating a line segment
	/// coming from p0
	void addLineEndCap(const geom::Coordinate &p0, const geom::Coordinate &p1);

	/// Add first offset point
	void addFirstSegment() {
		segList.addPt(offset1.p0);
	}

	void closeRing() {
		segList.closeRing();
	}

	/// Adds a CW circle around a point
	void createCircle(const geom::Coordinate &p, double distance);

	/// Adds a CW square around a point
	void createSquare(const geom::Coordinate &p, double distance);

	void addSegments(const geom::CoordinateSequence &pts, bool isForward) {
		segList.addPts(pts, isForward);
	}

	/** \brief
	 * Compute an offset segment for an input segment on a given
	 * side and at a given distance.
	 *
	 * The offset points are computed in full double precision,
	 * for accuracy.
	 *
	 * @param seg the segment to offset
	 * @param side the side of the segment the offset lies on
	 * @param distance the offset distance
	 * @param offset the points computed for the offset segment
	 */
	static void computeOffsetSegment(const geom::LineSegment &seg, int side, double distance,
	                                 geom::LineSegment &offset);

	/**
	 * Adds points for a circular fillet around a reflex corner.
	 *
	 * Adds the start and end points
	 *
	 * @param p base point of curve
	 * @param p0 start point of fillet curve
	 * @param p1 endpoint of fillet curve
	 * @param direction the orientation of the fillet
	 * @param radius the radius of the fillet
	 */
	void addDirectedFillet(const geom::Coordinate &p, const geom::Coordinate &p0, const geom::Coordinate &p1,
	                       int direction, double radius);

	/**
	 * Adds points for a circular fillet arc between two specified angles.
	 *
	 * The start and end point for the fillet are not added -
	 * the caller must add them if required.
	 *
	 * @param direction is -1 for a CW angle, 1 for a CCW angle
	 * @param radius the radius of the fillet
	 */
	void addDirectedFillet(const geom::Coordinate &p, double startAngle, double endAngle, int direction, double radius);

private:
	/**
	 * Factor which controls how close offset segments can be to
	 * skip adding a filler or mitre.
	 */
	static const double OFFSET_SEGMENT_SEPARATION_FACTOR; // 1.0E-3;

	/**
	 * Factor which controls how close curve vertices on inside turns
	 * can be to be snapped
	 */
	static const double INSIDE_TURN_VERTEX_SNAP_DISTANCE_FACTOR; // 1.0E-3;

	/**
	 * Factor which controls how close curve vertices can be to be snapped
	 */
	static const double CURVE_VERTEX_SNAP_DISTANCE_FACTOR; //  1.0E-6;

	/**
	 * Factor which determines how short closing segs can be for round buffers
	 */
	static const int MAX_CLOSING_SEG_LEN_FACTOR = 80;

	/** \brief
	 * the max error of approximation (distance) between a quad segment and
	 * the true fillet curve
	 */
	double maxCurveSegmentError; // 0.0

	/** \brief
	 * The angle quantum with which to approximate a fillet curve
	 * (based on the input # of quadrant segments)
	 */
	double filletAngleQuantum;

	/// The Closing Segment Factor controls how long "closing
	/// segments" are.  Closing segments are added at the middle of
	/// inside corners to ensure a smoother boundary for the buffer
	/// offset curve.  In some cases (particularly for round joins
	/// with default-or-better quantization) the closing segments
	/// can be made quite short.  This substantially improves
	/// performance (due to fewer intersections being created).
	///
	/// A closingSegFactor of 0 results in lines to the corner vertex.
	/// A closingSegFactor of 1 results in lines halfway
	/// to the corner vertex.
	/// A closingSegFactor of 80 results in lines 1/81 of the way
	/// to the corner vertex (this option is reasonable for the very
	/// common default situation of round joins and quadrantSegs >= 8).
	///
	/// The default is 1.
	///
	int closingSegLengthFactor; // 1;

	/// Owned by this object, destroyed by dtor
	///
	/// This actually gets created multiple times
	/// and each of the old versions is pushed
	/// to the ptLists std::vector to ensure all
	/// created CoordinateSequences are properly
	/// destroyed.
	///
	OffsetSegmentString segList;

	double distance;

	const geom::PrecisionModel *precisionModel;

	const BufferParameters &bufParams;

	algorithm::LineIntersector li;

	geom::Coordinate s0, s1, s2;

	geom::LineSegment seg0;

	geom::LineSegment seg1;

	geom::LineSegment offset0;

	geom::LineSegment offset1;

	int side;

	bool _hasNarrowConcaveAngle; // =false

	/**
	 * Use a value which results in a potential distance error which is
	 * significantly less than the error due to
	 * the quadrant segment discretization.
	 * For QS = 8 a value of 100 is reasonable.
	 * This should produce a maximum of 1% distance error.
	 */
	static const double SIMPLIFY_FACTOR; // 100.0;

	static const double PI; //  3.14159265358979

	// Not in JTS, used for single-sided buffers
	int endCapIndex;

	void addCollinear(bool addStartPoint);

	/// Adds the offset points for an outside (convex) turn
	///
	/// @param orientation
	/// @param addStartPoint
	///
	void addOutsideTurn(int orientation, bool addStartPoint);

	/// Adds the offset points for an inside (concave) turn
	///
	/// @param orientation
	/// @param addStartPoint
	///
	void addInsideTurn(int orientation, bool addStartPoint);

	/// The mitre will be beveled if it exceeds the mitre ratio limit.
	///
	/// @param offset0 the first offset segment
	/// @param offset1 the second offset segment
	/// @param distance the offset distance
	///
	void addMitreJoin(const geom::Coordinate &cornerPt, const geom::LineSegment &offset0,
	                  const geom::LineSegment &offset1, double distance);

	/// Adds a limited mitre join connecting two convex offset segments.
	/// A limited mitre join is beveled at the distance
	/// determined by the mitre limit factor,
	/// or as a standard bevel join, whichever is further.
	///
	/// @param offset0 the first offset segment
	/// @param offset1 the second offset segment
	/// @param distance the offset distance
	/// @param mitreLimitDistance the mitre limit ratio
	///
	void addLimitedMitreJoin(const geom::LineSegment &offset0, const geom::LineSegment &offset1, double distance,
	                         double mitreLimitDistance);

	/// \brief
	/// Adds a bevel join connecting the two offset segments
	/// around a reflex corner.
	///
	/// @param offset0 the first offset segment
	/// @param offset1 the second offset segment
	///
	void addBevelJoin(const geom::LineSegment &offset0, const geom::LineSegment &offset1);

	void init(double newDistance);

	/**
	 * Adds a bevel join connecting the two offset segments
	 * around a reflex corner.
	 * Projects a point to a given distance in a given direction angle.
	 *
	 * @param pt the point to project
	 * @param d the projection distance
	 * @param dir the direction angle (in radians)
	 * @return the projected point
	 */
	static geom::Coordinate project(const geom::Coordinate &pt, double d, double dir);

	/**
	 * Extends a line segment forwards or backwards a given distance.
	 *
	 * @param seg the base line segment
	 * @param dist the distance to extend by
	 * @return the extended segment
	 */
	static geom::LineSegment extend(const geom::LineSegment &seg, double dist);
};

} // namespace buffer
} // namespace operation
} // namespace geos
