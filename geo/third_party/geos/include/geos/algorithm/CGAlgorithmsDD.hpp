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

#pragma once

#include <geos/export.hpp>
#include <geos/math/DD.hpp>

// Forward declarations
namespace geos {
namespace geom {
class CoordinateXY;
class CoordinateSequence;
} // namespace geom
} // namespace geos

using namespace geos::math;

namespace geos {
namespace algorithm { // geos::algorithm

/// Implements basic computational geometry algorithms using extended precision float-point arithmetic.
class GEOS_DLL CGAlgorithmsDD {
public:
	enum { CLOCKWISE = -1, COLLINEAR = 0, COUNTERCLOCKWISE = 1 };

	enum { RIGHT = -1, LEFT = 1, STRAIGHT = 0, FAILURE = 2 };

	/** \brief
	 * Returns the index of the direction of the point `q` relative to
	 * a vector specified by `p1-p2`.
	 *
	 * @param p1 the origin point of the vector
	 * @param p2 the final point of the vector
	 * @param q the point to compute the direction to
	 *
	 * @return 1 if q is counter-clockwise (left) from p1-p2
	 * @return -1 if q is clockwise (right) from p1-p2
	 * @return 0 if q is collinear with p1-p2
	 */
	static int orientationIndex(const geom::CoordinateXY &p1, const geom::CoordinateXY &p2,
	                            const geom::CoordinateXY &q);

	static int orientationIndex(double p1x, double p1y, double p2x, double p2y, double qx, double qy);

	/**
	 * A filter for computing the orientation index of three coordinates.
	 *
	 * If the orientation can be computed safely using standard DP arithmetic,
	 * this routine returns the orientation index. Otherwise, a value `i > 1` is
	 * returned. In this case the orientation index must be computed using some
	 * other more robust method.
	 *
	 * The filter is fast to compute, so can be used to avoid the use of slower
	 * robust methods except when they are really needed, thus providing better
	 * average performance.
	 *
	 * Uses an approach due to Jonathan Shewchuk, which is in the public domain.
	 */
	static int orientationIndexFilter(double pax, double pay, double pbx, double pby, double pcx, double pcy) {
		/**
		 * A value which is safely greater than the relative round-off
		 * error in double-precision numbers
		 */
		double constexpr DP_SAFE_EPSILON = 1e-15;

		double detsum;
		double const detleft = (pax - pcx) * (pby - pcy);
		double const detright = (pay - pcy) * (pbx - pcx);
		double const det = detleft - detright;

		if (detleft > 0.0) {
			if (detright <= 0.0) {
				return orientation(det);
			} else {
				detsum = detleft + detright;
			}
		} else if (detleft < 0.0) {
			if (detright >= 0.0) {
				return orientation(det);
			} else {
				detsum = -detleft - detright;
			}
		} else {
			return orientation(det);
		}

		double const errbound = DP_SAFE_EPSILON * detsum;
		if ((det >= errbound) || (-det >= errbound)) {
			return orientation(det);
		}
		return CGAlgorithmsDD::FAILURE;
	};

	static int orientation(double x) {
		if (x < 0) {
			return CGAlgorithmsDD::RIGHT;
		}
		if (x > 0) {
			return CGAlgorithmsDD::LEFT;
		}
		return CGAlgorithmsDD::STRAIGHT;
	};
};

} // namespace algorithm
} // namespace geos
