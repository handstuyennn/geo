/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: io/WKTWriter.java rev. 1.34 (JTS-1.7)
 *
 **********************************************************************/

#pragma once

#include <cctype>
#include <geos/export.hpp>
#include <string>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateXY;
class CoordinateSequence;
class Geometry;
class GeometryCollection;
class Point;
class LineString;
class LinearRing;
class Polygon;
class MultiPoint;
class MultiLineString;
class MultiPolygon;
class PrecisionModel;
} // namespace geom
namespace io {
class Writer;
}
} // namespace geos

namespace geos {
namespace io {

/**
 * \class WKTWriter
 *
 * \brief Outputs the textual representation of a Geometry.
 * See also WKTReader.
 *
 * The WKTWriter outputs coordinates rounded to the precision
 * model. No more than the maximum number of necessary decimal places will be
 * output.
 *
 * The Well-known Text format is defined in the <A
 * HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 * Specification for SQL</A>.
 *
 * A non-standard "LINEARRING" tag is used for LinearRings. The WKT spec does
 * not define a special tag for LinearRings. The standard tag to use is
 * "LINESTRING".
 *
 * See WKTReader for parsing.
 *
 */
class GEOS_DLL WKTWriter {
public:
	/**
	 * Generates the WKT for a N-point <code>LineString</code>.
	 *
	 * @param seq the sequence to outpout
	 *
	 * @return the WKT
	 */
	static std::string toLineString(const geom::CoordinateSequence &seq);

	/**
	 * Generates the WKT for a 2-point <code>LineString</code>.
	 *
	 * @param p0 the first coordinate
	 * @param p1 the second coordinate
	 *
	 * @return the WKT
	 */
	static std::string toLineString(const geom::Coordinate &p0, const geom::Coordinate &p1);
};

} // namespace io
} // namespace geos
