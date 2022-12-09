/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2011 Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Angle.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <cmath>
#include <geos/algorithm/Angle.hpp>
#include <geos/geom/Coordinate.hpp>

namespace geos {
namespace algorithm { // geos.algorithm

/* public static */
double Angle::angle(const geom::CoordinateXY &p0, const geom::CoordinateXY &p1) {
	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;
	return atan2(dy, dx);
}

/* public static */
double Angle::angle(const geom::CoordinateXY &p) {
	return atan2(p.y, p.x);
}

/* public static */
double Angle::normalize(double angle) {
	while (angle > MATH_PI) {
		angle -= PI_TIMES_2;
	}
	while (angle <= -MATH_PI) {
		angle += PI_TIMES_2;
	}
	return angle;
}

/* public static */
double Angle::angleBetweenOriented(const geom::CoordinateXY &tip1, const geom::CoordinateXY &tail,
                                   const geom::CoordinateXY &tip2) {
	double a1 = angle(tail, tip1);
	double a2 = angle(tail, tip2);
	double angDel = a2 - a1;

	// normalize, maintaining orientation
	if (angDel <= -MATH_PI) {
		return angDel + PI_TIMES_2;
	}
	if (angDel > MATH_PI) {
		return angDel - PI_TIMES_2;
	}
	return angDel;
}

} // namespace algorithm
} // namespace geos
