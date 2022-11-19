#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GBOX *box2d_from_geohash(char *hash, int precision = -1);

GSERIALIZED *LWGEOM_from_GeoHash(char *hash, int precision = -1);

} // namespace duckdb
