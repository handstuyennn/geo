#include "postgis/lwgeom_in_geohash.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "postgis/lwgeom_box.hpp"

#include <cstring>
#include <string>

namespace duckdb {

static GBOX *parse_geohash(char *geohash, int precision) {
	GBOX *box = NULL;
	double lat[2], lon[2];

	if (NULL == geohash) {
		// geohash_lwpgerror("invalid GeoHash representation", 2);
		return nullptr;
	}

	decode_geohash_bbox(geohash, lat, lon, precision);

	box = gbox_new(lwflags(0, 0, 1));

	box->xmin = lon[0];
	box->ymin = lat[0];

	box->xmax = lon[1];
	box->ymax = lat[1];

	return box;
}

GBOX *box2d_from_geohash(char *geohash, int precision) {
	GBOX *box = NULL;
	return parse_geohash(geohash, precision);
}

GSERIALIZED *LWGEOM_from_GeoHash(char *geohash, int precision) {
	auto box = parse_geohash(geohash, precision);
	auto gser = BOX2D_to_LWGEOM(box);
	lwfree(box);

	return gser;
}

} // namespace duckdb
