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

#include <geos/algorithm/PointLocator.hpp>
#include <geos/algorithm/locate/PointOnGeometryLocator.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/operation/overlayng/IndexedPointOnLineLocator.hpp>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*public*/
geom::Location IndexedPointOnLineLocator::locate(const geom::CoordinateXY *p) {
	// TODO: optimize this with a segment index
	algorithm::PointLocator locator;
	return locator.locate(*p, &inputGeom);
}

} // namespace overlayng
} // namespace operation
} // namespace geos
