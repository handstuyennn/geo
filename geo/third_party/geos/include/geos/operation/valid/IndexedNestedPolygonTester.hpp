/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/locate/IndexedPointInAreaLocator.hpp>
#include <geos/export.hpp>
#include <geos/index/strtree/TemplateSTRtree.hpp>
#include <map>
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Polygon;
class LinearRing;
class MultiPolygon;
} // namespace geom
} // namespace geos

namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using algorithm::locate::IndexedPointInAreaLocator;
using geos::geom::CoordinateXY;
using geos::geom::LinearRing;
using geos::geom::MultiPolygon;
using geos::geom::Polygon;
using index::strtree::TemplateSTRtree;

class GEOS_DLL IndexedNestedPolygonTester {
private:
	const MultiPolygon *multiPoly;
	TemplateSTRtree<const Polygon *> index;
	std::map<const Polygon *, IndexedPointInAreaLocator> locators;
	CoordinateXY nestedPt;

	void loadIndex();

	bool findNestedPoint(const LinearRing *shell, const Polygon *possibleOuterPoly, IndexedPointInAreaLocator &locator,
	                     CoordinateXY &coordNested);

	IndexedPointInAreaLocator &getLocator(const Polygon *poly);

	/**
	 * Finds a point of a shell segment which lies inside a polygon, if any.
	 * The shell is assume to touch the polyon only at shell vertices,
	 * and does not cross the polygon.
	 *
	 * @param the shell to test
	 * @param the polygon to test against
	 * @param coordNested return parametr for found coordinate
	 * @return an interior segment point, or null if the shell is nested correctly
	 */
	static bool findIncidentSegmentNestedPoint(const LinearRing *shell, const Polygon *poly, CoordinateXY &coordNested);

public:
	IndexedNestedPolygonTester(const MultiPolygon *p_multiPoly);

	/**
	 * Gets a point on a nested polygon, if one exists.
	 *
	 * @return a point on a nested polygon, or null if none are nested
	 */
	const CoordinateXY &getNestedPoint() const {
		return nestedPt;
	}

	/**
	 * Tests if any polygon is nested (contained) within another polygon.
	 * This is invalid.
	 * The nested point will be set to reflect this.
	 * @return true if some polygon is nested
	 */
	bool isNested();
};

} // namespace valid
} // namespace operation
} // namespace geos
