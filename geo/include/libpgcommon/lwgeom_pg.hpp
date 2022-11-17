#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

/**
 * Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *geometry_serialize(LWGEOM *lwgeom);

/**
 * Compare SRIDs of two GSERIALIZEDs and print informative error message if they differ.
 */
void gserialized_error_if_srid_mismatch(const GSERIALIZED *g1, const GSERIALIZED *g2, const char *funcname);

/**
 * Compare SRIDs of GSERIALIZEDs to reference and print informative error message if they differ.
 */
void gserialized_error_if_srid_mismatch_reference(const GSERIALIZED *g1, const int32_t srid, const char *funcname);

} // namespace duckdb
