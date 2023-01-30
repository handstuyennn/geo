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
 * Copyright 2001-2006 Refractions Research Inc.
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

LWMLINE *lwmline_construct_empty(int32_t srid, char hasz, char hasm) {
	LWMLINE *ret = (LWMLINE *)lwcollection_construct_empty(MULTILINETYPE, srid, hasz, hasm);
	return ret;
}

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
