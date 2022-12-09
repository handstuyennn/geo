/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/EdgeRing.java 0b3c7e3eb0d3e
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/locate/IndexedPointInAreaLocator.hpp>
#include <geos/export.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/operation/polygonize/PolygonizeDirectedEdge.hpp>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class LineString;
class CoordinateSequence;
class GeometryFactory;
class Coordinate;
} // namespace geom
namespace planargraph {
class DirectedEdge;
}
} // namespace geos

namespace geos {
namespace operation {  // geos::operation
namespace polygonize { // geos::operation::polygonize

/** \brief
 * Represents a ring of PolygonizeDirectedEdge which form
 * a ring of a polygon.  The ring may be either an outer shell or a hole.
 */
class GEOS_DLL EdgeRing {
public:
	/**
	 * \brief
	 * Finds a point in a list of points which is not contained in
	 * another list of points.
	 *
	 * @param testPts the CoordinateSequence to test
	 * @param pts the CoordinateSequence to test the input points against
	 * @return a Coordinate reference from <code>testPts</code> which is
	 * not in <code>pts</code>, or <code>Coordinate::nullCoord</code>
	 */
	static const geom::Coordinate &ptNotInList(const geom::CoordinateSequence *testPts,
	                                           const geom::CoordinateSequence *pts);

	/** \brief
	 * Tests whether a given point is in an array of points.
	 * Uses a value-based test.
	 *
	 * @param pt a Coordinate for the test point
	 * @param pts an array of Coordinate to test
	 * @return <code>true</code> if the point is in the array
	 */
	static bool isInList(const geom::Coordinate &pt, const geom::CoordinateSequence *pts);
};

} // namespace polygonize
} // namespace operation
} // namespace geos