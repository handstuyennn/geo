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
 * Copyright (C) 2009 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 **********************************************************************/

#include "postgis/geography_measurement.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwgeodetic_tree.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "libpgcommon/lwgeom_transform.hpp"
#include "postgis/geography_measurement_trees.hpp"

namespace duckdb {

#ifdef PROJ_GEODESIC
/* round to 10 nm precision */
#define INVMINDIST 1.0e8
#else
/* round to 100 nm precision */
#define INVMINDIST 1.0e7
#endif

/*
 ** geography_distance(GSERIALIZED *g1, GSERIALIZED *g2, double tolerance, boolean use_spheroid)
 ** returns double distance in meters
 */
double geography_distance(GSERIALIZED *g1, GSERIALIZED *g2, bool use_spheroid) {
	double distance;
	SPHEROID s;

	gserialized_error_if_srid_mismatch(g1, g2, __func__);

	/* Initialize spheroid */
	spheroid_init_from_srid(gserialized_get_srid(g1), &s);

	/* Set to sphere if requested */
	if (!use_spheroid)
		s.a = s.b = s.radius;

	/* Return NULL on empty arguments. */
	if (gserialized_is_empty(g1) || gserialized_is_empty(g2)) {
		PG_ERROR_NULL();
	}

	/* Do the brute force calculation if the cached calculation doesn't tick over */
	// if (LW_FAILURE == geography_distance_cache(fcinfo, shared_geom1, shared_geom2, &s, &distance))
	// {
	/* default to using tree-based distance calculation at all times */
	/* in standard distance call. */
	geography_tree_distance(g1, g2, &s, FP_TOLERANCE, &distance);
	/*
	LWGEOM* lwgeom1 = lwgeom_from_gserialized(g1);
	LWGEOM* lwgeom2 = lwgeom_from_gserialized(g2);
	distance = lwgeom_distance_spheroid(lwgeom1, lwgeom2, &s, tolerance);
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	*/
	// }

	/* Knock off any funny business at the nanometer level, ticket #2168 */
	distance = round(distance * INVMINDIST) / INVMINDIST;

	/* Something went wrong, negative return... should already be eloged, return NULL */
	if (distance < 0.0) {
		PG_ERROR_NULL();
	}

	return distance;
}

/*
** geography_area(GSERIALIZED *g)
** returns double area in meters square
*/
double geography_area(GSERIALIZED *g, bool use_spheroid) {
	LWGEOM *lwgeom = NULL;
	GBOX gbox;
	double area;
	SPHEROID s;

	/* Initialize spheroid */
	spheroid_init_from_srid(gserialized_get_srid(g), &s);

	lwgeom = lwgeom_from_gserialized(g);

	/* EMPTY things have no area */
	if (lwgeom_is_empty(lwgeom)) {
		lwgeom_free(lwgeom);
		return 0.0;
	}

	if (lwgeom->bbox)
		gbox = *(lwgeom->bbox);
	else
		lwgeom_calculate_gbox_geodetic(lwgeom, &gbox);

	/* User requests spherical calculation, turn our spheroid into a sphere */
	if (!use_spheroid)
		s.a = s.b = s.radius;

	/* Calculate the area */
	if (use_spheroid)
		area = lwgeom_area_spheroid(lwgeom, &s);
	else
		area = lwgeom_area_sphere(lwgeom, &s);

	/* Clean up */
	lwgeom_free(lwgeom);

	/* Something went wrong... */
	if (area < 0.0) {
		throw "lwgeom_area_spher(oid) returned area < 0.0";
		return 0;
	}

	return area;
}

/*
** geography_perimeter(GSERIALIZED *g)
** returns double perimeter in meters for area features
*/
double geography_perimeter(GSERIALIZED *g, bool use_spheroid) {
	LWGEOM *lwgeom = NULL;
	double length;
	SPHEROID s;
	int type;

	/* Only return for area features. */
	type = gserialized_get_type(g);
	if (!(type == POLYGONTYPE || type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE)) {
		return 0.0;
	}

	lwgeom = lwgeom_from_gserialized(g);

	/* EMPTY things have no perimeter */
	if (lwgeom_is_empty(lwgeom)) {
		lwgeom_free(lwgeom);
		return 0.0;
	}

	/* Initialize spheroid */
	spheroid_init_from_srid(gserialized_get_srid(g), &s);

	/* User requests spherical calculation, turn our spheroid into a sphere */
	if (!use_spheroid)
		s.a = s.b = s.radius;

	/* Calculate the length */
	length = lwgeom_length_spheroid(lwgeom, &s);

	/* Something went wrong... */
	if (length < 0.0) {
		throw "lwgeom_length_spheroid returned length < 0.0";
		return 0.0;
	}

	/* Clean up, but not all the way to the point arrays */
	lwgeom_free(lwgeom);

	return length;
}

/*
** geography_azimuth(GSERIALIZED *g1, GSERIALIZED *g2)
** returns direction between points (north = 0)
** azimuth (bearing) and distance
*/
double geography_azimuth(GSERIALIZED *g1, GSERIALIZED *g2) {
	LWGEOM *lwgeom1 = NULL;
	LWGEOM *lwgeom2 = NULL;
	double azimuth;
	SPHEROID s;
	uint32_t type1, type2;

	/* Only return for points. */
	type1 = gserialized_get_type(g1);
	type2 = gserialized_get_type(g2);
	if (type1 != POINTTYPE || type2 != POINTTYPE) {
		throw "ST_Azimuth(geography, geography) is only valid for point inputs";
		return 0.0;
	}

	lwgeom1 = lwgeom_from_gserialized(g1);
	lwgeom2 = lwgeom_from_gserialized(g2);

	/* EMPTY things cannot be used */
	if (lwgeom_is_empty(lwgeom1) || lwgeom_is_empty(lwgeom2)) {
		lwgeom_free(lwgeom1);
		lwgeom_free(lwgeom2);
		throw "ST_Azimuth(geography, geography) cannot work with empty points";
		return 0.0;
	}

	/* Initialize spheroid */
	spheroid_init_from_srid(gserialized_get_srid(g1), &s);

	/* Calculate the direction */
	azimuth = lwgeom_azumith_spheroid(lwgeom_as_lwpoint(lwgeom1), lwgeom_as_lwpoint(lwgeom2), &s);

	/* Clean up */
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);

	/* Return NULL for unknown (same point) azimuth */
	if (isnan(azimuth)) {
		return 0.0;
	}

	return azimuth;
}

/*
** geography_length(GSERIALIZED *g)
** returns double length in meters
*/
double geography_length(GSERIALIZED *g, bool use_spheroid) {
	LWGEOM *lwgeom = NULL;
	double length;
	SPHEROID s;

	/* Get our geometry object loaded into memory. */
	lwgeom = lwgeom_from_gserialized(g);

	/* EMPTY things have no length */
	if (lwgeom_is_empty(lwgeom) || lwgeom->type == POLYGONTYPE || lwgeom->type == MULTIPOLYGONTYPE) {
		lwgeom_free(lwgeom);
		return 0.0;
	}

	/* Initialize spheroid */
	spheroid_init_from_srid(gserialized_get_srid(g), &s);

	/* User requests spherical calculation, turn our spheroid into a sphere */
	if (!use_spheroid)
		s.a = s.b = s.radius;

	/* Calculate the length */
	length = lwgeom_length_spheroid(lwgeom, &s);

	/* Something went wrong... */
	if (length < 0.0) {
		throw "lwgeom_length_spheroid returned length < 0.0";
		return 0.0;
	}

	/* Clean up */
	lwgeom_free(lwgeom);

	return length;
}

} // namespace duckdb
