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
 * Copyright 2016 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright 2016 Daniel Baston <dbaston@gmail.com>
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

std::vector<int> ST_ClusterDBSCAN(GSERIALIZED *gserArray[], int nelems, double tolerance, int minpoints);

} // namespace duckdb
