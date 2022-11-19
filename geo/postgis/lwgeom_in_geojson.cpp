#include "liblwgeom/gserialized.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "postgis/lwgeom_inout.hpp"

#include <cstring>
#include <string>

namespace duckdb {

GSERIALIZED *geom_from_geojson(char *geojson) {
	GSERIALIZED *geom;
	LWGEOM *lwgeom;
	char *srs = NULL;
	int32_t srid = WGS84_SRID;

	lwgeom = lwgeom_from_geojson(geojson, &srs);
	if (!lwgeom) {
		/* Shouldn't get here */
		// elog(ERROR, "lwgeom_from_geojson returned NULL");
		return nullptr;
	}

	// if (srs) {
	// 	srid = GetSRIDCacheBySRS(fcinfo, srs);
	// 	lwfree(srs);
	// }

	lwgeom_set_srid(lwgeom, srid);
	geom = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);

	return geom;
}

} // namespace duckdb
