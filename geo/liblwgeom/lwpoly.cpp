#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

#define CHECK_POLY_RINGS_ZM 1

/* construct a new LWPOLY.  arrays (points/points per ring) will NOT be copied
 * use SRID=SRID_UNKNOWN for unknown SRID (will have 8bit type's S = 0)
 */
LWPOLY *lwpoly_construct(int32_t srid, GBOX *bbox, uint32_t nrings, POINTARRAY **points) {
	LWPOLY *result;
	int hasz, hasm;
#ifdef CHECK_POLY_RINGS_ZM
	char zm;
	uint32_t i;
#endif

	if (nrings < 1)
		// lwerror("lwpoly_construct: need at least 1 ring");
		return nullptr;

	hasz = FLAGS_GET_Z(points[0]->flags);
	hasm = FLAGS_GET_M(points[0]->flags);

#ifdef CHECK_POLY_RINGS_ZM
	zm = FLAGS_GET_ZM(points[0]->flags);
	for (i = 1; i < nrings; i++) {
		if (zm != FLAGS_GET_ZM(points[i]->flags))
			// lwerror("lwpoly_construct: mixed dimensioned rings");
			return nullptr;
	}
#endif

	result = (LWPOLY *)lwalloc(sizeof(LWPOLY));
	result->type = POLYGONTYPE;
	result->flags = lwflags(hasz, hasm, 0);
	FLAGS_SET_BBOX(result->flags, bbox ? 1 : 0);
	result->srid = srid;
	result->nrings = nrings;
	result->maxrings = nrings;
	result->rings = points;
	result->bbox = bbox;

	return result;
}

LWPOLY *lwpoly_construct_empty(int32_t srid, char hasz, char hasm) {
	LWPOLY *result = (LWPOLY *)lwalloc(sizeof(LWPOLY));
	result->type = POLYGONTYPE;
	result->flags = lwflags(hasz, hasm, 0);
	result->srid = srid;
	result->nrings = 0;
	result->maxrings = 1; /* Allocate room for ring, just in case. */
	result->rings = (POINTARRAY **)lwalloc(result->maxrings * sizeof(POINTARRAY *));
	result->bbox = NULL;
	return result;
}

void lwpoly_free(LWPOLY *poly) {
	uint32_t t;

	if (!poly)
		return;

	if (poly->bbox)
		lwfree(poly->bbox);

	if (poly->rings) {
		for (t = 0; t < poly->nrings; t++)
			if (poly->rings[t])
				ptarray_free(poly->rings[t]);
		lwfree(poly->rings);
	}

	lwfree(poly);
}

uint32_t lwpoly_count_vertices(LWPOLY *poly) {
	uint32_t i = 0;
	uint32_t v = 0; /* vertices */
	assert(poly);
	for (i = 0; i < poly->nrings; i++) {
		v += poly->rings[i]->npoints;
	}
	return v;
}

LWPOLY *lwpoly_force_dims(const LWPOLY *poly, int hasz, int hasm, double zval, double mval) {
	LWPOLY *polyout;

	/* Return 2D empty */
	if (lwpoly_is_empty(poly)) {
		polyout = lwpoly_construct_empty(poly->srid, hasz, hasm);
	} else {
		POINTARRAY **rings = NULL;
		uint32_t i;
		rings = (POINTARRAY **)lwalloc(sizeof(POINTARRAY *) * poly->nrings);
		for (i = 0; i < poly->nrings; i++) {
			rings[i] = ptarray_force_dims(poly->rings[i], hasz, hasm, zval, mval);
		}
		polyout = lwpoly_construct(poly->srid, NULL, poly->nrings, rings);
	}
	polyout->type = poly->type;
	return polyout;
}

int lwpoly_startpoint(const LWPOLY *poly, POINT4D *pt) {
	if (poly->nrings < 1)
		return LW_FAILURE;
	return ptarray_startpoint(poly->rings[0], pt);
}

/**
 * Add a ring to a polygon. Point array will be referenced, not copied.
 */
int lwpoly_add_ring(LWPOLY *poly, POINTARRAY *pa) {
	if (!poly || !pa)
		return LW_FAILURE;

	/* We have used up our storage, add some more. */
	if (poly->nrings >= poly->maxrings) {
		int new_maxrings = 2 * (poly->nrings + 1);
		poly->rings = (POINTARRAY **)lwrealloc(poly->rings, new_maxrings * sizeof(POINTARRAY *));
		poly->maxrings = new_maxrings;
	}

	/* Add the new ring entry. */
	poly->rings[poly->nrings] = pa;
	poly->nrings++;

	return LW_SUCCESS;
}

} // namespace duckdb
