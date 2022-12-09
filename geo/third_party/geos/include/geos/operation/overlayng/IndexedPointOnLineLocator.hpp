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

#include <geos/algorithm/locate/PointOnGeometryLocator.hpp>
#include <geos/export.hpp>
#include <geos/geom/Location.hpp>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class Coordinate;
} // namespace geom
} // namespace geos

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Locates points on a linear geometry,
 * using a spatial index to provide good performance.
 *
 * @author mdavis
 */
class IndexedPointOnLineLocator : public algorithm::locate::PointOnGeometryLocator {

private:
	// Members
	const geom::Geometry &inputGeom;

public:
	IndexedPointOnLineLocator(const geom::Geometry &geomLinear) : inputGeom(geomLinear) {
	}

	geom::Location locate(const geom::CoordinateXY *p) override;
};

} // namespace overlayng
} // namespace operation
} // namespace geos
