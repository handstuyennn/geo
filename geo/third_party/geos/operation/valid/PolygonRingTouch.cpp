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

#include <geos/geom/Coordinate.hpp>
#include <geos/operation/valid/PolygonRing.hpp>
#include <geos/operation/valid/PolygonRingTouch.hpp>

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using namespace geos::geom;

/* public */
const CoordinateXY *PolygonRingTouch::getCoordinate() const {
	return &touchPt;
}

/* public */
PolygonRing *PolygonRingTouch::getRing() const {
	return ring;
}

/* public */
bool PolygonRingTouch::isAtLocation(const CoordinateXY &pt) const {
	return touchPt.equals2D(pt);
}

} // namespace valid
} // namespace operation
} // namespace geos
