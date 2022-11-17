#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

lwvarlena_t *LWGEOM_asGeoJson(GSERIALIZED *gser, size_t m_dec_digits = OUT_DEFAULT_DECIMAL_DIGITS);

} // namespace duckdb
