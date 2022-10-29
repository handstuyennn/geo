#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

LWMPOLY *lwmpoly_add_lwpoly(LWMPOLY *mobj, const LWPOLY *obj) {
	return (LWMPOLY *)lwcollection_add_lwgeom((LWCOLLECTION *)mobj, (LWGEOM *)obj);
}

} // namespace duckdb
