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

} // namespace duckdb
