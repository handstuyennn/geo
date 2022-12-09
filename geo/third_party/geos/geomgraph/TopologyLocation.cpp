/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/TopologyLocation.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#include <algorithm>
#include <cassert>
#include <geos/geom/Location.hpp>
#include <geos/geom/Position.hpp>
#include <geos/geomgraph/TopologyLocation.hpp>
#include <iostream>
#include <sstream>
#include <vector>

using geos::geom::Location;
using geos::geom::Position;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public*/
void TopologyLocation::merge(const TopologyLocation &gl) {
	// if the src is an Area label & and the dest is not, increase the dest to be an Area
	std::size_t sz = locationSize;
	std::size_t glsz = gl.locationSize;
	if (glsz > sz) {
		locationSize = 3;
		location[Position::LEFT] = Location::NONE;
		location[Position::RIGHT] = Location::NONE;
	}
	const std::size_t maxIndex = std::min(static_cast<std::size_t>(locationSize), glsz);
	for (std::size_t i = 0; i < maxIndex; ++i) {
		if (location[i] == Location::NONE) {
			location[i] = gl.location[i];
		}
	}
}

} // namespace geomgraph
} // namespace geos
