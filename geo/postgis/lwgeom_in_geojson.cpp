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
 * Copyright 2011 Kashif Rasul <kashif.rasul@gmail.com>
 *
 **********************************************************************/

#include "liblwgeom/gserialized.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "postgis/geography_inout.hpp"
#include "postgis/lwgeom_inout.hpp"

#include <cstring>
#include <string>

namespace duckdb {

GSERIALIZED *geom_from_geojson(char *geojson) {
	int32_t geog_typmod = -1;
	GSERIALIZED *geom;
	LWGEOM *lwgeom;
	char *srs = NULL;

	lwgeom = lwgeom_from_geojson(geojson, &srs);
	if (!lwgeom) {
		/* Shouldn't get here */
		// elog(ERROR, "lwgeom_from_geojson returned NULL");
		return nullptr;
	}

	geom = gserialized_geography_from_lwgeom(lwgeom, geog_typmod);
	lwgeom_free(lwgeom);

	return geom;
}

} // namespace duckdb
