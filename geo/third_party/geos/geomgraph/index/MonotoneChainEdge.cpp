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
 **********************************************************************/

#include <cassert>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/index/MonotoneChainEdge.hpp>
#include <geos/geomgraph/index/MonotoneChainIndexer.hpp>
#include <geos/geomgraph/index/SegmentIntersector.hpp>
#include <vector>

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index {     // geos.geomgraph.index

MonotoneChainEdge::MonotoneChainEdge(Edge *newE) : e(newE), pts(newE->getCoordinates()) {

	assert(e);
	MonotoneChainIndexer mci;
	mci.getChainStartIndices(pts, startIndex);
	assert(e);
}

std::vector<size_t> &MonotoneChainEdge::getStartIndexes() {
	return startIndex;
}

double MonotoneChainEdge::getMinX(std::size_t chainIndex) {
	double x1 = pts->getAt(startIndex[chainIndex]).x;
	double x2 = pts->getAt(startIndex[chainIndex + 1]).x;
	return x1 < x2 ? x1 : x2;
}

double MonotoneChainEdge::getMaxX(std::size_t chainIndex) {
	double x1 = pts->getAt(startIndex[chainIndex]).x;
	double x2 = pts->getAt(startIndex[chainIndex + 1]).x;
	return x1 > x2 ? x1 : x2;
}

void MonotoneChainEdge::computeIntersectsForChain(std::size_t chainIndex0, const MonotoneChainEdge &mce,
                                                  std::size_t chainIndex1, SegmentIntersector &si) {
	computeIntersectsForChain(startIndex[chainIndex0], startIndex[chainIndex0 + 1], mce, mce.startIndex[chainIndex1],
	                          mce.startIndex[chainIndex1 + 1], si);
}

void MonotoneChainEdge::computeIntersectsForChain(std::size_t start0, std::size_t end0, const MonotoneChainEdge &mce,
                                                  std::size_t start1, std::size_t end1, SegmentIntersector &ei) {
	// terminating condition for the recursion
	if (end0 - start0 == 1 && end1 - start1 == 1) {
		ei.addIntersections(e, start0, mce.e, start1);
		return;
	}

	if (!overlaps(start0, end0, mce, start1, end1)) {
		return;
	}
	// the chains overlap, so split each in half and iterate
	// (binary search)
	std::size_t mid0 = (start0 + end0) / 2;
	std::size_t mid1 = (start1 + end1) / 2;

	// Assert: mid != start or end
	// (since we checked above for end - start <= 1)
	// check terminating conditions before recursing
	if (start0 < mid0) {
		if (start1 < mid1)
			computeIntersectsForChain(start0, mid0, mce, start1, mid1, ei);
		if (mid1 < end1)
			computeIntersectsForChain(start0, mid0, mce, mid1, end1, ei);
	}
	if (mid0 < end0) {
		if (start1 < mid1)
			computeIntersectsForChain(mid0, end0, mce, start1, mid1, ei);
		if (mid1 < end1)
			computeIntersectsForChain(mid0, end0, mce, mid1, end1, ei);
	}
}

bool MonotoneChainEdge::overlaps(std::size_t start0, std::size_t end0, const MonotoneChainEdge &mce, std::size_t start1,
                                 std::size_t end1) {
	return Envelope::intersects(pts->getAt(start0), pts->getAt(end0), mce.pts->getAt(start1), mce.pts->getAt(end1));
}

} // namespace index
} // namespace geomgraph
} // namespace geos
