#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *BOX2D_to_LWGEOM(GBOX *box);

} // namespace duckdb
