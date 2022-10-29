#include "liblwgeom/gserialized2.hpp"

#include "liblwgeom/liblwgeom_internal.hpp"

#include <cassert>
#include <cstring>

namespace duckdb {

static inline void gserialized2_copy_point(double *dptr, lwflags_t flags, POINT4D *out_point) {
	uint8_t dim = 0;
	out_point->x = dptr[dim++];
	out_point->y = dptr[dim++];

	if (G2FLAGS_GET_Z(flags)) {
		out_point->z = dptr[dim++];
	}
	if (G2FLAGS_GET_M(flags)) {
		out_point->m = dptr[dim];
	}
}

static size_t gserialized2_from_lwgeom_any(const LWGEOM *geom, uint8_t *buf);

/* handle missaligned uint32_t data */
static inline uint32_t gserialized2_get_uint32_t(const uint8_t *loc) {
	return *((uint32_t *)loc);
}

lwflags_t gserialized2_get_lwflags(const GSERIALIZED *g) {
	lwflags_t lwflags = 0;
	uint8_t gflags = g->gflags;
	FLAGS_SET_Z(lwflags, G2FLAGS_GET_Z(gflags));
	FLAGS_SET_M(lwflags, G2FLAGS_GET_M(gflags));
	FLAGS_SET_BBOX(lwflags, G2FLAGS_GET_BBOX(gflags));
	FLAGS_SET_GEODETIC(lwflags, G2FLAGS_GET_GEODETIC(gflags));
	if (G2FLAGS_GET_EXTENDED(gflags)) {
		uint64_t xflags = 0;
		memcpy(&xflags, g->data, sizeof(uint64_t));
		FLAGS_SET_SOLID(lwflags, xflags & G2FLAG_X_SOLID);
	}
	return lwflags;
}

static int lwflags_uses_extended_flags(lwflags_t lwflags) {
	lwflags_t core_lwflags = LWFLAG_Z | LWFLAG_M | LWFLAG_BBOX | LWFLAG_GEODETIC;
	return (lwflags & (~core_lwflags)) != 0;
}

uint8_t lwflags_get_g2flags(lwflags_t lwflags) {
	uint8_t gflags = 0;
	G2FLAGS_SET_Z(gflags, FLAGS_GET_Z(lwflags));
	G2FLAGS_SET_M(gflags, FLAGS_GET_M(lwflags));
	G2FLAGS_SET_BBOX(gflags, FLAGS_GET_BBOX(lwflags));
	G2FLAGS_SET_GEODETIC(gflags, FLAGS_GET_GEODETIC(lwflags));
	G2FLAGS_SET_EXTENDED(gflags, lwflags_uses_extended_flags(lwflags));
	G2FLAGS_SET_VERSION(gflags, 1);
	return gflags;
}

static inline size_t gserialized2_box_size(const GSERIALIZED *g) {
	if (G2FLAGS_GET_GEODETIC(g->gflags))
		return 6 * sizeof(float);
	else
		return 2 * G2FLAGS_NDIMS(g->gflags) * sizeof(float);
}

/* Returns a pointer to the start of the geometry data */
static inline uint8_t *gserialized2_get_geometry_p(const GSERIALIZED *g) {
	uint32_t extra_data_bytes = 0;
	if (gserialized2_has_extended(g))
		extra_data_bytes += sizeof(uint64_t);

	if (gserialized2_has_bbox(g))
		extra_data_bytes += gserialized2_box_size(g);

	return ((uint8_t *)g->data) + extra_data_bytes;
}

uint32_t gserialized2_get_type(const GSERIALIZED *g) {
	uint8_t *ptr = gserialized2_get_geometry_p(g);
	return *((uint32_t *)(ptr));
}

int32_t gserialized2_get_srid(const GSERIALIZED *g) {
	int32_t srid = 0;
	srid = srid | (g->srid[0] << 16);
	srid = srid | (g->srid[1] << 8);
	srid = srid | (g->srid[2]);
	/* Only the first 21 bits are set. Slide up and back to pull
	   the negative bits down, if we need them. */
	srid = (srid << 11) >> 11;

	/* 0 is our internal unknown value. We'll map back and forth here for now */
	if (srid == 0)
		return SRID_UNKNOWN;
	else
		return srid;
}

static size_t gserialized2_from_extended_flags(lwflags_t lwflags, uint8_t *buf) {
	if (lwflags_uses_extended_flags(lwflags)) {
		uint64_t xflags = 0;
		if (FLAGS_GET_SOLID(lwflags))
			xflags |= G2FLAG_X_SOLID;

		// G2FLAG_X_CHECKED_VALID
		// G2FLAG_X_IS_VALID
		// G2FLAG_X_HAS_HASH

		memcpy(buf, &xflags, sizeof(uint64_t));
		return sizeof(uint64_t);
	}
	return 0;
}

void gserialized2_set_srid(GSERIALIZED *g, int32_t srid) {
	srid = clamp_srid(srid);

	/* 0 is our internal unknown value.
	 * We'll map back and forth here for now */
	if (srid == SRID_UNKNOWN)
		srid = 0;

	g->srid[0] = (srid & 0x001F0000) >> 16;
	g->srid[1] = (srid & 0x0000FF00) >> 8;
	g->srid[2] = (srid & 0x000000FF);
}

static size_t gserialized2_from_gbox(const GBOX *gbox, uint8_t *buf) {
	uint8_t *loc = buf;
	float *f;
	uint8_t i = 0;
	size_t return_size;

	assert(buf);

	f = (float *)buf;
	f[i++] = next_float_down(gbox->xmin);
	f[i++] = next_float_up(gbox->xmax);
	f[i++] = next_float_down(gbox->ymin);
	f[i++] = next_float_up(gbox->ymax);
	loc += 4 * sizeof(float);

	if (FLAGS_GET_GEODETIC(gbox->flags)) {
		f[i++] = next_float_down(gbox->zmin);
		f[i++] = next_float_up(gbox->zmax);
		loc += 2 * sizeof(float);

		return_size = (size_t)(loc - buf);
		return return_size;
	}

	if (FLAGS_GET_Z(gbox->flags)) {
		f[i++] = next_float_down(gbox->zmin);
		f[i++] = next_float_up(gbox->zmax);
		loc += 2 * sizeof(float);
	}

	if (FLAGS_GET_M(gbox->flags)) {
		f[i++] = next_float_down(gbox->mmin);
		f[i++] = next_float_up(gbox->mmax);
		loc += 2 * sizeof(float);
	}
	return_size = (size_t)(loc - buf);
	return return_size;
}

static size_t gserialized2_is_empty_recurse(const uint8_t *p, int *isempty) {
	int i;
	int32_t type, num;

	memcpy(&type, p, 4);
	memcpy(&num, p + 4, 4);

	if (lwtype_is_collection(type)) {
		size_t lz = 8;
		for (i = 0; i < num; i++) {
			lz += gserialized2_is_empty_recurse(p + lz, isempty);
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

int gserialized2_is_empty(const GSERIALIZED *g) {
	int isempty = 0;
	uint8_t *p = gserialized2_get_geometry_p(g);
	gserialized2_is_empty_recurse(p, &isempty);
	return isempty;
}

int gserialized2_has_bbox(const GSERIALIZED *g) {
	return G2FLAGS_GET_BBOX(g->gflags);
}

int gserialized2_has_extended(const GSERIALIZED *g) {
	return G2FLAGS_GET_EXTENDED(g->gflags);
}

/* Public function */
GSERIALIZED *gserialized2_from_lwgeom(LWGEOM *geom, size_t *size) {
	size_t expected_size = 0;
	size_t return_size = 0;
	uint8_t *ptr = NULL;
	GSERIALIZED *g = NULL;
	assert(geom);

	/*
	** See if we need a bounding box, add one if we don't have one.
	*/
	if ((!geom->bbox) && lwgeom_needs_bbox(geom) && (!lwgeom_is_empty(geom))) {
		lwgeom_add_bbox(geom);
	}

	/*
	** Harmonize the flags to the state of the lwgeom
	*/
	FLAGS_SET_BBOX(geom->flags, (geom->bbox ? 1 : 0));

	/* Set up the uint8_t buffer into which we are going to write the serialized geometry. */
	expected_size = gserialized2_from_lwgeom_size(geom);
	ptr = (uint8_t *)lwalloc(expected_size);
	g = (GSERIALIZED *)(ptr);

	/* Set the SRID! */
	gserialized2_set_srid(g, geom->srid);
	/*
	** We are aping PgSQL code here, PostGIS code should use
	** VARSIZE to set this for real.
	*/
	LWSIZE_SET(g->size, expected_size);
	g->gflags = lwflags_get_g2flags(geom->flags);

	/* Move write head past size, srid and flags. */
	ptr += 8;

	/* Write in the extended flags if necessary */
	ptr += gserialized2_from_extended_flags(geom->flags, ptr);

	/* Write in the serialized form of the gbox, if necessary. */
	if (geom->bbox)
		ptr += gserialized2_from_gbox(geom->bbox, ptr);

	/* Write in the serialized form of the geometry. */
	ptr += gserialized2_from_lwgeom_any(geom, ptr);

	/* Calculate size as returned by data processing functions. */
	return_size = ptr - (uint8_t *)g;

	assert(expected_size == return_size);
	if (size) /* Return the output size to the caller if necessary. */
		*size = return_size;

	return g;
}

int gserialized2_read_gbox_p(const GSERIALIZED *g, GBOX *gbox) {
	uint8_t gflags = g->gflags;
	/* Null input! */
	if (!(g && gbox))
		return LW_FAILURE;

	/* Initialize the flags on the box */
	gbox->flags = gserialized2_get_lwflags(g);

	/* Has pre-calculated box */
	if (G2FLAGS_GET_BBOX(gflags)) {
		int i = 0;
		const float *fbox = gserialized2_get_float_box_p(g, NULL);
		gbox->xmin = fbox[i++];
		gbox->xmax = fbox[i++];
		gbox->ymin = fbox[i++];
		gbox->ymax = fbox[i++];

		/* Geodetic? Read next dimension (geocentric Z) and return */
		if (G2FLAGS_GET_GEODETIC(gflags)) {
			gbox->zmin = fbox[i++];
			gbox->zmax = fbox[i++];
			return LW_SUCCESS;
		}
		/* Cartesian? Read extra dimensions (if there) and return */
		if (G2FLAGS_GET_Z(gflags)) {
			gbox->zmin = fbox[i++];
			gbox->zmax = fbox[i++];
		}
		if (G2FLAGS_GET_M(gflags)) {
			gbox->mmin = fbox[i++];
			gbox->mmax = fbox[i++];
		}
		return LW_SUCCESS;
	}
	return LW_FAILURE;
}

/***********************************************************************
 * De-serialize GSERIALIZED into an LWGEOM.
 */

static LWGEOM *lwgeom_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size, int32_t srid);

static LWPOINT *lwpoint_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size, int32_t srid) {
	uint8_t *start_ptr = data_ptr;
	LWPOINT *point;
	uint32_t npoints = 0;

	assert(data_ptr);

	point = (LWPOINT *)lwalloc(sizeof(LWPOINT));
	point->srid = srid;
	point->bbox = NULL;
	point->type = POINTTYPE;
	point->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the type. */
	npoints = gserialized2_get_uint32_t(data_ptr); /* Zero => empty geometry */
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

static LWLINE *lwline_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size, int32_t srid) {
	uint8_t *start_ptr = data_ptr;
	LWLINE *line;
	uint32_t npoints = 0;

	assert(data_ptr);

	line = (LWLINE *)lwalloc(sizeof(LWLINE));
	line->srid = srid;
	line->bbox = NULL;
	line->type = LINETYPE;
	line->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the type. */
	npoints = gserialized2_get_uint32_t(data_ptr); /* Zero => empty geometry */
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

static LWPOLY *lwpoly_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size, int32_t srid) {
	uint8_t *start_ptr = data_ptr;
	LWPOLY *poly;
	uint8_t *ordinate_ptr;
	uint32_t nrings = 0;
	uint32_t i = 0;

	assert(data_ptr);

	poly = (LWPOLY *)lwalloc(sizeof(LWPOLY));
	poly->srid = srid;
	poly->bbox = NULL;
	poly->type = POLYGONTYPE;
	poly->flags = lwflags;

	data_ptr += 4;                                /* Skip past the polygontype. */
	nrings = gserialized2_get_uint32_t(data_ptr); /* Zero => empty geometry */
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
		npoints = gserialized2_get_uint32_t(data_ptr);
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

static LWTRIANGLE *lwtriangle_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size,
                                                       int32_t srid) {
	uint8_t *start_ptr = data_ptr;
	LWTRIANGLE *triangle;
	uint32_t npoints = 0;

	assert(data_ptr);

	triangle = (LWTRIANGLE *)lwalloc(sizeof(LWTRIANGLE));
	triangle->srid = srid; /* Default */
	triangle->bbox = NULL;
	triangle->type = TRIANGLETYPE;
	triangle->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the type. */
	npoints = gserialized2_get_uint32_t(data_ptr); /* Zero => empty geometry */
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

static LWCIRCSTRING *lwcircstring_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size,
                                                           int32_t srid) {
	uint8_t *start_ptr = data_ptr;
	LWCIRCSTRING *circstring;
	uint32_t npoints = 0;

	assert(data_ptr);

	circstring = (LWCIRCSTRING *)lwalloc(sizeof(LWCIRCSTRING));
	circstring->srid = srid;
	circstring->bbox = NULL;
	circstring->type = CIRCSTRINGTYPE;
	circstring->flags = lwflags;

	data_ptr += 4;                                 /* Skip past the circstringtype. */
	npoints = gserialized2_get_uint32_t(data_ptr); /* Zero => empty geometry */
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

static LWCOLLECTION *lwcollection_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *size,
                                                           int32_t srid) {
	uint32_t type;
	uint8_t *start_ptr = data_ptr;
	LWCOLLECTION *collection;
	uint32_t ngeoms = 0;
	uint32_t i = 0;

	assert(data_ptr);

	type = gserialized2_get_uint32_t(data_ptr);
	data_ptr += 4; /* Skip past the type. */

	collection = (LWCOLLECTION *)lwalloc(sizeof(LWCOLLECTION));
	collection->srid = srid;
	collection->bbox = NULL;
	collection->type = type;
	collection->flags = lwflags;

	ngeoms = gserialized2_get_uint32_t(data_ptr);
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
		uint32_t subtype = gserialized2_get_uint32_t(data_ptr);
		size_t subsize = 0;

		if (!lwcollection_allows_subtype(type, subtype)) {
			// lwerror("Invalid subtype (%s) for collection type (%s)", lwtype_name(subtype), lwtype_name(type));
			lwfree(collection);
			return NULL;
		}
		collection->geoms[i] = lwgeom_from_gserialized2_buffer(data_ptr, lwflags, &subsize, srid);
		data_ptr += subsize;
	}

	if (size)
		*size = data_ptr - start_ptr;

	return collection;
}

LWGEOM *lwgeom_from_gserialized2_buffer(uint8_t *data_ptr, lwflags_t lwflags, size_t *g_size, int32_t srid) {
	uint32_t type;

	assert(data_ptr);

	type = gserialized2_get_uint32_t(data_ptr);

	switch (type) {
	case POINTTYPE:
		return (LWGEOM *)lwpoint_from_gserialized2_buffer(data_ptr, lwflags, g_size, srid);
	case LINETYPE:
		return (LWGEOM *)lwline_from_gserialized2_buffer(data_ptr, lwflags, g_size, srid);
	case POLYGONTYPE:
		return (LWGEOM *)lwpoly_from_gserialized2_buffer(data_ptr, lwflags, g_size, srid);
	case CIRCSTRINGTYPE:
		return (LWGEOM *)lwcircstring_from_gserialized2_buffer(data_ptr, lwflags, g_size, srid);
	case TRIANGLETYPE:
		return (LWGEOM *)lwtriangle_from_gserialized2_buffer(data_ptr, lwflags, g_size, srid);
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
		return (LWGEOM *)lwcollection_from_gserialized2_buffer(data_ptr, lwflags, g_size, srid);
		// Need to do with postgis

	default:
		// lwerror("Unknown geometry type: %d - %s", type, lwtype_name(type));
		return NULL;
	}
}

/***********************************************************************
 * Calculate the GSERIALIZED size for an LWGEOM.
 */

/* Private functions */

static size_t gserialized2_from_any_size(const LWGEOM *geom); /* Local prototype */

static size_t gserialized2_from_lwpoint_size(const LWPOINT *point) {
	size_t size = 4; /* Type number. */

	assert(point);

	size += 4; /* Number of points (one or zero (empty)). */
	size += point->point->npoints * FLAGS_NDIMS(point->flags) * sizeof(double);

	return size;
}

static size_t gserialized2_from_lwline_size(const LWLINE *line) {
	size_t size = 4; /* Type number. */

	assert(line);

	size += 4; /* Number of points (zero => empty). */
	size += line->points->npoints * FLAGS_NDIMS(line->flags) * sizeof(double);

	return size;
}

static size_t gserialized2_from_lwtriangle_size(const LWTRIANGLE *triangle) {
	size_t size = 4; /* Type number. */

	assert(triangle);

	size += 4; /* Number of points (zero => empty). */
	size += triangle->points->npoints * FLAGS_NDIMS(triangle->flags) * sizeof(double);

	return size;
}

static size_t gserialized2_from_lwpoly_size(const LWPOLY *poly) {
	size_t size = 4; /* Type number. */
	uint32_t i = 0;
	const size_t point_size = FLAGS_NDIMS(poly->flags) * sizeof(double);

	assert(poly);

	size += 4; /* Number of rings (zero => empty). */
	if (poly->nrings % 2)
		size += 4; /* Padding to double alignment. */

	for (i = 0; i < poly->nrings; i++) {
		size += 4; /* Number of points in ring. */
		size += poly->rings[i]->npoints * point_size;
	}

	return size;
}

static size_t gserialized2_from_lwcircstring_size(const LWCIRCSTRING *curve) {
	size_t size = 4; /* Type number. */

	assert(curve);

	size += 4; /* Number of points (zero => empty). */
	size += curve->points->npoints * FLAGS_NDIMS(curve->flags) * sizeof(double);

	return size;
}

static size_t gserialized2_from_lwcollection_size(const LWCOLLECTION *col) {
	size_t size = 4; /* Type number. */
	uint32_t i = 0;

	assert(col);

	size += 4; /* Number of sub-geometries (zero => empty). */

	for (i = 0; i < col->ngeoms; i++) {
		size_t subsize = gserialized2_from_any_size(col->geoms[i]);
		size += subsize;
	}

	return size;
}

static size_t gserialized2_from_any_size(const LWGEOM *geom) {
	switch (geom->type) {
	case POINTTYPE:
		return gserialized2_from_lwpoint_size((LWPOINT *)geom);
	case LINETYPE:
		return gserialized2_from_lwline_size((LWLINE *)geom);
	case POLYGONTYPE:
		return gserialized2_from_lwpoly_size((LWPOLY *)geom);
	case CIRCSTRINGTYPE:
		return gserialized2_from_lwcircstring_size((LWCIRCSTRING *)geom);
	case TRIANGLETYPE:
		return gserialized2_from_lwtriangle_size((LWTRIANGLE *)geom);
	case CURVEPOLYTYPE:
	case COMPOUNDTYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTICURVETYPE:
	case MULTIPOLYGONTYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return gserialized2_from_lwcollection_size((LWCOLLECTION *)geom);
	// Need to do with postgis
	default:
		// lwerror("Unknown geometry type: %d - %s", geom->type, lwtype_name(geom->type));
		return 0;
	}
}

/* Public function */
size_t gserialized2_from_lwgeom_size(const LWGEOM *geom) {
	size_t size = 8; /* Header overhead (varsize+flags+srid) */
	assert(geom);

	/* Reserve space for extended flags */
	if (lwflags_uses_extended_flags(geom->flags))
		size += 8;

	/* Reserve space for bounding box */
	if (geom->bbox)
		size += gbox_serialized_size(geom->flags);

	size += gserialized2_from_any_size(geom);

	return size;
}

static size_t gserialized2_from_lwpoint(const LWPOINT *point, uint8_t *buf) {
	uint8_t *loc;
	int ptsize = ptarray_point_size(point->point);
	int type = POINTTYPE;

	assert(point);
	assert(buf);

	if (FLAGS_GET_ZM(point->flags) != FLAGS_GET_ZM(point->point->flags))
		// lwerror("Dimensions mismatch in lwpoint");
		return 0;

	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);
	/* Write in the number of points (0 => empty). */
	memcpy(loc, &(point->point->npoints), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Copy in the ordinates. */
	if (point->point->npoints > 0) {
		memcpy(loc, getPoint_internal(point->point, 0), ptsize);
		loc += ptsize;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized2_from_lwline(const LWLINE *line, uint8_t *buf) {
	uint8_t *loc;
	int ptsize;
	size_t size;
	int type = LINETYPE;

	assert(line);
	assert(buf);

	if (FLAGS_GET_Z(line->flags) != FLAGS_GET_Z(line->points->flags))
		// lwerror("Dimensions mismatch in lwline");
		return 0;

	ptsize = ptarray_point_size(line->points);

	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints. */
	memcpy(loc, &(line->points->npoints), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Copy in the ordinates. */
	if (line->points->npoints > 0) {
		size = line->points->npoints * ptsize;
		memcpy(loc, getPoint_internal(line->points, 0), size);
		loc += size;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized2_from_lwpoly(const LWPOLY *poly, uint8_t *buf) {
	uint32_t i;
	uint8_t *loc;
	int ptsize;
	int type = POLYGONTYPE;

	assert(poly);
	assert(buf);

	ptsize = sizeof(double) * FLAGS_NDIMS(poly->flags);
	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the nrings. */
	memcpy(loc, &(poly->nrings), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints per ring. */
	for (i = 0; i < poly->nrings; i++) {
		memcpy(loc, &(poly->rings[i]->npoints), sizeof(uint32_t));
		loc += sizeof(uint32_t);
	}

	/* Add in padding if necessary to remain double aligned. */
	if (poly->nrings % 2) {
		memset(loc, 0, sizeof(uint32_t));
		loc += sizeof(uint32_t);
	}

	/* Copy in the ordinates. */
	for (i = 0; i < poly->nrings; i++) {
		POINTARRAY *pa = poly->rings[i];
		size_t pasize;

		if (FLAGS_GET_ZM(poly->flags) != FLAGS_GET_ZM(pa->flags))
			// lwerror("Dimensions mismatch in lwpoly");
			return 0;

		pasize = pa->npoints * ptsize;
		if (pa->npoints > 0)
			memcpy(loc, getPoint_internal(pa, 0), pasize);
		loc += pasize;
	}
	return (size_t)(loc - buf);
}

static size_t gserialized2_from_lwtriangle(const LWTRIANGLE *triangle, uint8_t *buf) {
	uint8_t *loc;
	int ptsize;
	size_t size;
	int type = TRIANGLETYPE;

	assert(triangle);
	assert(buf);

	if (FLAGS_GET_ZM(triangle->flags) != FLAGS_GET_ZM(triangle->points->flags))
		// lwerror("Dimensions mismatch in lwtriangle");
		return 0;

	ptsize = ptarray_point_size(triangle->points);

	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints. */
	memcpy(loc, &(triangle->points->npoints), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Copy in the ordinates. */
	if (triangle->points->npoints > 0) {
		size = triangle->points->npoints * ptsize;
		memcpy(loc, getPoint_internal(triangle->points, 0), size);
		loc += size;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized2_from_lwcircstring(const LWCIRCSTRING *curve, uint8_t *buf) {
	uint8_t *loc;
	int ptsize;
	size_t size;
	int type = CIRCSTRINGTYPE;

	assert(curve);
	assert(buf);

	if (FLAGS_GET_ZM(curve->flags) != FLAGS_GET_ZM(curve->points->flags))
		// lwerror("Dimensions mismatch in lwcircstring");
		return 0;

	ptsize = ptarray_point_size(curve->points);
	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints. */
	memcpy(loc, &curve->points->npoints, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Copy in the ordinates. */
	if (curve->points->npoints > 0) {
		size = curve->points->npoints * ptsize;
		memcpy(loc, getPoint_internal(curve->points, 0), size);
		loc += size;
	}

	return (size_t)(loc - buf);
}

LWGEOM *lwgeom_from_gserialized2(const GSERIALIZED *g) {
	lwflags_t lwflags = 0;
	int32_t srid = 0;
	uint32_t lwtype = 0;
	uint8_t *data_ptr = NULL;
	LWGEOM *lwgeom = NULL;
	GBOX bbox;
	size_t size = 0;

	assert(g);

	srid = gserialized2_get_srid(g);
	lwtype = gserialized2_get_type(g);
	lwflags = gserialized2_get_lwflags(g);

	data_ptr = (uint8_t *)g->data;

	/* Skip optional flags */
	if (G2FLAGS_GET_EXTENDED(g->gflags)) {
		data_ptr += sizeof(uint64_t);
	}

	/* Skip over optional bounding box */
	if (FLAGS_GET_BBOX(lwflags))
		data_ptr += gbox_serialized_size(lwflags);

	lwgeom = lwgeom_from_gserialized2_buffer(data_ptr, lwflags, &size, srid);

	if (!lwgeom)
		// lwerror("%s: unable create geometry", __func__); /* Ooops! */
		return NULL;

	lwgeom->type = lwtype;
	lwgeom->flags = lwflags;

	if (gserialized2_read_gbox_p(g, &bbox) == LW_SUCCESS) {
		lwgeom->bbox = gbox_copy(&bbox);
	} else if (lwgeom_needs_bbox(lwgeom) && (lwgeom_calculate_gbox(lwgeom, &bbox) == LW_SUCCESS)) {
		lwgeom->bbox = gbox_copy(&bbox);
	} else {
		lwgeom->bbox = NULL;
	}

	return lwgeom;
}

static size_t gserialized2_from_lwcollection(const LWCOLLECTION *coll, uint8_t *buf) {
	size_t subsize = 0;
	uint8_t *loc;
	uint32_t i;
	int type;

	assert(coll);
	assert(buf);

	type = coll->type;
	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the number of subgeoms. */
	memcpy(loc, &coll->ngeoms, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Serialize subgeoms. */
	for (i = 0; i < coll->ngeoms; i++) {
		if (FLAGS_GET_ZM(coll->flags) != FLAGS_GET_ZM(coll->geoms[i]->flags))
			// lwerror("Dimensions mismatch in lwcollection");
			return 0;
		subsize = gserialized2_from_lwgeom_any(coll->geoms[i], loc);
		loc += subsize;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized2_from_lwgeom_any(const LWGEOM *geom, uint8_t *buf) {
	assert(geom);
	assert(buf);

	switch (geom->type) {
	case POINTTYPE:
		return gserialized2_from_lwpoint((LWPOINT *)geom, buf);
	case LINETYPE:
		return gserialized2_from_lwline((LWLINE *)geom, buf);
	case POLYGONTYPE:
		return gserialized2_from_lwpoly((LWPOLY *)geom, buf);
	case TRIANGLETYPE:
		return gserialized2_from_lwtriangle((LWTRIANGLE *)geom, buf);
	case CIRCSTRINGTYPE:
		return gserialized2_from_lwcircstring((LWCIRCSTRING *)geom, buf);
	case CURVEPOLYTYPE:
	case COMPOUNDTYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTICURVETYPE:
	case MULTIPOLYGONTYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return gserialized2_from_lwcollection((LWCOLLECTION *)geom, buf);
	// Need to do with postgis
	default:
		// lwerror("Unknown geometry type: %d - %s", geom->type, lwtype_name(geom->type));
		return 0;
	}
}

const float *gserialized2_get_float_box_p(const GSERIALIZED *g, size_t *ndims) {
	uint8_t *ptr = (uint8_t *)(g->data);
	size_t bndims = G2FLAGS_NDIMS_BOX(g->gflags);

	if (ndims)
		*ndims = bndims;

	/* Cannot do anything if there's no box */
	if (!(g && gserialized_has_bbox(g)))
		return NULL;

	/* Advance past optional extended flags */
	if (gserialized2_has_extended(g))
		ptr += 8;

	return (const float *)(ptr);
}

int gserialized2_peek_first_point(const GSERIALIZED *g, POINT4D *out_point) {
	uint8_t *geometry_start = gserialized2_get_geometry_p(g);

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

	default:
		// lwerror("%s is currently not implemented for type %d", __func__, type);
		return LW_FAILURE;
	}

	gserialized2_copy_point(double_array_start, g->gflags, out_point);
	return LW_SUCCESS;
}

} // namespace duckdb
