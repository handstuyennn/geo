#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

void lwmpoint_free(LWMPOINT *mpt) {
	uint32_t i;

	if (!mpt)
		return;

	if (mpt->bbox)
		lwfree(mpt->bbox);

	for (i = 0; i < mpt->ngeoms; i++)
		if (mpt->geoms && mpt->geoms[i])
			lwpoint_free(mpt->geoms[i]);

	if (mpt->geoms)
		lwfree(mpt->geoms);

	lwfree(mpt);
}

LWMPOINT *lwmpoint_add_lwpoint(LWMPOINT *mobj, const LWPOINT *obj) {
	return (LWMPOINT *)lwcollection_add_lwgeom((LWCOLLECTION *)mobj, (LWGEOM *)obj);
}

} // namespace duckdb
