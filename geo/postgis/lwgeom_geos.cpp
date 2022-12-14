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

#include "geos_c.hpp"
#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/lwgeom_geos.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

namespace duckdb {

GSERIALIZED *GEOS2POSTGIS(GEOSGeom geom, char want3d) {
	LWGEOM *lwgeom;
	GSERIALIZED *result;

	lwgeom = GEOS2LWGEOM(geom, want3d);
	if (!lwgeom) {
		lwerror("%s: GEOS2LWGEOM returned NULL", __func__);
		return NULL;
	}

	if (lwgeom_needs_bbox(lwgeom))
		lwgeom_add_bbox(lwgeom);

	result = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);

	return result;
}

/*-----=POSTGIS2GEOS= */

GEOSGeometry *POSTGIS2GEOS(const GSERIALIZED *pglwgeom) {
	GEOSGeometry *ret;
	LWGEOM *lwgeom = lwgeom_from_gserialized(pglwgeom);
	if (!lwgeom) {
		lwerror("POSTGIS2GEOS: unable to deserialize input");
		return NULL;
	}
	ret = LWGEOM2GEOS(lwgeom, 0);
	lwgeom_free(lwgeom);

	return ret;
}

GSERIALIZED *centroid(GSERIALIZED *geom) {
	GSERIALIZED *result;
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);

	result = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);

	return result;
}

bool LWGEOM_isring(GSERIALIZED *geom) {
	GEOSGeometry *g1;
	int result;

	/* Empty things can't close */
	if (gserialized_is_empty(geom))
		return false;

	initGEOS(lwnotice, lwgeom_geos_error);

	g1 = POSTGIS2GEOS(geom);
	if (!g1)
		throw "First argument geometry could not be converted to GEOS";

	if (GEOSGeomTypeId(g1) != GEOS_LINESTRING) {
		GEOSGeom_destroy(g1);
		throw "ST_IsRing() should only be called on a linear feature";
	}

	result = GEOSisRing(g1);
	GEOSGeom_destroy(g1);

	if (result == 2)
		throw "GEOSisRing";

	return result;
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

/**
 * @brief This is the final function for GeomUnion
 * 			aggregate. Will have as input an array of Geometries.
 * 			Will iteratively call GEOSUnion on the GEOS-converted
 * 			versions of them and return PGIS-converted version back.
 * 			Changing combination order *might* speed up performance.
 */
GSERIALIZED *pgis_union_geometry_array(GSERIALIZED *gserArray[], int nelems) {
	bool isnull;

	int is3d = LW_FALSE, gotsrid = LW_FALSE;
	int geoms_size = 0, curgeom = 0, count = 0;

	GSERIALIZED *gser_out = NULL;

	GEOSGeometry *g = NULL;
	GEOSGeometry *g_union = NULL;
	GEOSGeometry **geoms = NULL;

	int32_t srid = SRID_UNKNOWN;

	int empty_type = 0;

	/* Null array, null geometry (should be empty?) */
	if (!gserArray)
		return nullptr;

	/* Empty array? Null return */
	if (nelems == 0)
		return nullptr;

	/* One geom, good geom? Return it */
	if (nelems == 1) {
		return gserArray[0];
	}

	/* Ok, we really need GEOS now ;) */
	initGEOS(lwnotice, lwgeom_geos_error);

	/*
	** Collect the non-empty inputs and stuff them into a GEOS collection
	*/
	geoms_size = nelems;
	geoms = (GEOSGeometry **)lwalloc(sizeof(GEOSGeometry *) * geoms_size);

	for (size_t i = 0; i < (size_t)nelems; i++) {
		GSERIALIZED *gser_in = gserArray[i];

		/* Skip null array items */
		if (!gser_in)
			continue;

		/* Check for SRID mismatch in array elements */
		if (gotsrid)
			gserialized_error_if_srid_mismatch_reference(gser_in, srid, __func__);
		else {
			/* Initialize SRID/dimensions info */
			srid = gserialized_get_srid(gser_in);
			is3d = gserialized_has_z(gser_in);
			gotsrid = 1;
		}

		/* Don't include empties in the union */
		if (gserialized_is_empty(gser_in)) {
			int gser_type = gserialized_get_type(gser_in);
			if (gser_type > empty_type) {
				empty_type = gser_type;
			}
		} else {
			g = POSTGIS2GEOS(gser_in);

			/* Uh oh! Exception thrown at construction... */
			if (!g) {
				throw "One of the geometries in the set could not be converted to GEOS";
			}

			/* Ensure we have enough space in our storage array */
			if (curgeom == geoms_size) {
				geoms_size *= 2;
				geoms = (GEOSGeometry **)lwrealloc(geoms, sizeof(GEOSGeometry *) * geoms_size);
			}

			geoms[curgeom] = g;
			curgeom++;
		}
	}

	/*
	** Take our GEOS geometries and turn them into a GEOS collection,
	** then pass that into cascaded union.
	*/
	if (curgeom > 0) {
		g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
		if (!g)
			throw "Could not create GEOS COLLECTION from geometry array";

		g_union = GEOSUnaryUnion(g);
		GEOSGeom_destroy(g);
		if (!g_union)
			throw "GEOSUnaryUnion";

		GEOSSetSRID(g_union, srid);
		gser_out = GEOS2POSTGIS(g_union, is3d);
		GEOSGeom_destroy(g_union);
	}
	/* No real geometries in our array, any empties? */
	else {
		/* If it was only empties, we'll return the largest type number */
		if (empty_type > 0) {
			LWGEOM *lwgeom = lwgeom_construct_empty(empty_type, srid, is3d, 0);
			gser_out = geometry_serialize(lwgeom);
			lwgeom_free(lwgeom);
			return gser_out;
		}
		/* Nothing but NULL, returns NULL */
		else {
			return nullptr;
		}
	}

	if (!gser_out) {
		/* Union returned a NULL geometry */
		return nullptr;
	}

	return gser_out;
}

GSERIALIZED *ST_Intersection(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	GSERIALIZED *result;
	LWGEOM *lwgeom1, *lwgeom2, *lwresult;
	double prec = -1;

	lwgeom1 = lwgeom_from_gserialized(geom1);
	lwgeom2 = lwgeom_from_gserialized(geom2);

	lwresult = lwgeom_intersection_prec(lwgeom1, lwgeom2, prec);
	result = geometry_serialize(lwresult);

	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	lwgeom_free(lwresult);

	return result;
}

} // namespace duckdb
