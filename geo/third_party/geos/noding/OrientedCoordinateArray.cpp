/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009    Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/OrientedCoordinateArray.java rev. 1.1 (JTS-1.9)
 *
 **********************************************************************/

// #include <cmath>
// #include <sstream>

#include <geos/geom/CoordinateSequence.hpp>
#include <geos/noding/OrientedCoordinateArray.hpp>

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/* private static */
bool OrientedCoordinateArray::orientation(const CoordinateSequence &pts) {
	return CoordinateSequence::increasingDirection(pts) == 1;
}

bool OrientedCoordinateArray::operator==(const OrientedCoordinateArray &other) const {
	auto sz1 = pts->size();
	auto sz2 = other.pts->size();

	if (sz1 != sz2) {
		return false;
	}

	if (orientationVar == other.orientationVar) {
		for (std::size_t i = 0; i < sz1; i++) {
			if (pts->getAt(i) != other.pts->getAt(i)) {
				return false;
			}
		}
	} else {
		for (std::size_t i = 0; i < sz1; i++) {
			if (pts->getAt(i) != other.pts->getAt(sz2 - i - 1)) {
				return false;
			}
		}
	}

	return true;
}

size_t OrientedCoordinateArray::HashCode::operator()(const geos::noding::OrientedCoordinateArray &oca) const {
	Coordinate::HashCode coordHash;

	auto sz = oca.pts->getSize();

	std::size_t result = std::hash<size_t> {}(sz);

	if (oca.orientationVar) {
		for (std::size_t i = 0; i < sz; i++) {
			result ^= coordHash(oca.pts->getAt(i));
		}
	} else {
		for (std::size_t i = sz; i > 0; i--) {
			result ^= coordHash(oca.pts->getAt(i - 1));
		}
	}

	return result;
}

} // namespace noding
} // namespace geos
