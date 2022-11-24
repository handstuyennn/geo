#include "liblwgeom/lwgeom_geos.hpp"

#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdarg.h>
#include <stdlib.h>

namespace duckdb {

LWGEOM *lwgeom_difference_prec(const LWGEOM *geom1, const LWGEOM *geom2, double prec) {
	LWGEOM *result = nullptr;
	// 	int32_t srid = RESULT_SRID(geom1, geom2);
	// 	uint8_t is3d = (FLAGS_GET_Z(geom1->flags) || FLAGS_GET_Z(geom2->flags));
	// 	GEOSGeometry *g1, *g2, *g3;

	// 	if (srid == SRID_INVALID)
	// 		return NULL;

	// 	/* A.Intersection(Empty) == Empty */
	// 	if (lwgeom_is_empty(geom2))
	// 		return lwgeom_clone_deep(geom1); /* match empty type? */

	// 	/* Empty.Intersection(A) == Empty */
	// 	if (lwgeom_is_empty(geom1))
	// 		return lwgeom_clone_deep(geom1); /* match empty type? */

	// 	initGEOS(lwnotice, lwgeom_geos_error);

	// 	if (!(g1 = LWGEOM2GEOS(geom1, AUTOFIX)))
	// 		GEOS_FAIL();
	// 	if (!(g2 = LWGEOM2GEOS(geom2, AUTOFIX)))
	// 		GEOS_FREE_AND_FAIL(g1);

	// 	if (prec >= 0) {
	// #if POSTGIS_GEOS_VERSION < 39
	// 		lwerror("Fixed-precision difference requires GEOS-3.9 or higher");
	// 		GEOS_FREE_AND_FAIL(g1, g2);
	// 		return NULL;
	// #else
	// 		g3 = GEOSDifferencePrec(g1, g2, prec);
	// #endif
	// 	} else {
	// 		g3 = GEOSDifference(g1, g2);
	// 	}

	// 	if (!g3)
	// 		GEOS_FREE_AND_FAIL(g1, g2);
	// 	GEOSSetSRID(g3, srid);

	// 	if (!(result = GEOS2LWGEOM(g3, is3d)))
	// 		GEOS_FREE_AND_FAIL(g1, g2, g3);

	// 	GEOS_FREE(g1, g2, g3);
	return result;
}

} // namespace duckdb