#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

std::vector<GSERIALIZED *> LWGEOM_dump(GSERIALIZED *geom);

} // namespace duckdb
