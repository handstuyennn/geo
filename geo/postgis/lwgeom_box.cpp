#include "postgis/lwgeom_box.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

#include <cstring>
#include <string>

namespace duckdb {

GSERIALIZED *BOX2D_to_LWGEOM(GBOX *box) {
	POINTARRAY *pa = ptarray_construct_empty(0, 0, 5);
	POINT4D pt;
	GSERIALIZED *result;

	/*
	 * Alter BOX2D cast so that a valid geometry is always
	 * returned depending upon the size of the BOX2D. The
	 * code makes the following assumptions:
	 *     - If the BOX2D is a single point then return a
	 *     POINT geometry
	 *     - If the BOX2D represents either a horizontal or
	 *     vertical line, return a LINESTRING geometry
	 *     - Otherwise return a POLYGON
	 */

	if ((box->xmin == box->xmax) && (box->ymin == box->ymax)) {
		/* Construct and serialize point */
		LWPOINT *point = lwpoint_make2d(SRID_UNKNOWN, box->xmin, box->ymin);
		result = geometry_serialize(lwpoint_as_lwgeom(point));
		lwpoint_free(point);
	} else if ((box->xmin == box->xmax) || (box->ymin == box->ymax)) {
		LWLINE *line;

		/* Assign coordinates to point array */
		pt.x = box->xmin;
		pt.y = box->ymin;
		ptarray_append_point(pa, &pt, LW_TRUE);
		pt.x = box->xmax;
		pt.y = box->ymax;
		ptarray_append_point(pa, &pt, LW_TRUE);

		/* Construct and serialize linestring */
		line = lwline_construct(SRID_UNKNOWN, NULL, pa);
		result = geometry_serialize(lwline_as_lwgeom(line));
		lwline_free(line);
	} else {
		POINT4D points[4];
		LWPOLY *poly;

		/* Initialize the 4 vertices of the polygon */
		points[0] = (POINT4D) {box->xmin, box->ymin, 0.0, 0.0};
		points[1] = (POINT4D) {box->xmin, box->ymax, 0.0, 0.0};
		points[2] = (POINT4D) {box->xmax, box->ymax, 0.0, 0.0};
		points[3] = (POINT4D) {box->xmax, box->ymin, 0.0, 0.0};

		/* Construct polygon */
		poly = lwpoly_construct_rectangle(LW_FALSE, LW_FALSE, &points[0], &points[1], &points[2], &points[3]);
		result = geometry_serialize(lwpoly_as_lwgeom(poly));
		lwpoly_free(poly);
	}

	return result;
}

} // namespace duckdb
