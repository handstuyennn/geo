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
 * Copyright 2001-2006 Refractions Research Inc.
 * Copyright 2017-2018 Daniel Baston <dbaston@gmail.com>
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *LWGEOM_makepoint(double x, double y);
GSERIALIZED *LWGEOM_makepoint(double x, double y, double z);
GSERIALIZED *LWGEOM_makeline(GSERIALIZED *geom1, GSERIALIZED *geom2);
GSERIALIZED *LWGEOM_makeline_garray(GSERIALIZED *gserArray[], int nelems);
GSERIALIZED *LWGEOM_makepoly(GSERIALIZED *geom, GSERIALIZED *gserArray[] = {}, int nelems = 0);
double ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2);
lwvarlena_t *ST_GeoHash(GSERIALIZED *gser, size_t m_chars = 0);
bool ST_IsCollection(GSERIALIZED *geom);
bool LWGEOM_isempty(GSERIALIZED *geom);
int LWGEOM_npoints(GSERIALIZED *geom);
GSERIALIZED *LWGEOM_closestpoint(GSERIALIZED *geom1, GSERIALIZED *geom2);
bool LWGEOM_dwithin(GSERIALIZED *geom1, GSERIALIZED *geom2, double tolerance);
double ST_Area(GSERIALIZED *geom);
double LWGEOM_angle(GSERIALIZED *geom1, GSERIALIZED *geom2);
double LWGEOM_angle(std::vector<GSERIALIZED *> geom_vec);
double LWGEOM_perimeter2d_poly(GSERIALIZED *geom);
double LWGEOM_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2);
double LWGEOM_length2d_linestring(GSERIALIZED *geom);
GSERIALIZED *LWGEOM_envelope(GSERIALIZED *geom);
double LWGEOM_maxdistance2d_linestring(GSERIALIZED *geom1, GSERIALIZED *geom2);
GSERIALIZED *LWGEOM_envelope_garray(GSERIALIZED *gserArray[], int nelems);

} // namespace duckdb
