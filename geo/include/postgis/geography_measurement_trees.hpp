#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

#ifndef _LIBGEOGRAPHY_MEASUREMENT_TREES_H
#define _LIBGEOGRAPHY_MEASUREMENT_TREES_H 1

int geography_tree_distance(const GSERIALIZED *g1, const GSERIALIZED *g2, const SPHEROID *s, double tolerance,
                            double *distance);

#endif /* !defined _LIBGEOGRAPHY_MEASUREMENT_TREES_H  */

} // namespace duckdb
