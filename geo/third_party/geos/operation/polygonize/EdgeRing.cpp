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
 **********************************************************************
 *
 * Last port: operation/polygonize/EdgeRing.java 0b3c7e3eb0d3e
 *
 **********************************************************************/

#include <cassert>
#include <geos/algorithm/Orientation.hpp>
#include <geos/algorithm/PointLocation.hpp>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateArraySequence.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/Location.hpp>
#include <geos/operation/polygonize/EdgeRing.hpp>
#include <geos/util.hpp> // TODO: drop this, includes too much
#include <geos/util/IllegalArgumentException.hpp>
#include <vector>

// #define DEBUG_ALLOC 1
// #define GEOS_PARANOIA_LEVEL 2

using namespace geos::planargraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation {  // geos.operation
namespace polygonize { // geos.operation.polygonize

/*public static*/
const Coordinate &EdgeRing::ptNotInList(const CoordinateSequence *testPts, const CoordinateSequence *pts) {
	const std::size_t npts = testPts->getSize();
	for (std::size_t i = 0; i < npts; ++i) {
		const Coordinate &testPt = testPts->getAt(i);
		if (!isInList(testPt, pts)) {
			return testPt;
		}
	}
	return Coordinate::getNull();
}

/*public static*/
bool EdgeRing::isInList(const Coordinate &pt, const CoordinateSequence *pts) {
	const std::size_t npts = pts->getSize();
	for (std::size_t i = 0; i < npts; ++i) {
		if (pt == pts->getAt(i)) {
			return true;
		}
	}
	return false;
}

} // namespace polygonize
} // namespace operation
} // namespace geos
