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

} // namespace duckdb
