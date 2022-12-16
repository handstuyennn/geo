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
	LWGEOM *lwgeom, *lwresult;

	lwgeom = lwgeom_from_gserialized(geom);
	lwresult = lwgeom_centroid(lwgeom);
	lwgeom_free(lwgeom);

	if (!lwresult)
		return nullptr;

	result = geometry_serialize(lwresult);
	lwgeom_free(lwresult);
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

GSERIALIZED *convexhull(GSERIALIZED *geom1) {
	GEOSGeometry *g1, *g3;
	GSERIALIZED *result;
	LWGEOM *lwout;
	int32_t srid;
	GBOX bbox;

	/* Empty.ConvexHull() == Empty */
	if (gserialized_is_empty(geom1))
		return geom1;

	srid = gserialized_get_srid(geom1);

	initGEOS(lwnotice, lwgeom_geos_error);

	g1 = POSTGIS2GEOS(geom1);

	if (!g1)
		throw "First argument geometry could not be converted to GEOS";

	g3 = GEOSConvexHull(g1);
	GEOSGeom_destroy(g1);

	if (!g3)
		throw "GEOSConvexHull";

	GEOSSetSRID(g3, srid);

	lwout = GEOS2LWGEOM(g3, gserialized_has_z(geom1));
	GEOSGeom_destroy(g3);

	if (!lwout) {
		throw "convexhull() failed to convert GEOS geometry to LWGEOM";
		return nullptr;
	}

	/* Copy input bbox if any */
	if (gserialized_get_gbox_p(geom1, &bbox)) {
		/* Force the box to have the same dimensionality as the lwgeom */
		bbox.flags = lwout->flags;
		lwout->bbox = gbox_copy(&bbox);
	}

	result = geometry_serialize(lwout);
	lwgeom_free(lwout);

	if (!result) {
		throw "GEOS convexhull() threw an error (result postgis geometry formation)!";
		return nullptr;
	}

	return result;
}

GSERIALIZED *buffer(GSERIALIZED *geom1, double size, string styles_text) {
	GEOSBufferParams *bufferparams;
	GEOSGeometry *g1, *g3 = NULL;
	GSERIALIZED *result;
	LWGEOM *lwg;
	int quadsegs = 8;   /* the default */
	int singleside = 0; /* the default */
	enum { ENDCAP_ROUND = 1, ENDCAP_FLAT = 2, ENDCAP_SQUARE = 3 };
	enum { JOIN_ROUND = 1, JOIN_MITRE = 2, JOIN_BEVEL = 3 };
	const double DEFAULT_MITRE_LIMIT = 5.0;
	const int DEFAULT_ENDCAP_STYLE = ENDCAP_ROUND;
	const int DEFAULT_JOIN_STYLE = JOIN_ROUND;
	double mitreLimit = DEFAULT_MITRE_LIMIT;
	int endCapStyle = DEFAULT_ENDCAP_STYLE;
	int joinStyle = DEFAULT_JOIN_STYLE;

	/* Empty.Buffer() == Empty[polygon] */
	if (gserialized_is_empty(geom1)) {
		lwg = lwpoly_as_lwgeom(
		    lwpoly_construct_empty(gserialized_get_srid(geom1), 0, 0)); // buffer wouldn't give back z or m anyway
		result = geometry_serialize(lwg);
		lwgeom_free(lwg);
		return result;
	}

	initGEOS(lwnotice, lwgeom_geos_error);

	g1 = POSTGIS2GEOS(geom1);
	if (!g1)
		throw "First argument geometry could not be converted to GEOS";

	char *param;
	int n = styles_text.size();
 
    // declaring character array
    char params[n + 1];

	strcpy(params, styles_text.c_str());

	for (param = params;; param = NULL) {
		char *key, *val;
		param = strtok(param, " ");
		if (!param)
			break;

		key = param;
		val = strchr(key, '=');
		if (!val || *(val + 1) == '\0') {
			lwerror("Missing value for buffer parameter %s", key);
			break;
		}
		*val = '\0';
		++val;

		if (!strcmp(key, "endcap")) {
			/* Supported end cap styles:
			 *   "round", "flat", "square"
			 */
			if (!strcmp(val, "round")) {
				endCapStyle = ENDCAP_ROUND;
			} else if (!strcmp(val, "flat") || !strcmp(val, "butt")) {
				endCapStyle = ENDCAP_FLAT;
			} else if (!strcmp(val, "square")) {
				endCapStyle = ENDCAP_SQUARE;
			} else {
				lwerror("Invalid buffer end cap "
				        "style: %s (accept: "
				        "'round', 'flat', 'butt' "
				        "or 'square'"
				        ")",
				        val);
				break;
			}

		} else if (!strcmp(key, "join")) {
			if (!strcmp(val, "round")) {
				joinStyle = JOIN_ROUND;
			} else if (!strcmp(val, "mitre") || !strcmp(val, "miter")) {
				joinStyle = JOIN_MITRE;
			} else if (!strcmp(val, "bevel")) {
				joinStyle = JOIN_BEVEL;
			} else {
				lwerror("Invalid buffer end cap "
				        "style: %s (accept: "
				        "'round', 'mitre', 'miter' "
				        " or 'bevel'"
				        ")",
				        val);
				break;
			}
		} else if (!strcmp(key, "mitre_limit") || !strcmp(key, "miter_limit")) {
			/* mitreLimit is a float */
			mitreLimit = atof(val);
		} else if (!strcmp(key, "quad_segs")) {
			/* quadrant segments is an int */
			quadsegs = atoi(val);
		} else if (!strcmp(key, "side")) {
			if (!strcmp(val, "both")) {
				singleside = 0;
			} else if (!strcmp(val, "left")) {
				singleside = 1;
			} else if (!strcmp(val, "right")) {
				singleside = 1;
				size *= -1;
			} else {
				lwerror("Invalid side parameter: %s (accept: 'right', 'left', 'both')", val);
				break;
			}
		} else {
			lwerror("Invalid buffer parameter: %s (accept: 'endcap', 'join', 'mitre_limit', 'miter_limit', "
			        "'quad_segs' and 'side')",
			        key);
			break;
		}
	}
	// lwfree(params); /* was pstrduped */

	bufferparams = GEOSBufferParams_create();
	if (bufferparams) {
		if (GEOSBufferParams_setEndCapStyle(bufferparams, endCapStyle) &&
		    GEOSBufferParams_setJoinStyle(bufferparams, joinStyle) &&
		    GEOSBufferParams_setMitreLimit(bufferparams, mitreLimit) &&
		    GEOSBufferParams_setQuadrantSegments(bufferparams, quadsegs) &&
		    GEOSBufferParams_setSingleSided(bufferparams, singleside)) {
			g3 = GEOSBufferWithParams(g1, bufferparams, size);
		} else {
			lwerror("Error setting buffer parameters.");
		}
		GEOSBufferParams_destroy(bufferparams);
	} else {
		lwerror("Error setting buffer parameters.");
	}

	GEOSGeom_destroy(g1);

	if (!g3)
		throw "GEOSBuffer";

	GEOSSetSRID(g3, gserialized_get_srid(geom1));

	result = GEOS2POSTGIS(g3, gserialized_has_z(geom1));
	GEOSGeom_destroy(g3);

	if (!result) {
		throw "GEOS buffer() threw an error (result postgis geometry formation)!";
		return nullptr;
	}

	return result;
}

} // namespace duckdb
