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

#include <cmath>
#include <exception>
#include <geos/algorithm/Distance.hpp>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/noding/NodedSegmentString.hpp>
#include <geos/noding/SegmentString.hpp>
#include <geos/noding/snap/SnappingIntersectionAdder.hpp>
#include <geos/noding/snap/SnappingPointIndex.hpp>
#include <geos/util.hpp>
#include <iostream>
#include <vector>

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snap {   // geos.noding.snap

SnappingIntersectionAdder::SnappingIntersectionAdder(double p_snapTolerance, SnappingPointIndex &p_snapPointIndex)
    : SegmentIntersector(), snapTolerance(p_snapTolerance), snapPointIndex(p_snapPointIndex) {
}

/*public*/
void SnappingIntersectionAdder::processIntersections(SegmentString *seg0, std::size_t segIndex0, SegmentString *seg1,
                                                     std::size_t segIndex1) {
	// don't bother intersecting a segment with itself
	if (seg0 == seg1 && segIndex0 == segIndex1)
		return;

	const Coordinate &p00 = seg0->getCoordinate(segIndex0);
	const Coordinate &p01 = seg0->getCoordinate(segIndex0 + 1);
	const Coordinate &p10 = seg1->getCoordinate(segIndex1);
	const Coordinate &p11 = seg1->getCoordinate(segIndex1 + 1);

	/**
	 * Don't node intersections which are just
	 * due to the shared vertex of adjacent segments.
	 */
	if (!isAdjacent(seg0, segIndex0, seg1, segIndex1)) {
		li.computeIntersection(p00, p01, p10, p11);
		/**
		 * Process single point intersections only.
		 * Two-point (collinear) ones are handled by the near-vertex code
		 */
		if (li.hasIntersection() && li.getIntersectionNum() == 1) {

			const Coordinate &intPt = li.getIntersection(0);
			const Coordinate &snapPt = snapPointIndex.snap(intPt);

			static_cast<NodedSegmentString *>(seg0)->addIntersection(snapPt, segIndex0);
			static_cast<NodedSegmentString *>(seg1)->addIntersection(snapPt, segIndex1);
		}
	}

	/**
	 * The segments must also be snapped to the other segment endpoints.
	 */
	processNearVertex(seg0, segIndex0, p00, seg1, segIndex1, p10, p11);
	processNearVertex(seg0, segIndex0, p01, seg1, segIndex1, p10, p11);
	processNearVertex(seg1, segIndex1, p10, seg0, segIndex0, p00, p01);
	processNearVertex(seg1, segIndex1, p11, seg0, segIndex0, p00, p01);
}

/*private*/
void SnappingIntersectionAdder::processNearVertex(SegmentString *srcSS, std::size_t srcIndex, const geom::Coordinate &p,
                                                  SegmentString *ss, std::size_t segIndex, const geom::Coordinate &p0,
                                                  const geom::Coordinate &p1) {
	/**
	 * Don't add intersection if candidate vertex is near endpoints of segment.
	 * This avoids creating "zig-zag" linework
	 * (since the vertex could actually be outside the segment envelope).
	 * Also, this should have already been snapped.
	 */
	if (p.distance(p0) < snapTolerance)
		return;
	if (p.distance(p1) < snapTolerance)
		return;

	double distSeg = algorithm::Distance::pointToSegment(p, p0, p1);
	if (distSeg < snapTolerance) {
		// add node to target segment
		static_cast<NodedSegmentString *>(ss)->addIntersection(p, segIndex);
		// add node at vertex to source SS
		static_cast<NodedSegmentString *>(srcSS)->addIntersection(p, srcIndex);
	}
}

/*private static*/
bool SnappingIntersectionAdder::isAdjacent(SegmentString *ss0, std::size_t segIndex0, SegmentString *ss1,
                                           std::size_t segIndex1) {
	if (ss0 != ss1)
		return false;
	long l0 = static_cast<long>(segIndex0);
	long l1 = static_cast<long>(segIndex1);

	bool isAdjacent = (std::abs(l0 - l1) == 1);
	if (isAdjacent) {
		return true;
	}
	if (ss0->isClosed()) {
		std::size_t maxSegIndex = ss0->size();
		if ((segIndex0 == 0 && (segIndex1 + 1) == maxSegIndex) || (segIndex1 == 0 && (segIndex0 + 1) == maxSegIndex)) {
			return true;
		}
	}
	return false;
}

} // namespace snap
} // namespace noding
} // namespace geos
