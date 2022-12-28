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
 * Copyright (C) 2010 - Oslandia
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

/* construct a new LWTRIANGLE.
 * use SRID=SRID_UNKNOWN for unknown SRID (will have 8bit type's S = 0)
 */
LWTRIANGLE *lwtriangle_construct(int32_t srid, GBOX *bbox, POINTARRAY *points) {
	LWTRIANGLE *result;

	result = (LWTRIANGLE *)lwalloc(sizeof(LWTRIANGLE));
	result->type = TRIANGLETYPE;

	result->flags = points->flags;
	FLAGS_SET_BBOX(result->flags, bbox ? 1 : 0);

	result->srid = srid;
	result->points = points;
	result->bbox = bbox;

	return result;
}

LWTRIANGLE *lwtriangle_construct_empty(int32_t srid, char hasz, char hasm) {
	LWTRIANGLE *result = (LWTRIANGLE *)lwalloc(sizeof(LWTRIANGLE));
	result->type = TRIANGLETYPE;
	result->flags = lwflags(hasz, hasm, 0);
	result->srid = srid;
	result->points = ptarray_construct_empty(hasz, hasm, 1);
	result->bbox = NULL;
	return result;
}

void lwtriangle_free(LWTRIANGLE *triangle) {
	if (!triangle)
		return;

	if (triangle->bbox)
		lwfree(triangle->bbox);

	if (triangle->points)
		ptarray_free(triangle->points);

	lwfree(triangle);
}

/**
 * Find the area of the outer ring
 */
double lwtriangle_area(const LWTRIANGLE *triangle) {
	double area = 0.0;
	uint32_t i;
	POINT2D p1;
	POINT2D p2;

	if (!triangle->points->npoints)
		return area; /* empty triangle */

	for (i = 0; i < triangle->points->npoints - 1; i++) {
		getPoint2d_p(triangle->points, i, &p1);
		getPoint2d_p(triangle->points, i + 1, &p2);
		area += (p1.x * p2.y) - (p1.y * p2.x);
	}

	area /= 2.0;

	return fabs(area);
}

double lwtriangle_perimeter_2d(const LWTRIANGLE *triangle) {
	if (triangle->points)
		return ptarray_length_2d(triangle->points);
	else
		return 0.0;
}

} // namespace duckdb
