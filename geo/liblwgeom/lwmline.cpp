#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

void lwmline_free(LWMLINE *mline) {
	if (!mline)
		return;

	if (mline->bbox)
		lwfree(mline->bbox);

	if (mline->geoms) {
		for (uint32_t i = 0; i < mline->ngeoms; i++)
			if (mline->geoms[i])
				lwline_free(mline->geoms[i]);
		lwfree(mline->geoms);
	}

	lwfree(mline);
}

LWMLINE *lwmline_add_lwline(LWMLINE *mobj, const LWLINE *obj) {
	return (LWMLINE *)lwcollection_add_lwgeom((LWCOLLECTION *)mobj, (LWGEOM *)obj);
}

} // namespace duckdb
