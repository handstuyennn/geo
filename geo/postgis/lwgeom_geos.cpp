#include "postgis/lwgeom_geos.hpp"

#include "liblwgeom/gserialized.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

namespace duckdb {

GSERIALIZED *centroid(GSERIALIZED *geom) {
	GSERIALIZED *result;
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);

	result = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);

	return result;
}

bool LWGEOM_isring(GSERIALIZED *geom) {
	// GEOSGeometry *g1;
	// int result;

	// geom = PG_GETARG_GSERIALIZED_P(0);

	// /* Empty things can't close */
	// if ( gserialized_is_empty(geom) )
	// 	PG_RETURN_BOOL(false);

	// initGEOS(lwpgnotice, lwgeom_geos_error);

	// g1 = POSTGIS2GEOS(geom);
	// if (!g1)
	// 	HANDLE_GEOS_ERROR("First argument geometry could not be converted to GEOS");

	// if ( GEOSGeomTypeId(g1) != GEOS_LINESTRING )
	// {
	// 	GEOSGeom_destroy(g1);
	// 	elog(ERROR, "ST_IsRing() should only be called on a linear feature");
	// }

	// result = GEOSisRing(g1);
	// GEOSGeom_destroy(g1);

	// if (result == 2) HANDLE_GEOS_ERROR("GEOSisRing");

	// PG_FREE_IF_COPY(geom, 0);
	// PG_RETURN_BOOL(result);
	return false;
}

} // namespace duckdb
