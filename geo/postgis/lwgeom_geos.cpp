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
 * Copyright 2009-2014 Sandro Santilli <strk@kbt.io>
 * Copyright 2008 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright 2001-2003 Refractions Research Inc.
 *
 **********************************************************************/

#include "postgis/lwgeom_geos.hpp"

#include "liblwgeom/gserialized.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

namespace duckdb {

GSERIALIZED *centroid(GSERIALIZED *geom) {
	GSERIALIZED *result;
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);

	result = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);

	return result;
}

bool LWGEOM_isring(GSERIALIZED *geom) {
	// GEOSGeometry *g1;
	// int result;

	// geom = PG_GETARG_GSERIALIZED_P(0);

	// /* Empty things can't close */
	// if ( gserialized_is_empty(geom) )
	// 	PG_RETURN_BOOL(false);

	// initGEOS(lwpgnotice, lwgeom_geos_error);

	// g1 = POSTGIS2GEOS(geom);
	// if (!g1)
	// 	HANDLE_GEOS_ERROR("First argument geometry could not be converted to GEOS");

	// if ( GEOSGeomTypeId(g1) != GEOS_LINESTRING )
	// {
	// 	GEOSGeom_destroy(g1);
	// 	elog(ERROR, "ST_IsRing() should only be called on a linear feature");
	// }

	// result = GEOSisRing(g1);
	// GEOSGeom_destroy(g1);

	// if (result == 2) HANDLE_GEOS_ERROR("GEOSisRing");

	// PG_FREE_IF_COPY(geom, 0);
	// PG_RETURN_BOOL(result);
	return false;
}

GSERIALIZED *ST_Difference(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	GSERIALIZED *result;
	LWGEOM *lwgeom1, *lwgeom2, *lwresult;
	double prec = -1;

	lwgeom1 = lwgeom_from_gserialized(geom1);
	lwgeom2 = lwgeom_from_gserialized(geom2);

	lwresult = lwgeom_difference_prec(lwgeom1, lwgeom2, prec);
	if (!lwresult) {
		lwgeom_free(lwgeom1);
		lwgeom_free(lwgeom2);
		return nullptr;
	}
	result = geometry_serialize(lwresult);

	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	lwgeom_free(lwresult);

	return result;
}

GSERIALIZED *ST_Union(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	GSERIALIZED *result;
	LWGEOM *lwgeom1, *lwgeom2, *lwresult;
	double gridSize = -1;

	// if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
	// 	gridSize = PG_GETARG_FLOAT8(2);

	lwgeom1 = lwgeom_from_gserialized(geom1);
	lwgeom2 = lwgeom_from_gserialized(geom2);

	lwresult = lwgeom_union_prec(lwgeom1, lwgeom2, gridSize);
	result = geometry_serialize(lwresult);

	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	lwgeom_free(lwresult);

	return result;
}

} // namespace duckdb
