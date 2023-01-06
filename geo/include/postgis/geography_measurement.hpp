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
 * Copyright (C) 2009 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

#ifndef _LIBGEOGRAPHY_MEASUREMENT_H
#define _LIBGEOGRAPHY_MEASUREMENT_H 1

double geography_distance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid);
double geography_maxdistance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid);
double geography_area(GSERIALIZED *g, bool use_spheroid);
double geography_perimeter(GSERIALIZED *g, bool use_spheroid);
double geography_azimuth(GSERIALIZED *g1, GSERIALIZED *g2);
double geography_length(GSERIALIZED *g, bool use_spheroid);

#endif /* !defined _LIBGEOGRAPHY_MEASUREMENT_H  */

} // namespace duckdb
