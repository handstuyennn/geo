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

GSERIALIZED *LWGEOM_snaptogrid(GSERIALIZED *in_geom, double ipx, double ipy, double xsize, double ysize) {
	LWGEOM *in_lwgeom;
	GSERIALIZED *out_geom = NULL;
	LWGEOM *out_lwgeom;
	gridspec grid;

	/* Set grid values to zero to start */
	memset(&grid, 0, sizeof(gridspec));

	grid.ipx = ipx;
	grid.ipy = ipy;
	grid.xsize = xsize;
	grid.ysize = ysize;

	/* Return input geometry if input geometry is empty */
	if (gserialized_is_empty(in_geom)) {
		return in_geom;
	}

	/* Return input geometry if input grid is meaningless */
	if (grid.xsize == 0 && grid.ysize == 0 && grid.zsize == 0 && grid.msize == 0) {
		return in_geom;
	}

	in_lwgeom = lwgeom_from_gserialized(in_geom);

	out_lwgeom = lwgeom_grid(in_lwgeom, &grid);
	if (out_lwgeom == NULL)
		return nullptr;

	/* COMPUTE_BBOX TAINTING */
	if (in_lwgeom->bbox)
		lwgeom_refresh_bbox(out_lwgeom);

	out_geom = geometry_serialize(out_lwgeom);

	lwgeom_free(in_lwgeom);
	lwgeom_free(out_lwgeom);

	return out_geom;
}

} // namespace duckdb
