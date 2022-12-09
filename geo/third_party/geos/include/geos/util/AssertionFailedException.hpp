/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/util/GEOSException.hpp>
#include <string>

namespace geos {
namespace util { // geos.util

/** \class AssertionFailedException util.h geos.h
 * \brief Indicates a bug in GEOS code.
 */
class GEOS_DLL AssertionFailedException : public GEOSException {

public:
	AssertionFailedException() : GEOSException("AssertionFailedException", "") {
	}

	AssertionFailedException(const std::string &msg) : GEOSException("AssertionFailedException", msg) {
	}

	~AssertionFailedException() noexcept override {
	}
};

} // namespace util
} // namespace geos
