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
 * Copyright 2012 J Smith <dark.panda@gmail.com>
 *
 **********************************************************************/

#include "postgis/lwgeom_in_geohash.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "postgis/lwgeom_box.hpp"

#include <cstring>
#include <string>

namespace duckdb {

static GBOX *parse_geohash(char *geohash, int precision) {
	GBOX *box = NULL;
	double lat[2], lon[2];

	if (NULL == geohash) {
		// geohash_lwpgerror("invalid GeoHash representation", 2);
		return nullptr;
	}

	decode_geohash_bbox(geohash, lat, lon, precision);

	box = gbox_new(lwflags(0, 0, 1));

	box->xmin = lon[0];
	box->ymin = lat[0];

	box->xmax = lon[1];
	box->ymax = lat[1];

	return box;
}

GBOX *box2d_from_geohash(char *geohash, int precision) {
	GBOX *box = NULL;
	return parse_geohash(geohash, precision);
}

GSERIALIZED *LWGEOM_from_GeoHash(char *geohash, int precision) {
	auto box = parse_geohash(geohash, precision);
	auto gser = BOX2D_to_LWGEOM(box);
	lwfree(box);

	return gser;
}

} // namespace duckdb
