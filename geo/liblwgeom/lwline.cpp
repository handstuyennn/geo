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
 * Copyright (C) 2012 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2006 Refractions Research Inc.
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

namespace duckdb {

int lwline_is_closed(const LWLINE *line) {
	if (FLAGS_GET_Z(line->flags))
		return ptarray_is_closed_3d(line->points);

	return ptarray_is_closed_2d(line->points);
}

/*
 * Construct a new LWLINE.  points will *NOT* be copied
 * use SRID=SRID_UNKNOWN for unknown SRID (will have 8bit type's S = 0)
 */
LWLINE *lwline_construct(int32_t srid, GBOX *bbox, POINTARRAY *points) {
	LWLINE *result = (LWLINE *)lwalloc(sizeof(LWLINE));
	result->type = LINETYPE;
	result->flags = points->flags;
	FLAGS_SET_BBOX(result->flags, bbox ? 1 : 0);
	result->srid = srid;
	result->points = points;
	result->bbox = bbox;
	return result;
}

LWLINE *lwline_construct_empty(int32_t srid, char hasz, char hasm) {
	LWLINE *result = (LWLINE *)lwalloc(sizeof(LWLINE));
	result->type = LINETYPE;
	result->flags = lwflags(hasz, hasm, 0);
	result->srid = srid;
	result->points = ptarray_construct_empty(hasz, hasm, 1);
	result->bbox = NULL;
	return result;
}

void lwline_free(LWLINE *line) {
	if (!line)
		return;

	if (line->bbox)
		lwfree(line->bbox);
	if (line->points)
		ptarray_free(line->points);
	lwfree(line);
}

/* @brief Clone LWLINE object. Serialized point lists are not copied.
 *
 * @see ptarray_clone
 */
LWLINE *lwline_clone(const LWLINE *g) {
	LWLINE *ret = (LWLINE *)lwalloc(sizeof(LWLINE));

	memcpy(ret, g, sizeof(LWLINE));

	ret->points = (POINTARRAY *)ptarray_clone(g->points);

	if (g->bbox)
		ret->bbox = gbox_copy(g->bbox);
	return ret;
}

/* Deep clone LWLINE object. POINTARRAY *is* copied. */
LWLINE *lwline_clone_deep(const LWLINE *g) {
	LWLINE *ret = (LWLINE *)lwalloc(sizeof(LWLINE));

	memcpy(ret, g, sizeof(LWLINE));

	if (g->bbox)
		ret->bbox = gbox_copy(g->bbox);
	if (g->points)
		ret->points = ptarray_clone_deep(g->points);
	FLAGS_SET_READONLY(ret->flags, 0);

	return ret;
}

LWLINE *lwline_force_dims(const LWLINE *line, int hasz, int hasm, double zval, double mval) {
	POINTARRAY *pdims = NULL;
	LWLINE *lineout;

	/* Return 2D empty */
	if (lwline_is_empty(line)) {
		lineout = lwline_construct_empty(line->srid, hasz, hasm);
	} else {
		pdims = ptarray_force_dims(line->points, hasz, hasm, zval, mval);
		lineout = lwline_construct(line->srid, NULL, pdims);
	}
	lineout->type = line->type;
	return lineout;
}

uint32_t lwline_count_vertices(LWLINE *line) {
	assert(line);
	if (!line->points)
		return 0;
	return line->points->npoints;
}

/*
 * Construct a LWLINE from an array of point and line geometries
 * LWLINE dimensions are large enough to host all input dimensions.
 */
LWLINE *lwline_from_lwgeom_array(int32_t srid, uint32_t ngeoms, LWGEOM **geoms) {
	uint32_t i;
	int hasz = LW_FALSE;
	int hasm = LW_FALSE;
	POINTARRAY *pa;
	LWLINE *line;
	POINT4D pt;
	LWPOINTITERATOR *it;

	/*
	 * Find output dimensions, check integrity
	 */
	for (i = 0; i < ngeoms; i++) {
		if (FLAGS_GET_Z(geoms[i]->flags))
			hasz = LW_TRUE;
		if (FLAGS_GET_M(geoms[i]->flags))
			hasm = LW_TRUE;
		if (hasz && hasm)
			break; /* Nothing more to learn! */
	}

	/*
	 * ngeoms should be a guess about how many points we have in input.
	 * It's an underestimate for lines and multipoints */
	pa = ptarray_construct_empty(hasz, hasm, ngeoms);

	for (i = 0; i < ngeoms; i++) {
		LWGEOM *g = geoms[i];

		if (lwgeom_is_empty(g))
			continue;

		if (g->type == POINTTYPE) {
			lwpoint_getPoint4d_p((LWPOINT *)g, &pt);
			ptarray_append_point(pa, &pt, LW_TRUE);
		} else if (g->type == LINETYPE) {
			/*
			 * Append the new line points, de-duplicating against the previous points.
			 * Duplicated points internal to the linestring are untouched.
			 */
			ptarray_append_ptarray(pa, ((LWLINE *)g)->points, -1);
		} else if (g->type == MULTIPOINTTYPE) {
			it = lwpointiterator_create(g);
			while (lwpointiterator_next(it, &pt)) {
				ptarray_append_point(pa, &pt, LW_TRUE);
			}
			lwpointiterator_destroy(it);
		} else {
			ptarray_free(pa);
			lwerror("lwline_from_ptarray: invalid input type: %s", lwtype_name(g->type));
			return NULL;
		}
	}

	if (pa->npoints > 0)
		line = lwline_construct(srid, NULL, pa);
	else {
		/* Is this really any different from the above ? */
		ptarray_free(pa);
		line = lwline_construct_empty(srid, hasz, hasm);
	}

	return line;
}

/**
 * Returns freshly allocated #LWPOINT that corresponds to the index where.
 * Returns NULL if the geometry is empty or the index invalid.
 */
LWPOINT *lwline_get_lwpoint(const LWLINE *line, uint32_t where) {
	POINT4D pt;
	LWPOINT *lwpoint;
	POINTARRAY *pa;

	if (lwline_is_empty(line) || where >= line->points->npoints)
		return NULL;

	pa = ptarray_construct_empty(FLAGS_GET_Z(line->flags), FLAGS_GET_M(line->flags), 1);
	pt = getPoint4d(line->points, where);
	ptarray_append_point(pa, &pt, LW_TRUE);
	lwpoint = lwpoint_construct(line->srid, NULL, pa);
	return lwpoint;
}

double lwline_length_2d(const LWLINE *line) {
	if (lwline_is_empty(line))
		return 0.0;
	return ptarray_length_2d(line->points);
}

} // namespace duckdb
