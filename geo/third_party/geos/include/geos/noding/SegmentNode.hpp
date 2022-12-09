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
 * Last port: noding/SegmentNode.java 4667170ea (JTS-1.17)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/noding/SegmentNode.hpp>
#include <geos/noding/SegmentPointComparator.hpp>
#include <iostream>
#include <vector>

// Forward declarations
namespace geos {
namespace noding {
class NodedSegmentString;
}
} // namespace geos

namespace geos {
namespace noding { // geos.noding

/**
 * \brief
 * Represents an intersection point between two NodedSegmentString.
 *
 * Final class.
 */
class GEOS_DLL SegmentNode {

private:
	// const NodedSegmentString* segString;

	int segmentOctant;

	bool isInteriorVar;

public:
	/// the point of intersection (own copy)
	geom::Coordinate coord;

	/// the index of the containing line segment in the parent edge
	std::size_t segmentIndex;

	/// Construct a node on the given NodedSegmentString
	///
	/// @param ss the parent NodedSegmentString
	///
	/// @param nCoord the coordinate of the intersection, will be copied
	///
	/// @param nSegmentIndex the index of the segment on parent
	///                      NodedSegmentString
	///        where the Node is located.
	///
	/// @param nSegmentOctant
	///
	SegmentNode(const NodedSegmentString &ss, const geom::Coordinate &nCoord, std::size_t nSegmentIndex,
	            int nSegmentOctant);

	~SegmentNode() {
	}

	/// \brief
	/// Return true if this Node is *internal* (not on the boundary)
	/// of the corresponding segment. Currently only the *first*
	/// segment endpoint is checked, actually.
	///
	bool isInterior() const {
		return isInteriorVar;
	}

	/**
	 * @return -1 this EdgeIntersection is located before
	 *            the argument location
	 * @return 0 this EdgeIntersection is at the argument location
	 * @return 1 this EdgeIntersection is located after the
	 *           argument location
	 */
	int compareTo(const SegmentNode &other) const {
		if (segmentIndex < other.segmentIndex) {
			return -1;
		}
		if (segmentIndex > other.segmentIndex) {
			return 1;
		}

		if (coord.equals2D(other.coord)) {

			return 0;
		}

		// an exterior node is the segment start point,
		// so always sorts first
		// this guards against a robustness problem
		// where the octants are not reliable
		if (!isInteriorVar)
			return -1;
		if (!other.isInteriorVar)
			return 1;

		return SegmentPointComparator::compare(segmentOctant, coord, other.coord);
	};
};

} // namespace noding
} // namespace geos
