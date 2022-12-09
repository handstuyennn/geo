/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateFilter.hpp>
#include <map>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class Envelope;
class PrecisionModel;
} // namespace geom
namespace operation {}
} // namespace geos

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;

/**
 * Unions a collection of geometries in an
 * efficient way, using {@link OverlayNG}
 * to ensure robust computation.
 * @author Martin Davis
 */
class GEOS_DLL PrecisionUtil {
private:
	/**
	 * Determines the maximum magnitude (absolute value) of the bounds of an
	 * of an envelope.
	 * This is equal to the largest ordinate value
	 * which must be accommodated by a scale factor.
	 *
	 */
	static double maxBoundMagnitude(const Envelope *env);

	/**
	 * Computes the scale factor which will
	 * produce a given number of digits of precision (significant digits)
	 * when used to round the given number.
	 *
	 * For example: to provide 5 decimal digits of precision
	 * for the number 123.456 the precision scale factor is 100;
	 * for 3 digits of precision the scale factor is 1;
	 * for 2 digits of precision the scale factor is 0.1.
	 *
	 * Rounding to the scale factor can be performed with {@link PrecisionModel#round}
	 *
	 * @see PrecisionModel.round
	 */
	static double precisionScale(double value, int precisionDigits);

public:
	static constexpr int MAX_ROBUST_DP_DIGITS = 14;

	PrecisionUtil() {};

	/**
	 * Computes a safe scale factor for a numeric value.
	 * A safe scale factor ensures that rounded
	 * number has no more than MAX_PRECISION_DIGITS
	 * digits of precision.
	 */
	static double safeScale(double value);

	/**
	 * Computes a safe scale factor for a geometry.
	 * A safe scale factor ensures that the rounded
	 * ordinates have no more than MAX_PRECISION_DIGITS
	 * digits of precision.
	 */
	static double safeScale(const Geometry *geom);

	/**
	 * Computes a safe scale factor for two geometries.
	 * A safe scale factor ensures that the rounded
	 * ordinates have no more than MAX_PRECISION_DIGITS
	 * digits of precision.
	 */
	static double safeScale(const Geometry *a, const Geometry *b);
};

} // namespace overlayng
} // namespace operation
} // namespace geos
