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
 * Copyright (C) 2001-2005 Refractions Research Inc.
 *
 **********************************************************************/

#include "postgis/lwgeom_functions_analytic.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

namespace duckdb {

/***********************************************************************
 * Simple Douglas-Peucker line simplification.
 * No checks are done to avoid introduction of self-intersections.
 * No topology relations are considered.
 *
 * --strk@kbt.io;
 ***********************************************************************/

GSERIALIZED *LWGEOM_simplify2d(GSERIALIZED *geom, double dist) {
	GSERIALIZED *result;
	int type = gserialized_get_type(geom);
	LWGEOM *in;
	bool preserve_collapsed = false;
	int modified = LW_FALSE;

	/* Can't simplify points! */
	if (type == POINTTYPE || type == MULTIPOINTTYPE)
		return geom;

	// /* Handle optional argument to preserve collapsed features */
	// if ((PG_NARGS() > 2) && (!PG_ARGISNULL(2)))
	// 	preserve_collapsed = PG_GETARG_BOOL(2);

	in = lwgeom_from_gserialized(geom);

	modified = lwgeom_simplify_in_place(in, dist, preserve_collapsed);
	if (!modified)
		return geom;

	if (!in || lwgeom_is_empty(in))
		return nullptr;

	result = geometry_serialize(in);

    lwgeom_free(in);

	return result;
}

} // namespace duckdb
