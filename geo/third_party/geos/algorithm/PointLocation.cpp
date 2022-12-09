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

#include <cmath>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/algorithm/PointLocation.hpp>
#include <geos/algorithm/RayCrossingCounter.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Location.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <vector>

namespace geos {
namespace algorithm { // geos.algorithm

/* public static */
geom::Location PointLocation::locateInRing(const geom::CoordinateXY &p,
                                           const std::vector<const geom::Coordinate *> &ring) {
	return RayCrossingCounter::locatePointInRing(p, ring);
}

/* public static */
geom::Location PointLocation::locateInRing(const geom::CoordinateXY &p, const geom::CoordinateSequence &ring) {
	return RayCrossingCounter::locatePointInRing(p, ring);
}

} // namespace algorithm
} // namespace geos
