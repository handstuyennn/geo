/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/NodedSegmentString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/algorithm/LineIntersector.hpp>
#include <geos/noding/NodedSegmentString.hpp>

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos::noding

const SegmentNodeList &NodedSegmentString::getNodeList() const {
	return nodeList;
}

SegmentNodeList &NodedSegmentString::getNodeList() {
	return nodeList;
}

/* public static */
void NodedSegmentString::getNodedSubstrings(const SegmentString::NonConstVect &segStrings,
                                            SegmentString::NonConstVect *resultEdgeList) {
	assert(resultEdgeList);
	for (SegmentString::NonConstVect::const_iterator i = segStrings.begin(), iEnd = segStrings.end(); i != iEnd; ++i) {
		NodedSegmentString *ss = dynamic_cast<NodedSegmentString *>(*i);
		assert(ss);
		ss->getNodeList().addSplitEdges(resultEdgeList);
	}
}

/* public */
std::vector<Coordinate> NodedSegmentString::getNodedCoordinates() {
	return nodeList.getSplitCoordinates();
}

/* public static */
SegmentString::NonConstVect *NodedSegmentString::getNodedSubstrings(const SegmentString::NonConstVect &segStrings) {
	SegmentString::NonConstVect *resultEdgelist = new SegmentString::NonConstVect();
	getNodedSubstrings(segStrings, resultEdgelist);
	return resultEdgelist;
}

/* virtual public */
const geom::Coordinate &NodedSegmentString::getCoordinate(std::size_t i) const {
	return pts->getAt(i);
}

/* virtual public */
geom::CoordinateSequence *NodedSegmentString::getCoordinates() const {
	return pts.get();
}

geom::CoordinateSequence *NodedSegmentString::releaseCoordinates() {
	return pts.release();
}

/* virtual public */
bool NodedSegmentString::isClosed() const {
	return pts->getAt(0) == pts->getAt(size() - 1);
}

} // namespace noding
} // namespace geos
