#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

LWCURVEPOLY *lwcurvepoly_construct_empty(int32_t srid, char hasz, char hasm) {
	LWCURVEPOLY *ret;

	ret = (LWCURVEPOLY *)lwalloc(sizeof(LWCURVEPOLY));
	ret->type = CURVEPOLYTYPE;
	ret->flags = lwflags(hasz, hasm, 0);
	ret->srid = srid;
	ret->nrings = 0;
	ret->maxrings = 1; /* Allocate room for sub-members, just in case. */
	ret->rings = (LWGEOM **)lwalloc(ret->maxrings * sizeof(LWGEOM *));
	ret->bbox = NULL;

	return ret;
}

int lwcurvepoly_add_ring(LWCURVEPOLY *poly, LWGEOM *ring) {
	uint32_t i;

	/* Can't do anything with NULLs */
	if (!poly || !ring) {
		return LW_FAILURE;
	}

	/* Check that we're not working with garbage */
	if (poly->rings == NULL && (poly->nrings || poly->maxrings)) {
		// lwerror("Curvepolygon is in inconsistent state. Null memory but non-zero collection counts.");
		return LW_FAILURE;
	}

	/* Check that we're adding an allowed ring type */
	if (!(ring->type == LINETYPE || ring->type == CIRCSTRINGTYPE || ring->type == COMPOUNDTYPE)) {
		return LW_FAILURE;
	}

	/* In case this is a truly empty, make some initial space  */
	if (poly->rings == NULL) {
		poly->maxrings = 2;
		poly->nrings = 0;
		poly->rings = (LWGEOM **)lwalloc(poly->maxrings * sizeof(LWGEOM *));
	}

	/* Allocate more space if we need it */
	if (poly->nrings == poly->maxrings) {
		poly->maxrings *= 2;
		poly->rings = (LWGEOM **)lwrealloc(poly->rings, sizeof(LWGEOM *) * poly->maxrings);
	}

	/* Make sure we don't already have a reference to this geom */
	for (i = 0; i < poly->nrings; i++) {
		if (poly->rings[i] == ring) {
			return LW_SUCCESS;
		}
	}

	/* Add the ring and increment the ring count */
	poly->rings[poly->nrings] = (LWGEOM *)ring;
	poly->nrings++;
	return LW_SUCCESS;
}

} // namespace duckdb
