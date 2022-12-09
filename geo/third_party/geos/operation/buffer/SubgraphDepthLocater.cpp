/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/SubgraphDepthLocater.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <cassert>
#include <geos/algorithm/Orientation.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/Position.hpp>
#include <geos/geomgraph/DirectedEdge.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/operation/buffer/BufferSubgraph.hpp>
#include <geos/operation/buffer/SubgraphDepthLocater.hpp>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

/*
 * A segment from a directed edge which has been assigned a depth value
 * for its sides.
 */
class DepthSegment {

private:
	geom::LineSegment upwardSeg;

public:
	int leftDepth;

	/// @param seg will be copied to private space
	DepthSegment(const geom::LineSegment &seg, int depth) : upwardSeg(seg), leftDepth(depth) {
		// input seg is assumed to be normalized
		// upwardSeg.normalize();
	}

	/**
	 * Defines a comparision operation on DepthSegments
	 * which orders them left to right
	 *
	 * <pre>
	 * DS1 < DS2   if   DS1.seg is left of DS2.seg
	 * DS1 > DS2   if   DS1.seg is right of DS2.seg
	 * </pre>
	 *
	 * @param other
	 * @return
	 */
	int compareTo(const DepthSegment &other) const {
		/**
		 * If segment envelopes do not overlap, then
		 * can use standard segment lexicographic ordering.
		 */
		if (upwardSeg.minX() >= other.upwardSeg.maxX() || upwardSeg.maxX() <= other.upwardSeg.minX() ||
		    upwardSeg.minY() >= other.upwardSeg.maxY() || upwardSeg.maxY() <= other.upwardSeg.minY()) {
			return upwardSeg.compareTo(other.upwardSeg);
		};

		/**
		 * Otherwise if envelopes overlap, use relative segment orientation.
		 *
		 * Collinear segments should be evaluated by previous logic
		 */
		int orientIndex = upwardSeg.orientationIndex(&(other.upwardSeg));

		/*
		 * If comparison between this and other is indeterminate,
		 * try the opposite call order.
		 * orientationIndex value is 1 if this is left of other,
		 * so have to flip sign to get proper comparison value of
		 * -1 if this is leftmost
		 */
		if (orientIndex == 0) {
			orientIndex = -1 * other.upwardSeg.orientationIndex(&upwardSeg);
		}

		// if orientation is determinate, return it
		if (orientIndex != 0) {
			return orientIndex;
		}

		/**
		 * If segment envelopes overlap and they are collinear,
		 * since segments do not cross they must be equal.
		 */
		// assert: segments are equal
		return 0;
	}
};

struct DepthSegmentLessThan {
	bool operator()(const DepthSegment *first, const DepthSegment *second) {
		assert(first);
		assert(second);
		if (first->compareTo(*second) < 0) {
			return true;
		} else {
			return false;
		}
	}
};

/*public*/
int SubgraphDepthLocater::getDepth(const Coordinate &p) {
	std::vector<DepthSegment *> stabbedSegments;
	findStabbedSegments(p, stabbedSegments);

	// if no segments on stabbing line subgraph must be outside all others
	if (stabbedSegments.empty()) {
		return 0;
	}

	DepthSegment *ds = *std::min_element(stabbedSegments.begin(), stabbedSegments.end(), DepthSegmentLessThan());
	int ret = ds->leftDepth;

	for (std::vector<DepthSegment *>::iterator it = stabbedSegments.begin(), itEnd = stabbedSegments.end(); it != itEnd;
	     ++it) {
		delete *it;
	}

	return ret;
}

/*private*/
void SubgraphDepthLocater::findStabbedSegments(const Coordinate &stabbingRayLeftPt,
                                               std::vector<DepthSegment *> &stabbedSegments) {
	std::size_t size = subgraphs->size();
	for (std::size_t i = 0; i < size; ++i) {
		BufferSubgraph *bsg = (*subgraphs)[i];

		// optimization - don't bother checking subgraphs
		// which the ray does not intersect
		const Envelope *env = bsg->getEnvelope();
		if (stabbingRayLeftPt.y < env->getMinY() || stabbingRayLeftPt.y > env->getMaxY() ||
		    stabbingRayLeftPt.x > env->getMaxX()) {
			continue;
		}

		findStabbedSegments(stabbingRayLeftPt, bsg->getDirectedEdges(), stabbedSegments);
	}
}

/*private*/
void SubgraphDepthLocater::findStabbedSegments(const Coordinate &stabbingRayLeftPt,
                                               std::vector<DirectedEdge *> *dirEdges,
                                               std::vector<DepthSegment *> &stabbedSegments) {

	/*
	 * Check all forward DirectedEdges only. This is still general,
	 * because each Edge has a forward DirectedEdge.
	 */
	for (std::size_t i = 0, n = dirEdges->size(); i < n; ++i) {
		DirectedEdge *de = (*dirEdges)[i];
		if (!de->isForward()) {
			continue;
		}

		const Envelope *env = de->getEdge()->getEnvelope();
		if (stabbingRayLeftPt.y < env->getMinY() || stabbingRayLeftPt.y > env->getMaxY() ||
		    stabbingRayLeftPt.x > env->getMaxX()) {
			continue;
		}

		findStabbedSegments(stabbingRayLeftPt, de, stabbedSegments);
	}
}

/*private*/
void SubgraphDepthLocater::findStabbedSegments(const Coordinate &stabbingRayLeftPt, DirectedEdge *dirEdge,
                                               std::vector<DepthSegment *> &stabbedSegments) {
	const CoordinateSequence *pts = dirEdge->getEdge()->getCoordinates();

// It seems that LineSegment is *very* slow... undef this
// to see yourself
// LineSegment has been refactored to be mostly inline, still
// it makes copies of the given coordinates, while the 'non-LineSegment'
// based code below uses pointers instead. I'll kip the SKIP_LS
// defined until LineSegment switches to Coordinate pointers instead.
//
#define SKIP_LS 1

	auto n = pts->getSize() - 1;
	for (std::size_t i = 0; i < n; ++i) {
		const Coordinate *low = &(pts->getAt(i));
		const Coordinate *high = &(pts->getAt(i + 1));
		const Coordinate *swap = nullptr;

		if (low->y > high->y) {
			swap = low;
			low = high;
			high = swap;
		}

		// skip segment if it is left of the stabbing line
		// skip if segment is above or below stabbing line
		double maxx = std::max(low->x, high->x);
		if (maxx < stabbingRayLeftPt.x) {
			continue;
		}

		// skip horizontal segments (there will be a non-horizontal
		// one carrying the same depth info
		if (low->y == high->y) {
			continue;
		}

		// skip if segment is above or below stabbing line
		if (stabbingRayLeftPt.y < low->y || stabbingRayLeftPt.y > high->y) {
			continue;
		}

		// skip if stabbing ray is right of the segment
		if (Orientation::index(*low, *high, stabbingRayLeftPt) == Orientation::RIGHT) {
			continue;
		}

		int depth = swap ? dirEdge->getDepth(Position::RIGHT) : dirEdge->getDepth(Position::LEFT);

		seg.p0 = *low;
		seg.p1 = *high;

		DepthSegment *ds = new DepthSegment(seg, depth);
		stabbedSegments.push_back(ds);
	}
}

} // namespace buffer
} // namespace operation
} // namespace geos
