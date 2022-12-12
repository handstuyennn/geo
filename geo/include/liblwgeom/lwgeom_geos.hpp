/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * PostGIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * PostGIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostGIS.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 *
 * Copyright 2011 Sandro Santilli <strk@kbt.io>
 * Copyright 2018 Darafei Praliaskouski <me@komzpa.net>
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "geos_c.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

/*
 ** Public prototypes for GEOS utility functions.
 */
LWGEOM *GEOS2LWGEOM(const GEOSGeometry *geom, uint8_t want3d);
GEOSGeometry *LWGEOM2GEOS(const LWGEOM *g, uint8_t autofix);

POINTARRAY *ptarray_from_GEOSCoordSeq(const GEOSCoordSequence *cs, uint8_t want3d);

} // namespace duckdb
