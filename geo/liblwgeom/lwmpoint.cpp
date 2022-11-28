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
