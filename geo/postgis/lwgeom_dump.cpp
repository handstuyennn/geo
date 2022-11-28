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
 * Copyright 2001-2009 Refractions Research Inc.
 *
 **********************************************************************/

#include "postgis/lwgeom_dump.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

#include <cstring>
#include <string>

namespace duckdb {

std::vector<GSERIALIZED *> dump_recursive(LWGEOM *lwgeom);

std::vector<GSERIALIZED *> dump_recursive(LWGEOM *lwgeom) {
	if (lwgeom_is_empty(lwgeom))
		return {};

	if (!lwgeom_is_collection(lwgeom)) {
		auto ret = geometry_serialize(lwgeom);
		return {ret};
	}

	std::vector<GSERIALIZED *> ret;
	LWCOLLECTION *lwcoll = (LWCOLLECTION *)lwgeom;
	uint32_t i;
	LWGEOM *subgeom;

	for (i = 0; i < lwcoll->ngeoms; i++) {
		subgeom = lwcoll->geoms[i];
		auto subVec = dump_recursive(subgeom);
		for (auto gser : subVec) {
			ret.push_back(gser);
		}
	}

	return ret;
}

std::vector<GSERIALIZED *> LWGEOM_dump(GSERIALIZED *geom) {
	LWGEOM *lwgeom;

	lwgeom = lwgeom_from_gserialized(geom);

	/* Return nothing for empties */
	if (lwgeom_is_empty(lwgeom))
		return {};

	auto ret = dump_recursive(lwgeom);
	lwgeom_free(lwgeom);

	return ret;
}

} // namespace duckdb
