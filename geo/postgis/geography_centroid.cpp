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
 * Copyright (C) 2017 Danny Götte <danny.goette@fem.tu-ilmenau.de>
 *
 **********************************************************************/

#include "postgis/geography_centroid.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_transform.hpp"

namespace duckdb {

GSERIALIZED *geography_centroid(GSERIALIZED *g, bool use_spheroid) {
	LWGEOM *lwgeom = NULL;
	// LWGEOM *lwgeom_out = NULL;
	// LWPOINT *lwpoint_out = NULL;
	// GSERIALIZED *g_out = NULL;
	// int32_t srid;
	SPHEROID s;

	/* Get our geometry object loaded into memory. */
	lwgeom = lwgeom_from_gserialized(g);

	if (g == NULL) {
		return nullptr;
	}

	// srid = lwgeom_get_srid(lwgeom);

	/* on empty input, return empty output */
	// if (gserialized_is_empty(g)) {
	// 	LWCOLLECTION *empty = lwcollection_construct_empty(COLLECTIONTYPE, srid, 0, 0);
	// 	lwgeom_out = lwcollection_as_lwgeom(empty);
	// 	g_out = geography_serialize(lwgeom_out);
	// 	PG_RETURN_POINTER(g_out);
	// }

	/* Initialize spheroid */
	spheroid_init_from_srid(gserialized_get_srid(g), &s);

	/* Set to sphere if requested */
	if (!use_spheroid)
		s.a = s.b = s.radius;

	switch (lwgeom_get_type(lwgeom)) {

	case POINTTYPE: {
		/* centroid of a point is itself */
		return g;
	}

	default:
		// elog(ERROR, "ST_Centroid(geography) unhandled geography type");
		return nullptr;
	}

	// lwgeom_out = lwpoint_as_lwgeom(lwpoint_out);
	// g_out = geography_serialize(lwgeom_out);

	// PG_RETURN_POINTER(g_out);
	return nullptr;
}

} // namespace duckdb
