/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/valid/RepeatedPointTester.java rev. 1.8 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp> // for composition

// Forward declarations
namespace geos {
namespace geom {
// class Coordinate;
class CoordinateSequence;
class Geometry;
class Polygon;
class MultiPolygon;
class MultiLineString;
class GeometryCollection;
} // namespace geom
} // namespace geos

namespace geos {
namespace operation { // geos::operation
namespace valid {     // geos::operation::valid

/** \brief
 * Implements the appropriate checks for repeated points
 * (consecutive identical coordinates) as defined in the
 * JTS spec.
 */
class GEOS_DLL RepeatedPointTester {
public:
	RepeatedPointTester() {
	}
	geom::CoordinateXY &getCoordinate();
	bool hasRepeatedPoint(const geom::Geometry *g);
	bool hasRepeatedPoint(const geom::CoordinateSequence *coord);

private:
	geom::CoordinateXY repeatedCoord;
	bool hasRepeatedPoint(const geom::Polygon *p);
	bool hasRepeatedPoint(const geom::GeometryCollection *gc);
	bool hasRepeatedPoint(const geom::MultiPolygon *gc);
	bool hasRepeatedPoint(const geom::MultiLineString *gc);
};

} // namespace valid
} // namespace operation
} // namespace geos
