#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

#ifndef _LIBGEOGRAPHY_MEASUREMENT_H
#define _LIBGEOGRAPHY_MEASUREMENT_H 1

double geography_distance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid);

#endif /* !defined _LIBGEOGRAPHY_MEASUREMENT_H  */

} // namespace duckdb
