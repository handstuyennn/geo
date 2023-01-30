/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * PostGIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * PostGIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostGIS.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 *
 * Copyright (C) 2001-2006 Refractions Research Inc.
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

LWMPOLY *lwmpoly_construct_empty(int32_t srid, char hasz, char hasm) {
	LWMPOLY *ret = (LWMPOLY *)lwcollection_construct_empty(MULTIPOLYGONTYPE, srid, hasz, hasm);
	return ret;
}

LWMPOLY *lwmpoly_add_lwpoly(LWMPOLY *mobj, const LWPOLY *obj) {
	return (LWMPOLY *)lwcollection_add_lwgeom((LWCOLLECTION *)mobj, (LWGEOM *)obj);
}

/* Deep clone LWPOLY object. POINTARRAY are copied, as is ring array */
LWPOLY *lwpoly_clone_deep(const LWPOLY *g) {
	uint32_t i;
	LWPOLY *ret = (LWPOLY *)lwalloc(sizeof(LWPOLY));
	memcpy(ret, g, sizeof(LWPOLY));
	if (g->bbox)
		ret->bbox = gbox_copy(g->bbox);
	ret->rings = (POINTARRAY **)lwalloc(sizeof(POINTARRAY *) * g->nrings);
	for (i = 0; i < ret->nrings; i++) {
		ret->rings[i] = ptarray_clone_deep(g->rings[i]);
	}
	FLAGS_SET_READONLY(ret->flags, 0);
	return ret;
}

void lwmpoly_free(LWMPOLY *mpoly) {
	uint32_t i;
	if (!mpoly)
		return;
	if (mpoly->bbox)
		lwfree(mpoly->bbox);

	for (i = 0; i < mpoly->ngeoms; i++)
		if (mpoly->geoms && mpoly->geoms[i])
			lwpoly_free(mpoly->geoms[i]);

	if (mpoly->geoms)
		lwfree(mpoly->geoms);

	lwfree(mpoly);
}

} // namespace duckdb
