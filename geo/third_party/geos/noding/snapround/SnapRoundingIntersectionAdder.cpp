/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <cassert>
#include <exception>
#include <geos/algorithm/Distance.hpp>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/noding/IntersectionFinderAdder.hpp>
#include <geos/noding/NodedSegmentString.hpp>
#include <geos/noding/NodingValidator.hpp>
#include <geos/noding/SegmentString.hpp>
#include <geos/noding/snapround/HotPixel.hpp>
#include <geos/noding/snapround/SnapRoundingIntersectionAdder.hpp>
#include <geos/util.hpp>
#include <iostream>
#include <vector>

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding {    // geos.noding
namespace snapround { // geos.noding.snapround

/*public*/
void SnapRoundingIntersectionAdder::processIntersections(SegmentString *e0, std::size_t segIndex0, SegmentString *e1,
                                                         std::size_t segIndex1) {
	// don't bother intersecting a segment with itself
	if (e0 == e1 && segIndex0 == segIndex1)
		return;

	const Coordinate &p00 = e0->getCoordinate(segIndex0);
	const Coordinate &p01 = e0->getCoordinate(segIndex0 + 1);
	const Coordinate &p10 = e1->getCoordinate(segIndex1);
	const Coordinate &p11 = e1->getCoordinate(segIndex1 + 1);

	li.computeIntersection(p00, p01, p10, p11);
	if (li.hasIntersection()) {
		if (li.isInteriorIntersection()) {
			for (std::size_t intIndex = 0, intNum = li.getIntersectionNum(); intIndex < intNum; intIndex++) {
				// Take a copy of the intersection coordinate
				intersections->emplace_back(li.getIntersection(intIndex));
			}
			static_cast<NodedSegmentString *>(e0)->addIntersections(&li, segIndex0, 0);
			static_cast<NodedSegmentString *>(e1)->addIntersections(&li, segIndex1, 1);
			return;
		}
	}

	/**
	 * Segments did not actually intersect, within the limits of orientation index robustness.
	 *
	 * To avoid certain robustness issues in snap-rounding,
	 * also treat very near vertex-segment situations as intersections.
	 */
	processNearVertex(p00, e1, segIndex1, p10, p11);
	processNearVertex(p01, e1, segIndex1, p10, p11);
	processNearVertex(p10, e0, segIndex0, p00, p01);
	processNearVertex(p11, e0, segIndex0, p00, p01);
}

/*private*/
void SnapRoundingIntersectionAdder::processNearVertex(const geom::Coordinate &p, SegmentString *edge,
                                                      std::size_t segIndex, const geom::Coordinate &p0,
                                                      const geom::Coordinate &p1) {
	/**
	 * Don't add intersection if candidate vertex is near endpoints of segment.
	 * This avoids creating "zig-zag" linework
	 * (since the vertex could actually be outside the segment envelope).
	 */
	if (p.distance(p0) < nearnessTol)
		return;
	if (p.distance(p1) < nearnessTol)
		return;

	double distSeg = algorithm::Distance::pointToSegment(p, p0, p1);
	if (distSeg < nearnessTol) {
		intersections->emplace_back(p);
		static_cast<NodedSegmentString *>(edge)->addIntersection(p, segIndex);
	}
}

} // namespace snapround
} // namespace noding
} // namespace geos
