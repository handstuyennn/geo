#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

GSERIALIZED *LWGEOM_makepoint(double x, double y);
GSERIALIZED *LWGEOM_makepoint(double x, double y, double z);
GSERIALIZED *LWGEOM_makeline(GSERIALIZED *geom1, GSERIALIZED *geom2);
GSERIALIZED *LWGEOM_makeline_garray(GSERIALIZED *gserArray[], int nelems);
GSERIALIZED *LWGEOM_makepoly(GSERIALIZED *geom, GSERIALIZED *gserArray[] = {}, int nelems = 0);
double ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2);
lwvarlena_t *ST_GeoHash(GSERIALIZED *gser, size_t m_chars = 0);

} // namespace duckdb
