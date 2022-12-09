/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/PointLocation.java @ 2017-09-04
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Location.hpp>

namespace geos {
namespace algorithm { // geos::algorithm

/** \brief
 * Functions for locating points within basic geometric
 * structures such as lines and rings.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL PointLocation {
public:
	/** \brief
	 * Determines whether a point lies in the interior, on the boundary, or in the
	 * exterior of a ring. The ring may be oriented in either direction.
	 *
	 * This method does *not* first check the point against the envelope of
	 * the ring.
	 *
	 * @param p point to check for ring inclusion
	 * @param ring an array of coordinates representing the ring (which must have
	 *             first point identical to last point)
	 * @return the [Location](@ref geom::Location) of p relative to the ring
	 */
	static geom::Location locateInRing(const geom::CoordinateXY &p, const std::vector<const geom::Coordinate *> &ring);
	static geom::Location locateInRing(const geom::CoordinateXY &p, const geom::CoordinateSequence &ring);
};

} // namespace algorithm
} // namespace geos
