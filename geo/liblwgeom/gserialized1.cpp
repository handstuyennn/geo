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
 * Copyright 2009 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright 2017 Darafei Praliaskouski <me@komzpa.net>
 *
 **********************************************************************/

#include "liblwgeom/gserialized1.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

#include <cassert>
#include <cstring>

namespace duckdb {

static inline void gserialized1_copy_point(double *dptr, lwflags_t flags, POINT4D *out_point) {
	uint8_t dim = 0;
	out_point->x = dptr[dim++];
	out_point->y = dptr[dim++];

	if (G1FLAGS_GET_Z(flags)) {
		out_point->z = dptr[dim++];
	}
	if (G1FLAGS_GET_M(flags)) {
		out_point->m = dptr[dim];
	}
}

lwflags_t gserialized1_get_lwflags(const GSERIALIZED *g) {
	lwflags_t lwflags = 0;
	uint8_t gflags = g->gflags;
	FLAGS_SET_Z(lwflags, G1FLAGS_GET_Z(gflags));
	FLAGS_SET_M(lwflags, G1FLAGS_GET_M(gflags));
	FLAGS_SET_BBOX(lwflags, G1FLAGS_GET_BBOX(gflags));
	FLAGS_SET_GEODETIC(lwflags, G1FLAGS_GET_GEODETIC(gflags));
	FLAGS_SET_SOLID(lwflags, G1FLAGS_GET_SOLID(gflags));
	return lwflags;
}

/* handle missaligned uint32_t data */
static inline uint32_t gserialized1_get_uint32_t(const uint8_t *loc) {
	return *((uint32_t *)loc);
}

int gserialized1_has_bbox(const GSERIALIZED *gser) {
	return G1FLAGS_GET_BBOX(gser->gflags);
}

int gserialized1_has_z(const GSERIALIZED *gser) {
	return G1FLAGS_GET_Z(gser->gflags);
}

static size_t gserialized1_box_size(const GSERIALIZED *g) {
	if (G1FLAGS_GET_GEODETIC(g->gflags))
		return 6 * sizeof(float);
	else
		return 2 * G1FLAGS_NDIMS(g->gflags) * sizeof(float);
}

int gserialized1_read_gbox_p(const GSERIALIZED *g, GBOX *gbox) {

	/* Null input! */
	if (!(g && gbox))
		return LW_FAILURE;

	/* Initialize the flags on the box */
	gbox->flags = gserialized1_get_lwflags(g);

	/* Has pre-calculated box */
	if (G1FLAGS_GET_BBOX(g->gflags)) {
		int i = 0;
		float *fbox = (float *)(g->data);
		gbox->xmin = fbox[i++];
		gbox->xmax = fbox[i++];
		gbox->ymin = fbox[i++];
		gbox->ymax = fbox[i++];

		/* Geodetic? Read next dimension (geocentric Z) and return */
		if (G1FLAGS_GET_GEODETIC(g->gflags)) {
			gbox->zmin = fbox[i++];
			gbox->zmax = fbox[i++];
			return LW_SUCCESS;
		}
		/* Cartesian? Read extra dimensions (if there) and return */
		if (G1FLAGS_GET_Z(g->gflags)) {
			gbox->zmin = fbox[i++];
			gbox->zmax = fbox[i++];
		}
		if (G1FLAGS_GET_M(g->gflags)) {
			gbox->mmin = fbox[i++];
			gbox->mmax = fbox[i++];
		}
		return LW_SUCCESS;
	}
	return LW_FAILURE;
}

int32_t gserialized1_get_srid(const GSERIALIZED *s) {
	int32_t srid = 0;
	srid = srid | (s->srid[0] << 16);
	srid = srid | (s->srid[1] << 8);
	srid = srid | s->srid[2];
	/* Only the first 21 bits are set. Slide up and back to pull
	   the negative bits down, if we need them. */
	srid = (srid << 11) >> 11;

	/* 0 is our internal unknown value. We'll map back and forth here for now */
	if (srid == 0)
		return SRID_UNKNOWN;
	else
		return srid;
}

void gserialized1_set_srid(GSERIALIZED *s, int32_t srid) {
	srid = clamp_srid(srid);

	/* 0 is our internal unknown value.
	 * We'll map back and forth here for now */
	if (srid == SRID_UNKNOWN)
		srid = 0;

	s->srid[0] = (srid & 0x001F0000) >> 16;
	s->srid[1] = (srid & 0x0000FF00) >> 8;
	s->srid[2] = (srid & 0x000000FF);
}

static size_t gserialized1_is_empty_recurse(const uint8_t *p, int *isempty) {
	int i;
	int32_t type, num;

	memcpy(&type, p, 4);
	memcpy(&num, p + 4, 4);

	if (lwtype_is_collection(type)) {
		size_t lz = 8;
		for (i = 0; i < num; i++) {
			lz += gserialized1_is_empty_recurse(p + lz, isempty);
			if (!*isempty)
				return lz;
		}
		*isempty = LW_TRUE;
		return lz;
	} else {
		*isempty = (num == 0 ? LW_TRUE : LW_FALSE);
		return 8;
	}
}

int gserialized1_is_empty(const GSERIALIZED *g) {
	uint8_t *p = (uint8_t *)g;
	int isempty = 0;
	assert(g);

	p += 8; /* Skip varhdr and srid/flags */
	if (gserialized1_has_bbox(g))
		p += gserialized1_box_size(g); /* Skip the box */

	gserialized1_is_empty_recurse(p, &isempty);
	return isempty;
}

uint32_t gserialized1_get_type(const GSERIALIZED *g) {
	uint32_t *ptr;
	ptr = (uint32_t *)(g->data);
	if (G1FLAGS_GET_BBOX(g->gflags)) {
		ptr += (gserialized1_box_size(g) / sizeof(uint32_t));
	}
	return *ptr;
}

/***********************************************************************
 * De-serialize GSERIALIZED into an LWGEOM.
 */

static LWGEOM *lwgeom_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size);

static LWPOINT *lwpoint_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size) {
	uint8_t *start_ptr = data_ptr;
	LWPOINT *point;
	uint32_t npoints = 0;

	assert(data_ptr);

	point = (LWPOINT *)lwalloc(sizeof(LWPOINT));
	point->srid = SRID_UNKNOWN; /* Default */
	point->bbox = NULL;
	point->type = POINTTYPE;
	point->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the type. */
	npoints = gserialized1_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4;                                 /* Skip past the npoints. */

	if (npoints > 0)
		point->point = ptarray_construct_reference_data(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), 1, data_ptr);
	else
		point->point = ptarray_construct(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), 0); /* Empty point */

	data_ptr += npoints * FLAGS_NDIMS(lwflags) * sizeof(double);

	if (size)
		*size = data_ptr - start_ptr;

	return point;
}

static LWLINE *lwline_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size) {
	uint8_t *start_ptr = data_ptr;
	LWLINE *line;
	uint32_t npoints = 0;

	assert(data_ptr);

	line = (LWLINE *)lwalloc(sizeof(LWLINE));
	line->srid = SRID_UNKNOWN; /* Default */
	line->bbox = NULL;
	line->type = LINETYPE;
	line->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the type. */
	npoints = gserialized1_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4;                                 /* Skip past the npoints. */

	if (npoints > 0)
		line->points = ptarray_construct_reference_data(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), npoints, data_ptr);

	else
		line->points = ptarray_construct(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), 0); /* Empty linestring */

	data_ptr += FLAGS_NDIMS(lwflags) * npoints * sizeof(double);

	if (size)
		*size = data_ptr - start_ptr;

	return line;
}

static LWPOLY *lwpoly_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size) {
	uint8_t *start_ptr = data_ptr;
	LWPOLY *poly;
	uint8_t *ordinate_ptr;
	uint32_t nrings = 0;
	uint32_t i = 0;

	assert(data_ptr);

	poly = (LWPOLY *)lwalloc(sizeof(LWPOLY));
	poly->srid = SRID_UNKNOWN; /* Default */
	poly->bbox = NULL;
	poly->type = POLYGONTYPE;
	poly->flags = lwflags;

	data_ptr += 4;                                /* Skip past the polygontype. */
	nrings = gserialized1_get_uint32_t(data_ptr); /* Zero => empty geometry */
	poly->nrings = nrings;
	data_ptr += 4; /* Skip past the nrings. */

	ordinate_ptr = data_ptr; /* Start the ordinate pointer. */
	if (nrings > 0) {
		poly->rings = (POINTARRAY **)lwalloc(sizeof(POINTARRAY *) * nrings);
		poly->maxrings = nrings;
		ordinate_ptr += nrings * 4; /* Move past all the npoints values. */
		if (nrings % 2)             /* If there is padding, move past that too. */
			ordinate_ptr += 4;
	} else /* Empty polygon */
	{
		poly->rings = NULL;
		poly->maxrings = 0;
	}

	for (i = 0; i < nrings; i++) {
		uint32_t npoints = 0;

		/* Read in the number of points. */
		npoints = gserialized1_get_uint32_t(data_ptr);
		data_ptr += 4;

		/* Make a point array for the ring, and move the ordinate pointer past the ring ordinates. */
		poly->rings[i] =
		    ptarray_construct_reference_data(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), npoints, ordinate_ptr);

		ordinate_ptr += sizeof(double) * FLAGS_NDIMS(lwflags) * npoints;
	}

	if (size)
		*size = ordinate_ptr - start_ptr;

	return poly;
}

static LWTRIANGLE *lwtriangle_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size) {
	uint8_t *start_ptr = data_ptr;
	LWTRIANGLE *triangle;
	uint32_t npoints = 0;

	assert(data_ptr);

	triangle = (LWTRIANGLE *)lwalloc(sizeof(LWTRIANGLE));
	triangle->srid = SRID_UNKNOWN; /* Default */
	triangle->bbox = NULL;
	triangle->type = TRIANGLETYPE;
	triangle->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the type. */
	npoints = gserialized1_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4;                                 /* Skip past the npoints. */

	if (npoints > 0)
		triangle->points =
		    ptarray_construct_reference_data(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), npoints, data_ptr);
	else
		triangle->points = ptarray_construct(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), 0); /* Empty triangle */

	data_ptr += FLAGS_NDIMS(lwflags) * npoints * sizeof(double);

	if (size)
		*size = data_ptr - start_ptr;

	return triangle;
}

static LWCIRCSTRING *lwcircstring_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size) {
	uint8_t *start_ptr = data_ptr;
	LWCIRCSTRING *circstring;
	uint32_t npoints = 0;

	assert(data_ptr);

	circstring = (LWCIRCSTRING *)lwalloc(sizeof(LWCIRCSTRING));
	circstring->srid = SRID_UNKNOWN; /* Default */
	circstring->bbox = NULL;
	circstring->type = CIRCSTRINGTYPE;
	circstring->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the circstringtype. */
	npoints = gserialized1_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4;                                 /* Skip past the npoints. */

	if (npoints > 0)
		circstring->points =
		    ptarray_construct_reference_data(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), npoints, data_ptr);
	else
		circstring->points =
		    ptarray_construct(FLAGS_GET_Z(lwflags), FLAGS_GET_M(lwflags), 0); /* Empty circularstring */

	data_ptr += FLAGS_NDIMS(lwflags) * npoints * sizeof(double);

	if (size)
		*size = data_ptr - start_ptr;

	return circstring;
}

static LWCOLLECTION *lwcollection_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size) {
	uint32_t type;
	uint8_t *start_ptr = data_ptr;
	LWCOLLECTION *collection;
	uint32_t ngeoms = 0;
	uint32_t i = 0;

	assert(data_ptr);

	type = gserialized1_get_uint32_t(data_ptr);
	data_ptr += 4; /* Skip past the type. */

	collection = (LWCOLLECTION *)lwalloc(sizeof(LWCOLLECTION));
	collection->srid = SRID_UNKNOWN; /* Default */
	collection->bbox = NULL;
	collection->type = type;
	collection->flags = lwflags;

	ngeoms = gserialized1_get_uint32_t(data_ptr);
	collection->ngeoms = ngeoms; /* Zero => empty geometry */
	data_ptr += 4;               /* Skip past the ngeoms. */

	if (ngeoms > 0) {
		collection->geoms = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * ngeoms);
		collection->maxgeoms = ngeoms;
	} else {
		collection->geoms = NULL;
		collection->maxgeoms = 0;
	}

	/* Sub-geometries are never de-serialized with boxes (#1254) */
	FLAGS_SET_BBOX(lwflags, 0);

	for (i = 0; i < ngeoms; i++) {
		uint32_t subtype = gserialized1_get_uint32_t(data_ptr);
		size_t subsize = 0;

		if (!lwcollection_allows_subtype(type, subtype)) {
			lwerror("Invalid subtype (%s) for collection type (%s)", lwtype_name(subtype), lwtype_name(type));
			lwfree(collection);
			return NULL;
		}
		collection->geoms[i] = lwgeom_from_gserialized1_buffer(data_ptr, lwflags, &subsize);
		data_ptr += subsize;
	}

	if (size)
		*size = data_ptr - start_ptr;

	return collection;
}

LWGEOM *lwgeom_from_gserialized1_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *g_size) {
	uint32_t type;

	assert(data_ptr);

	type = gserialized1_get_uint32_t(data_ptr);

	switch (type) {
	case POINTTYPE:
		return (LWGEOM *)lwpoint_from_gserialized1_buffer(data_ptr, lwflags, g_size);
	case LINETYPE:
		return (LWGEOM *)lwline_from_gserialized1_buffer(data_ptr, lwflags, g_size);
	case POLYGONTYPE:
		return (LWGEOM *)lwpoly_from_gserialized1_buffer(data_ptr, lwflags, g_size);
	case CIRCSTRINGTYPE:
		return (LWGEOM *)lwcircstring_from_gserialized1_buffer(data_ptr, lwflags, g_size);
	case TRIANGLETYPE:
		return (LWGEOM *)lwtriangle_from_gserialized1_buffer(data_ptr, lwflags, g_size);
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case COMPOUNDTYPE:
	case CURVEPOLYTYPE:
	case MULTICURVETYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return (LWGEOM *)lwcollection_from_gserialized1_buffer(data_ptr, lwflags, g_size);
		// Need to do with postgis

	default: {
		lwerror("Unknown geometry type: %d - %s", type, lwtype_name(type));
		return NULL;
	}
	}
}

LWGEOM *lwgeom_from_gserialized1(const GSERIALIZED *g) {
	lwflags_t lwflags = 0;
	int32_t srid = 0;
	uint32_t lwtype = 0;
	uint8_t *data_ptr = NULL;
	LWGEOM *lwgeom = NULL;
	GBOX bbox;
	size_t size = 0;

	assert(g);

	srid = gserialized1_get_srid(g);
	lwtype = gserialized1_get_type(g);
	lwflags = gserialized1_get_lwflags(g);

	data_ptr = (uint8_t *)g->data;
	if (FLAGS_GET_BBOX(lwflags))
		data_ptr += gbox_serialized_size(lwflags);

	lwgeom = lwgeom_from_gserialized1_buffer(data_ptr, lwflags, &size);

	if (!lwgeom) {
		lwerror("%s: unable create geometry", __func__); /* Ooops! */
		return NULL;
	}

	lwgeom->type = lwtype;
	lwgeom->flags = lwflags;

	if (gserialized1_read_gbox_p(g, &bbox) == LW_SUCCESS) {
		lwgeom->bbox = gbox_copy(&bbox);
	} else if (lwgeom_needs_bbox(lwgeom) && (lwgeom_calculate_gbox(lwgeom, &bbox) == LW_SUCCESS)) {
		lwgeom->bbox = gbox_copy(&bbox);
	} else {
		lwgeom->bbox = NULL;
	}

	lwgeom_set_srid(lwgeom, srid);

	return lwgeom;
}

int gserialized1_peek_first_point(const GSERIALIZED *g, POINT4D *out_point) {
	uint8_t *geometry_start = ((uint8_t *)g->data);
	if (gserialized1_has_bbox(g)) {
		geometry_start += gserialized1_box_size(g);
	}

	uint32_t isEmpty = (((uint32_t *)geometry_start)[1]) == 0;
	if (isEmpty) {
		return LW_FAILURE;
	}

	uint32_t type = (((uint32_t *)geometry_start)[0]);
	/* Setup double_array_start depending on the geometry type */
	double *double_array_start = NULL;
	switch (type) {
	case (POINTTYPE):
		/* For points we only need to jump over the type and npoints 32b ints */
		double_array_start = (double *)(geometry_start + 2 * sizeof(uint32_t));
		break;

	default: {
		lwerror("%s is currently not implemented for type %d", __func__, type);
		return LW_FAILURE;
	}
	}

	gserialized1_copy_point(double_array_start, g->gflags, out_point);
	return LW_SUCCESS;
}

/**
 * Read the bounding box off a serialization and calculate one if
 * it is not already there.
 */
int gserialized1_get_gbox_p(const GSERIALIZED *g, GBOX *box) {
	/* Try to just read the serialized box. */
	if (gserialized1_read_gbox_p(g, box) == LW_SUCCESS) {
		return LW_SUCCESS;
	}
	/* No box? Try to peek into simpler geometries and */
	/* derive a box without creating an lwgeom */
	else if (gserialized1_peek_gbox_p(g, box) == LW_SUCCESS) {
		return LW_SUCCESS;
	}
	/* Damn! Nothing for it but to create an lwgeom... */
	/* See http://trac.osgeo.org/postgis/ticket/1023 */
	else {
		LWGEOM *lwgeom = lwgeom_from_gserialized(g);
		int ret = lwgeom_calculate_gbox(lwgeom, box);
		gbox_float_round(box);
		lwgeom_free(lwgeom);
		return ret;
	}
}

/*
 * Populate a bounding box *without* allocating an LWGEOM. Useful
 * for some performance purposes.
 */
int gserialized1_peek_gbox_p(const GSERIALIZED *g, GBOX *gbox) {
	uint32_t type = gserialized1_get_type(g);

	/* Peeking doesn't help if you already have a box or are geodetic */
	if (G1FLAGS_GET_GEODETIC(g->gflags) || G1FLAGS_GET_BBOX(g->gflags)) {
		return LW_FAILURE;
	}

	/* Boxes of points are easy peasy */
	if (type == POINTTYPE) {
		int i = 1; /* Start past <pointtype><padding> */
		double *dptr = (double *)(g->data);

		/* Read the empty flag */
		int32_t *iptr = (int32_t *)(g->data);
		int isempty = (iptr[1] == 0);

		/* EMPTY point has no box */
		if (isempty)
			return LW_FAILURE;

		gbox->xmin = gbox->xmax = dptr[i++];
		gbox->ymin = gbox->ymax = dptr[i++];
		gbox->flags = gserialized1_get_lwflags(g);
		if (G1FLAGS_GET_Z(g->gflags)) {
			gbox->zmin = gbox->zmax = dptr[i++];
		}
		if (G1FLAGS_GET_M(g->gflags)) {
			gbox->mmin = gbox->mmax = dptr[i++];
		}
		gbox_float_round(gbox);
		return LW_SUCCESS;
	}
	/* We can calculate the box of a two-point cartesian line trivially */
	else if (type == LINETYPE) {
		int ndims = G1FLAGS_NDIMS(g->gflags);
		int i = 0; /* Start at <linetype><npoints> */
		double *dptr = (double *)(g->data);
		int32_t *iptr = (int32_t *)(g->data);
		int npoints = iptr[1]; /* Read the npoints */

		/* This only works with 2-point lines */
		if (npoints != 2)
			return LW_FAILURE;

		/* Advance to X */
		/* Past <linetype><npoints> */
		i++;
		gbox->xmin = FP_MIN(dptr[i], dptr[i + ndims]);
		gbox->xmax = FP_MAX(dptr[i], dptr[i + ndims]);

		/* Advance to Y */
		i++;
		gbox->ymin = FP_MIN(dptr[i], dptr[i + ndims]);
		gbox->ymax = FP_MAX(dptr[i], dptr[i + ndims]);

		gbox->flags = gserialized1_get_lwflags(g);
		if (G1FLAGS_GET_Z(g->gflags)) {
			/* Advance to Z */
			i++;
			gbox->zmin = FP_MIN(dptr[i], dptr[i + ndims]);
			gbox->zmax = FP_MAX(dptr[i], dptr[i + ndims]);
		}
		if (G1FLAGS_GET_M(g->gflags)) {
			/* Advance to M */
			i++;
			gbox->mmin = FP_MIN(dptr[i], dptr[i + ndims]);
			gbox->mmax = FP_MAX(dptr[i], dptr[i + ndims]);
		}
		gbox_float_round(gbox);
		return LW_SUCCESS;
	}
	/* We can also do single-entry multi-points */
	else if (type == MULTIPOINTTYPE) {
		int i = 0; /* Start at <multipointtype><ngeoms> */
		double *dptr = (double *)(g->data);
		int32_t *iptr = (int32_t *)(g->data);
		int ngeoms = iptr[1]; /* Read the ngeoms */
		int npoints;

		/* This only works with single-entry multipoints */
		if (ngeoms != 1)
			return LW_FAILURE;

		/* Npoints is at <multipointtype><ngeoms><pointtype><npoints> */
		npoints = iptr[3];

		/* The check below is necessary because we can have a MULTIPOINT
		 * that contains a single, empty POINT (ngeoms = 1, npoints = 0) */
		if (npoints != 1)
			return LW_FAILURE;

		/* Move forward two doubles (four ints) */
		/* Past <multipointtype><ngeoms> */
		/* Past <pointtype><npoints> */
		i += 2;

		/* Read the doubles from the one point */
		gbox->xmin = gbox->xmax = dptr[i++];
		gbox->ymin = gbox->ymax = dptr[i++];
		gbox->flags = gserialized1_get_lwflags(g);
		if (G1FLAGS_GET_Z(g->gflags)) {
			gbox->zmin = gbox->zmax = dptr[i++];
		}
		if (G1FLAGS_GET_M(g->gflags)) {
			gbox->mmin = gbox->mmax = dptr[i++];
		}
		gbox_float_round(gbox);
		return LW_SUCCESS;
	}
	/* And we can do single-entry multi-lines with two vertices (!!!) */
	else if (type == MULTILINETYPE) {
		int ndims = G1FLAGS_NDIMS(g->gflags);
		int i = 0; /* Start at <multilinetype><ngeoms> */
		double *dptr = (double *)(g->data);
		int32_t *iptr = (int32_t *)(g->data);
		int ngeoms = iptr[1]; /* Read the ngeoms */
		int npoints;

		/* This only works with 1-line multilines */
		if (ngeoms != 1)
			return LW_FAILURE;

		/* Npoints is at <multilinetype><ngeoms><linetype><npoints> */
		npoints = iptr[3];

		if (npoints != 2)
			return LW_FAILURE;

		/* Advance to X */
		/* Move forward two doubles (four ints) */
		/* Past <multilinetype><ngeoms> */
		/* Past <linetype><npoints> */
		i += 2;
		gbox->xmin = FP_MIN(dptr[i], dptr[i + ndims]);
		gbox->xmax = FP_MAX(dptr[i], dptr[i + ndims]);

		/* Advance to Y */
		i++;
		gbox->ymin = FP_MIN(dptr[i], dptr[i + ndims]);
		gbox->ymax = FP_MAX(dptr[i], dptr[i + ndims]);

		gbox->flags = gserialized1_get_lwflags(g);
		if (G1FLAGS_GET_Z(g->gflags)) {
			/* Advance to Z */
			i++;
			gbox->zmin = FP_MIN(dptr[i], dptr[i + ndims]);
			gbox->zmax = FP_MAX(dptr[i], dptr[i + ndims]);
		}
		if (G1FLAGS_GET_M(g->gflags)) {
			/* Advance to M */
			i++;
			gbox->mmin = FP_MIN(dptr[i], dptr[i + ndims]);
			gbox->mmax = FP_MAX(dptr[i], dptr[i + ndims]);
		}
		gbox_float_round(gbox);
		return LW_SUCCESS;
	}

	return LW_FAILURE;
}

} // namespace duckdb
