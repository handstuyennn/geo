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

} // namespace duckdb
