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
 * Copyright 2008 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "geos_c.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *GEOS2POSTGIS(GEOSGeom geom, char want3d);
GEOSGeometry *POSTGIS2GEOS(const GSERIALIZED *g);

extern void lwgeom_geos_error(const char *fmt, ...);

GSERIALIZED *centroid(GSERIALIZED *geom);
bool LWGEOM_isring(GSERIALIZED *geom);
GSERIALIZED *ST_Difference(GSERIALIZED *geom1, GSERIALIZED *geom2);
GSERIALIZED *ST_Union(GSERIALIZED *geom1, GSERIALIZED *geom2);
GSERIALIZED *pgis_union_geometry_array(GSERIALIZED *gserArray[], int nelems);
GSERIALIZED *ST_Intersection(GSERIALIZED *geom1, GSERIALIZED *geom2);
GSERIALIZED *convexhull(GSERIALIZED *geom);
GSERIALIZED *buffer(GSERIALIZED *geom1, double size, string styles_text = "");
bool ST_Equals(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool contains(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool touches(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool ST_Intersects(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool covers(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool coveredby(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool disjoint(GSERIALIZED *geom1, GSERIALIZED *geom2);

} // namespace duckdb
