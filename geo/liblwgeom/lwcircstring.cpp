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
 * Copyright (C) 2001-2006 Refractions Research Inc.
 *
 **********************************************************************/

/* basic LWCIRCSTRING functions */

#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

/*
 * Construct a new LWCIRCSTRING.  points will *NOT* be copied
 * use SRID=SRID_UNKNOWN for unknown SRID (will have 8bit type's S = 0)
 */
LWCIRCSTRING *lwcircstring_construct(int32_t srid, GBOX *bbox, POINTARRAY *points) {
	LWCIRCSTRING *result;

	/*
	 * The first arc requires three points.  Each additional
	 * arc requires two more points.  Thus the minimum point count
	 * is three, and the count must be odd.
	 */
	if (points->npoints % 2 != 1 || points->npoints < 3) {
		return NULL;
	}

	result = (LWCIRCSTRING *)lwalloc(sizeof(LWCIRCSTRING));

	result->type = CIRCSTRINGTYPE;

	result->flags = points->flags;
	FLAGS_SET_BBOX(result->flags, bbox ? 1 : 0);

	result->srid = srid;
	result->points = points;
	result->bbox = bbox;

	return result;
}

LWCIRCSTRING *lwcircstring_construct_empty(int32_t srid, char hasz, char hasm) {
	LWCIRCSTRING *result = (LWCIRCSTRING *)lwalloc(sizeof(LWCIRCSTRING));
	result->type = CIRCSTRINGTYPE;
	result->flags = lwflags(hasz, hasm, 0);
	result->srid = srid;
	result->points = ptarray_construct_empty(hasz, hasm, 1);
	result->bbox = NULL;
	return result;
}

void lwcircstring_free(LWCIRCSTRING *curve) {
	if (!curve)
		return;

	if (curve->bbox)
		lwfree(curve->bbox);
	if (curve->points)
		ptarray_free(curve->points);
	lwfree(curve);
}

int lwcircstring_is_closed(const LWCIRCSTRING *curve) {
	if (lwgeom_has_z((LWGEOM *)curve))
		return ptarray_is_closed_3d(curve->points);

	return ptarray_is_closed_2d(curve->points);
}

double lwcircstring_length_2d(const LWCIRCSTRING *circ) {
	if (lwcircstring_is_empty(circ))
		return 0.0;

	return ptarray_arc_length_2d(circ->points);
}

} // namespace duckdb
