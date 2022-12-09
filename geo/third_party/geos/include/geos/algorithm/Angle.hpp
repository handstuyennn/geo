/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2011  Sandro Santilli <strk@kbt.io>
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

#pragma once

#include <geos/algorithm/Orientation.hpp> // for constants
#include <geos/export.hpp>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
} // namespace geos

namespace geos {
namespace algorithm { // geos::algorithm

/// Utility functions for working with angles.
//
/// Unless otherwise noted, methods in this class express angles in radians.
///
class GEOS_DLL Angle {
public:
	static constexpr double PI_TIMES_2 = 2.0 * MATH_PI;
	static constexpr double PI_OVER_2 = MATH_PI / 2.0;
	static constexpr double PI_OVER_4 = MATH_PI / 4.0;

	/// Constant representing counterclockwise orientation
	static const int COUNTERCLOCKWISE = Orientation::COUNTERCLOCKWISE;

	/// Constant representing clockwise orientation
	static const int CLOCKWISE = Orientation::CLOCKWISE;

	/// Constant representing no orientation
	static const int NONE = Orientation::COLLINEAR;

	/// \brief
	/// Returns the angle of the vector from p0 to p1,
	/// relative to the positive X-axis.
	///
	/// The angle is normalized to be in the range [ -Pi, Pi ].
	///
	/// @return the normalized angle (in radians) that p0-p1 makes
	///         with the positive x-axis.
	///
	static double angle(const geom::CoordinateXY &p0, const geom::CoordinateXY &p1);

	/// \brief
	/// Returns the angle that the vector from (0,0) to p,
	/// relative to the positive X-axis.
	//
	/// The angle is normalized to be in the range ( -Pi, Pi ].
	///
	/// @return the normalized angle (in radians) that p makes
	///          with the positive x-axis.
	///
	static double angle(const geom::CoordinateXY &p);

	/// \brief
	/// Computes the normalized value of an angle, which is the
	/// equivalent angle in the range ( -Pi, Pi ].
	///
	/// @param angle the angle to normalize
	/// @return an equivalent angle in the range (-Pi, Pi]
	///
	static double normalize(double angle);

	/// Returns the oriented smallest angle between two vectors.
	///
	/// The computed angle will be in the range (-Pi, Pi].
	/// A positive result corresponds to a counterclockwise rotation
	/// from v1 to v2;
	/// a negative result corresponds to a clockwise rotation.
	///
	/// @param tip1 the tip of v1
	/// @param tail the tail of each vector
	/// @param tip2 the tip of v2
	/// @return the angle between v1 and v2, relative to v1
	///
	static double angleBetweenOriented(const geom::CoordinateXY &tip1, const geom::CoordinateXY &tail,
	                                   const geom::CoordinateXY &tip2);
};

} // namespace algorithm
} // namespace geos
