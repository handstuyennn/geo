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
 * Copyright 2001-2005 Refractions Research Inc.
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *LWGEOM_from_text(char *text, int srid = SRID_UNKNOWN);
GSERIALIZED *LWGEOM_from_WKB(const char *bytea_wkb, size_t byte_size, int srid = SRID_UNKNOWN);
GSERIALIZED *LWGEOM_boundary(GSERIALIZED *geom);
int LWGEOM_dimension(GSERIALIZED *geom);
GSERIALIZED *LWGEOM_endpoint_linestring(GSERIALIZED *geom);
std::string geometry_geometrytype(GSERIALIZED *geom);
bool LWGEOM_isclosed(GSERIALIZED *geom);
int LWGEOM_numgeometries_collection(GSERIALIZED *geom);
int LWGEOM_numpoints_linestring(GSERIALIZED *geom);
GSERIALIZED *LWGEOM_pointn_linestring(GSERIALIZED *geom, int where);
GSERIALIZED *LWGEOM_startpoint_linestring(GSERIALIZED *geom);
double LWGEOM_x_point(GSERIALIZED *geom);
double LWGEOM_y_point(GSERIALIZED *geom);

} // namespace duckdb
