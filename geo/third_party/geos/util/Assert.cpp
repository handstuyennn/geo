/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/util/Assert.hpp>
#include <geos/util/AssertionFailedException.hpp>
#include <string>

using std::string;
using namespace geos::geom;

namespace geos {
namespace util { // geos.util

void Assert::isTrue(bool assertion, const std::string &message) {
	if (!assertion) {
		if (message.empty()) {
			throw AssertionFailedException();
		} else {
			throw AssertionFailedException(message);
		}
	}
}

void Assert::equals(const CoordinateXY &expectedValue, const CoordinateXY &actualValue, const std::string &message) {
	if (!(actualValue == expectedValue)) {
		throw AssertionFailedException("Expected Coordinate but encountered Coordinate" +
		                               (!message.empty() ? ": " + message : ""));
	}
}

void Assert::shouldNeverReachHere(const std::string &message) {
	throw AssertionFailedException("Should never reach here" + (!message.empty() ? ": " + message : ""));
}

} // namespace util
} // namespace geos
