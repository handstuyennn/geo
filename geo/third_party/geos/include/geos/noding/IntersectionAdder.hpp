/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/IntersectionAdder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#pragma once

#include <cstdlib> // for abs()
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/noding/SegmentIntersector.hpp> // for inheritance
#include <iostream>
#include <vector>

// Forward declarations
namespace geos {
namespace noding {
class SegmentString;
}
namespace algorithm {
class LineIntersector;
}
} // namespace geos

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Computes the intersections between two line segments in SegmentString
 * and adds them to each string.
 *
 * The SegmentIntersector is passed to a Noder.
 * The NodedSegmentString::addIntersections(algorithm::LineIntersector* li, std::size_t segmentIndex, std::size_t
 * geomIndex) method is called whenever the Noder detects that two SegmentStrings *might* intersect. This class is an
 * example of the *Strategy* pattern.
 *
 */
class GEOS_DLL IntersectionAdder : public SegmentIntersector {
private:
	/**
	 * These variables keep track of what types of intersections were
	 * found during ALL edges that have been intersected.
	 */
	bool hasIntersectionVar;
	bool hasProper;
	bool hasProperInterior;
	bool hasInterior;

	// the proper intersection point found
	geom::Coordinate properIntersectionPoint;

	algorithm::LineIntersector &li;

	/**
	 * A trivial intersection is an apparent self-intersection which
	 * in fact is simply the point shared by adjacent line segments.
	 * Note that closed edges require a special check for the point
	 * shared by the beginning and end segments.
	 */
	bool isTrivialIntersection(const SegmentString *e0, std::size_t segIndex0, const SegmentString *e1,
	                           std::size_t segIndex1);

public:
	int numIntersections;
	int numInteriorIntersections;
	int numProperIntersections;

	// testing only
	int numTests;

	IntersectionAdder(algorithm::LineIntersector &newLi)
	    : hasIntersectionVar(false), hasProper(false), hasProperInterior(false), hasInterior(false),
	      properIntersectionPoint(), li(newLi), numIntersections(0), numInteriorIntersections(0),
	      numProperIntersections(0), numTests(0) {
	}

	/** \brief
	 * This method is called by clients of the SegmentIntersector class to
	 * process intersections for two segments of the SegmentStrings being intersected.
	 *
	 * Note that some clients (such as MonotoneChains) may optimize away
	 * this call for segment pairs which they have determined do not
	 * intersect (e.g. by an disjoint envelope test).
	 */
	void processIntersections(SegmentString *e0, std::size_t segIndex0, SegmentString *e1,
	                          std::size_t segIndex1) override;

	static bool isAdjacentSegments(std::size_t i1, std::size_t i2) {
		return (i1 > i2 ? i1 - i2 : i2 - i1) == 1;
	}
};

} // namespace noding
} // namespace geos
