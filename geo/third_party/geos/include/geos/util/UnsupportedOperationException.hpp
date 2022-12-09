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
namespace util { // geos::util

/**
 * \class UnsupportedOperationException util.h geos.h
 *
 * \brief Indicates that the requested operation is unsupported.
 *
 * This exception is thrown - for example - when requesting the
 * X or Y member of an empty Point
 */
class GEOS_DLL UnsupportedOperationException : public GEOSException {
public:
	UnsupportedOperationException() : GEOSException("UnsupportedOperationException", "") {
	}

	UnsupportedOperationException(const std::string &msg) : GEOSException("UnsupportedOperationException", msg) {
	}

	~UnsupportedOperationException() noexcept override {
	}
};

} // namespace util
} // namespace geos
