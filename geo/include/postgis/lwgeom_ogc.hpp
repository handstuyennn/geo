#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *LWGEOM_from_text(char *text, int srid = SRID_UNKNOWN);
GSERIALIZED *LWGEOM_from_WKB(const char *bytea_wkb, size_t byte_size, int srid = SRID_UNKNOWN);
GSERIALIZED *LWGEOM_boundary(GSERIALIZED *geom);
int LWGEOM_dimension(GSERIALIZED *geom);
GSERIALIZED *LWGEOM_endpoint_linestring(GSERIALIZED *geom);
std::string geometry_geometrytype(GSERIALIZED *geom);
bool LWGEOM_isclosed(GSERIALIZED *geom);
int LWGEOM_numgeometries_collection(GSERIALIZED *geom);
int LWGEOM_numpoints_linestring(GSERIALIZED *geom);
GSERIALIZED *LWGEOM_pointn_linestring(GSERIALIZED *geom, int where);
double LWGEOM_x_point(const void *base, size_t size);

} // namespace duckdb
