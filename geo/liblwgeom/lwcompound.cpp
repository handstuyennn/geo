#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

int lwcompound_is_closed(const LWCOMPOUND *compound) {
	size_t size;
	int npoints = 0;

	if (lwgeom_has_z((LWGEOM *)compound)) {
		size = sizeof(POINT3D);
	} else {
		size = sizeof(POINT2D);
	}

	if (compound->geoms[compound->ngeoms - 1]->type == CIRCSTRINGTYPE) {
		npoints = ((LWCIRCSTRING *)compound->geoms[compound->ngeoms - 1])->points->npoints;
	} else if (compound->geoms[compound->ngeoms - 1]->type == LINETYPE) {
		npoints = ((LWLINE *)compound->geoms[compound->ngeoms - 1])->points->npoints;
	}

	if (memcmp(getPoint_internal((POINTARRAY *)compound->geoms[0]->data, 0),
	           getPoint_internal((POINTARRAY *)compound->geoms[compound->ngeoms - 1]->data, npoints - 1), size)) {
		return LW_FALSE;
	}

	return LW_TRUE;
}

int lwcompound_add_lwgeom(LWCOMPOUND *comp, LWGEOM *geom) {
	LWCOLLECTION *col = (LWCOLLECTION *)comp;

	/* Empty things can't continuously join up with other things */
	if (lwgeom_is_empty(geom)) {
		return LW_FAILURE;
	}

	if (col->ngeoms > 0) {
		POINT4D last, first;
		/* First point of the component we are adding */
		LWLINE *newline = (LWLINE *)geom;
		/* Last point of the previous component */
		LWLINE *prevline = (LWLINE *)(col->geoms[col->ngeoms - 1]);

		getPoint4d_p(newline->points, 0, &first);
		getPoint4d_p(prevline->points, prevline->points->npoints - 1, &last);

		if (!(FP_EQUALS(first.x, last.x) && FP_EQUALS(first.y, last.y))) {
			return LW_FAILURE;
		}
	}

	col = lwcollection_add_lwgeom(col, geom);
	return LW_SUCCESS;
}

} // namespace duckdb
