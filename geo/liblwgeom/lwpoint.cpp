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

#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

namespace duckdb {

LWPOINT *lwpoint_construct_empty(int32_t srid, char hasz, char hasm) {
	LWPOINT *result = (LWPOINT *)lwalloc(sizeof(LWPOINT));
	result->type = POINTTYPE;
	result->flags = lwflags(hasz, hasm, 0);
	result->srid = srid;
	result->point = ptarray_construct(hasz, hasm, 0);
	result->bbox = NULL;
	return result;
}

LWPOINT *lwpoint_make2d(int32_t srid, double x, double y) {
	POINT4D p = {x, y, 0.0, 0.0};
	POINTARRAY *pa = ptarray_construct_empty(0, 0, 1);

	ptarray_append_point(pa, &p, LW_TRUE);
	return lwpoint_construct(srid, NULL, pa);
}

LWPOINT *lwpoint_make3dz(int32_t srid, double x, double y, double z) {
	POINT4D p = {x, y, z, 0.0};
	POINTARRAY *pa = ptarray_construct_empty(1, 0, 1);

	ptarray_append_point(pa, &p, LW_TRUE);

	return lwpoint_construct(srid, NULL, pa);
}

/*
 * Construct a new point.  point will not be copied
 * use SRID=SRID_UNKNOWN for unknown SRID (will have 8bit type's S = 0)
 */
LWPOINT *lwpoint_construct(int32_t srid, GBOX *bbox, POINTARRAY *point) {
	LWPOINT *result;
	lwflags_t flags = 0;

	if (point == NULL)
		return NULL; /* error */

	result = (LWPOINT *)lwalloc(sizeof(LWPOINT));
	result->type = POINTTYPE;
	FLAGS_SET_Z(flags, FLAGS_GET_Z(point->flags));
	FLAGS_SET_M(flags, FLAGS_GET_M(point->flags));
	FLAGS_SET_BBOX(flags, bbox ? 1 : 0);
	result->flags = flags;
	result->srid = srid;
	result->point = point;
	result->bbox = bbox;

	return result;
}

void lwpoint_free(LWPOINT *pt) {
	if (!pt)
		return;

	if (pt->bbox)
		lwfree(pt->bbox);
	if (pt->point)
		ptarray_free(pt->point);
	lwfree(pt);
}

LWPOINT *lwpoint_force_dims(const LWPOINT *point, int hasz, int hasm, double zval, double mval) {
	POINTARRAY *pdims = NULL;
	LWPOINT *pointout;

	/* Return 2D empty */
	if (lwpoint_is_empty(point)) {
		pointout = lwpoint_construct_empty(point->srid, hasz, hasm);
	} else {
		/* Always we duplicate the ptarray and return */
		pdims = ptarray_force_dims(point->point, hasz, hasm, zval, mval);
		pointout = lwpoint_construct(point->srid, NULL, pdims);
	}
	pointout->type = point->type;
	return pointout;
}

int lwpoint_getPoint4d_p(const LWPOINT *point, POINT4D *out) {
	return lwpoint_is_empty(point) ? 0 : getPoint4d_p(point->point, 0, out);
}

double lwpoint_get_x(const LWPOINT *point) {
	POINT4D pt;
	if (lwpoint_is_empty(point)) {
		lwerror("lwpoint_get_x called with empty geometry");
		return 0;
	}
	getPoint4d_p(point->point, 0, &pt);
	return pt.x;
}

double lwpoint_get_y(const LWPOINT *point) {
	POINT4D pt;
	if (lwpoint_is_empty(point)) {
		lwerror("lwpoint_get_y called with empty geometry");
		return 0;
	}
	getPoint4d_p(point->point, 0, &pt);
	return pt.y;
}

} // namespace duckdb
