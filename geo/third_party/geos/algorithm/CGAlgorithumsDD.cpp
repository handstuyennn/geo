/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2014 Mateusz Loskot <mateusz@loskot.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/CGAlgorithmsDD.java r789 (JTS-1.14)
 *
 **********************************************************************/

#include <cmath>
#include <geos/algorithm/CGAlgorithmsDD.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <sstream>

using namespace geos::geom;
using namespace geos::algorithm;

namespace {

inline int OrientationDD(const DD &dd) {
	static DD const zero(0.0);
	if (dd < zero) {
		return CGAlgorithmsDD::RIGHT;
	}

	if (dd > zero) {
		return CGAlgorithmsDD::LEFT;
	}

	return CGAlgorithmsDD::STRAIGHT;
}

} // namespace

namespace geos {
namespace algorithm { // geos::algorithm

int CGAlgorithmsDD::orientationIndex(double p1x, double p1y, double p2x, double p2y, double qx, double qy) {
	if (!std::isfinite(qx) || !std::isfinite(qy)) {
		throw util::IllegalArgumentException("CGAlgorithmsDD::orientationIndex encountered NaN/Inf numbers");
	}

	// fast filter for orientation index
	// avoids use of slow extended-precision arithmetic in many cases
	int index = orientationIndexFilter(p1x, p1y, p2x, p2y, qx, qy);
	if (index <= 1) {
		return index;
	}

	// normalize coordinates
	DD dx1 = DD(p2x) + DD(-p1x);
	DD dy1 = DD(p2y) + DD(-p1y);
	DD dx2 = DD(qx) + DD(-p2x);
	DD dy2 = DD(qy) + DD(-p2y);

	// sign of determinant - inlined for performance
	DD mx1y2(dx1 * dy2);
	DD my1x2(dy1 * dx2);
	DD d = mx1y2 - my1x2;
	return OrientationDD(d);
}

// inlining this method worsened performance slighly
int CGAlgorithmsDD::orientationIndex(const CoordinateXY &p1, const CoordinateXY &p2, const CoordinateXY &q) {

	return orientationIndex(p1.x, p1.y, p2.x, p2.y, q.x, q.y);
}

} // namespace algorithm
} // namespace geos
