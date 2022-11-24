#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *centroid(GSERIALIZED *geom);
bool LWGEOM_isring(GSERIALIZED *geom);
GSERIALIZED *ST_Difference(GSERIALIZED *geom1, GSERIALIZED *geom2);

} // namespace duckdb
