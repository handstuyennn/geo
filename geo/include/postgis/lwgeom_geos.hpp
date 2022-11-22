#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *centroid(GSERIALIZED *geom);
bool LWGEOM_isring(GSERIALIZED *geom);

} // namespace duckdb
