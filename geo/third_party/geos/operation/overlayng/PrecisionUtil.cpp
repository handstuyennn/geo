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

#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateFilter.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/PrecisionModel.hpp>
#include <geos/operation/overlayng/PrecisionUtil.hpp>
#include <sstream>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;

/*private static*/
double PrecisionUtil::precisionScale(double value, int precisionDigits) {
	// the smallest power of 10 greater than the value
	int magnitude = (int)(std::log(value) / std::log(10) + 1.0);
	int precDigits = precisionDigits - magnitude;

	double scaleFactor = std::pow(10.0, precDigits);
	return scaleFactor;
}

/*private static*/
double PrecisionUtil::maxBoundMagnitude(const Envelope *env) {
	return std::max(std::max(std::abs(env->getMaxX()), std::abs(env->getMaxY())),
	                std::max(std::abs(env->getMinX()), std::abs(env->getMinY())));
}

/*public static*/
double PrecisionUtil::safeScale(double value) {
	return precisionScale(value, MAX_ROBUST_DP_DIGITS);
}

/*public static*/
double PrecisionUtil::safeScale(const Geometry *geom) {
	return safeScale(maxBoundMagnitude(geom->getEnvelopeInternal()));
}

/*public static*/
double PrecisionUtil::safeScale(const Geometry *a, const Geometry *b) {
	double maxBnd = maxBoundMagnitude(a->getEnvelopeInternal());
	if (b != nullptr) {
		double maxBndB = maxBoundMagnitude(b->getEnvelopeInternal());
		maxBnd = std::max(maxBnd, maxBndB);
	}
	double scale = PrecisionUtil::safeScale(maxBnd);
	return scale;
}
} // namespace overlayng
} // namespace operation
} // namespace geos
