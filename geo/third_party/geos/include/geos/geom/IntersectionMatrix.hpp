/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/IntersectionMatrix.java rev. 1.18
 *
 **********************************************************************/

#pragma once

#include <array>
#include <geos/export.hpp>
#include <geos/geom/Location.hpp>
#include <string>

namespace geos {
namespace geom { // geos::geom

/** \brief
 * Implementation of Dimensionally Extended Nine-Intersection Model
 * (DE-9IM) matrix.
 *
 * Dimensionally Extended Nine-Intersection Model (DE-9IM) matrix.
 * This class can used to represent both computed DE-9IM's (like 212FF1FF2)
 * as well as patterns for matching them (like T*T******).
 *
 * Methods are provided to:
 *
 *  - set and query the elements of the matrix in a convenient fashion
 *  - convert to and from the standard string representation
 *    (specified in SFS Section 2.1.13.2).
 *  - test to see if a matrix matches a given pattern string.
 *
 * For a description of the DE-9IM, see the
 * <a href="http://www.opengis.org/techno/specs.htm">OpenGIS Simple
 * Features Specification for SQL.</a>
 *
 * \todo Suggestion: add equal and not-equal operator to this class.
 */
class GEOS_DLL IntersectionMatrix {}; // class IntersectionMatrix

} // namespace geom
} // namespace geos