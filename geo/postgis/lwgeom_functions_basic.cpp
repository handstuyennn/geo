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
 * Copyright 2017-2018 Daniel Baston <dbaston@gmail.com>
 *
 **********************************************************************/

#include "postgis/lwgeom_functions_basic.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

#include <float.h>

namespace duckdb {

GSERIALIZED *LWGEOM_makepoint(double x, double y) {
	LWPOINT *point;
	GSERIALIZED *result;

	point = lwpoint_make2d(SRID_UNKNOWN, x, y);

	result = geometry_serialize((LWGEOM *)point);
	lwgeom_free((LWGEOM *)point);

	return result;
}

GSERIALIZED *LWGEOM_makepoint(double x, double y, double z) {
	LWPOINT *point;
	GSERIALIZED *result;

	point = lwpoint_make3dz(SRID_UNKNOWN, x, y, z);

	result = geometry_serialize((LWGEOM *)point);
	lwgeom_free((LWGEOM *)point);

	return result;
}

GSERIALIZED *LWGEOM_makeline(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	LWGEOM *lwgeoms[2];
	GSERIALIZED *result = NULL;
	LWLINE *outline;

	if ((gserialized_get_type(geom1) != POINTTYPE && gserialized_get_type(geom1) != LINETYPE) ||
	    (gserialized_get_type(geom2) != POINTTYPE && gserialized_get_type(geom2) != LINETYPE)) {
		// elog(ERROR, "Input geometries must be points or lines");
		return NULL;
	}

	gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

	lwgeoms[0] = lwgeom_from_gserialized(geom1);
	lwgeoms[1] = lwgeom_from_gserialized(geom2);

	outline = lwline_from_lwgeom_array(lwgeoms[0]->srid, 2, lwgeoms);

	result = geometry_serialize((LWGEOM *)outline);

	lwgeom_free(lwgeoms[0]);
	lwgeom_free(lwgeoms[1]);

	return result;
}

GSERIALIZED *LWGEOM_makeline_garray(GSERIALIZED *gserArray[], int nelems) {
	GSERIALIZED *result = NULL;
	LWGEOM **geoms;
	LWGEOM *outlwg;
	uint32 ngeoms;
	int32_t srid = SRID_UNKNOWN;

	/* Return null on 0-elements input array */
	if (nelems == 0)
		return nullptr;

	/* possibly more then required */
	geoms = (LWGEOM **)malloc(sizeof(LWGEOM *) * nelems);
	ngeoms = 0;

	for (size_t i = 0; i < (size_t)nelems; i++) {
		GSERIALIZED *geom = gserArray[i];

		if (!geom)
			continue;

		if (gserialized_get_type(geom) != POINTTYPE && gserialized_get_type(geom) != LINETYPE &&
		    gserialized_get_type(geom) != MULTIPOINTTYPE) {
			continue;
		}

		geoms[ngeoms++] = lwgeom_from_gserialized(geom);

		/* Check SRID homogeneity */
		if (ngeoms == 1) {
			/* Get first geometry SRID */
			srid = geoms[ngeoms - 1]->srid;
			/* TODO: also get ZMflags */
		} else
			gserialized_error_if_srid_mismatch_reference(geom, srid, __func__);
	}

	/* Return null on 0-points input array */
	if (ngeoms == 0) {
		/* TODO: should we return LINESTRING EMPTY here ? */
		return nullptr;
	}

	outlwg = (LWGEOM *)lwline_from_lwgeom_array(srid, ngeoms, geoms);

	result = geometry_serialize(outlwg);
	lwgeom_free(outlwg);

	return result;
}

GSERIALIZED *LWGEOM_makepoly(GSERIALIZED *pglwg1, GSERIALIZED *gserArray[], int nholes) {
	GSERIALIZED *result = NULL;
	const LWLINE *shell = NULL;
	const LWLINE **holes = NULL;
	LWPOLY *outpoly;
	uint32 i;

	/* Get input shell */
	if (gserialized_get_type(pglwg1) != LINETYPE) {
		// lwpgerror("Shell is not a line");
		return nullptr;
	}
	shell = lwgeom_as_lwline(lwgeom_from_gserialized(pglwg1));

	/* Get input holes if any */
	if (nholes > 0) {
		holes = (const LWLINE **)lwalloc(sizeof(LWLINE *) * nholes);
		for (i = 0; i < (size_t)nholes; i++) {
			LWLINE *hole;
			auto g = gserArray[i];
			if (gserialized_get_type(g) != LINETYPE) {
				// lwpgerror("Hole %d is not a line", i);
				return nullptr;
			}
			hole = lwgeom_as_lwline(lwgeom_from_gserialized(g));
			holes[i] = hole;
		}
	}

	outpoly = lwpoly_from_lwlines(shell, nholes, holes);
	result = geometry_serialize((LWGEOM *)outpoly);

	lwline_free((LWLINE *)shell);

	for (i = 0; i < (size_t)nholes; i++) {
		lwline_free((LWLINE *)holes[i]);
	}

	return result;
}

double ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	double mindist;
	LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
	LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
	gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

	mindist = lwgeom_mindistance2d(lwgeom1, lwgeom2);

	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);

	/* if called with empty geometries the ingoing mindistance is untouched, and makes us return NULL*/
	if (mindist < FLT_MAX)
		return mindist;

	PG_ERROR_NULL();
	return mindist;
}

lwvarlena_t *ST_GeoHash(GSERIALIZED *geom, size_t m_chars) {
	int precision = m_chars;
	lwvarlena_t *geohash = NULL;

	geohash = lwgeom_geohash((LWGEOM *)(lwgeom_from_gserialized(geom)), precision);
	if (geohash)
		return geohash;

	return nullptr;
}

bool ST_IsCollection(GSERIALIZED *geom) {
	int type = gserialized_get_type(geom);
	return lwtype_is_collection(type);
}

bool LWGEOM_isempty(GSERIALIZED *geom) {
	return gserialized_is_empty(geom);
}

/** number of points in an object */
int LWGEOM_npoints(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	int npoints = 0;

	npoints = lwgeom_count_vertices(lwgeom);
	lwgeom_free(lwgeom);

	return npoints;
}

/**
Returns the point in first input geometry that is closest to the second input geometry in 2d
*/
GSERIALIZED *LWGEOM_closestpoint(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	GSERIALIZED *result;
	LWGEOM *point;
	LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
	LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
	gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

	point = lwgeom_closest_point(lwgeom1, lwgeom2);

	if (lwgeom_is_empty(point))
		return nullptr;

	result = geometry_serialize(point);
	lwgeom_free(point);
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);

	return result;
}

/**
Returns boolean describing if
mininimum 2d distance between objects in
geom1 and geom2 is shorter than tolerance
*/
bool LWGEOM_dwithin(GSERIALIZED *geom1, GSERIALIZED *geom2, double tolerance) {
	double mindist;
	LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
	LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);

	if (tolerance < 0) {
		throw "Tolerance cannot be less than zero\n";
		return false;
	}

	gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

	mindist = lwgeom_mindistance2d_tolerance(lwgeom1, lwgeom2, tolerance);

	/*empty geometries cases should be right handled since return from underlying
	 functions should be FLT_MAX which causes false as answer*/
	return tolerance >= mindist;
}

/**
 * @brief Calculate the area of all the subobj in a polygon
 * 		area(point) = 0
 * 		area (line) = 0
 * 		area(polygon) = find its 2d area
 */
double ST_Area(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	double area = 0.0;

	area = lwgeom_area(lwgeom);

	lwgeom_free(lwgeom);

	return area;
}

/**
 * Compute the angle defined by 3 points or the angle between 2 vectors
 * defined by 4 points
 * given Point geometries.
 * @return NULL on exception (same point).
 * 		Return radians otherwise (always positive).
 */
double LWGEOM_angle(GSERIALIZED *geom1, GSERIALIZED *geom2, GSERIALIZED *geom3) {
	GSERIALIZED *seri_geoms[4];
	LWGEOM *geom_unser;
	LWPOINT *lwpoint;
	POINT2D points[4];
	double az1, az2;
	double result;
	int32_t srids[4];
	int i = 0;
	int j = 0;
	int err_code = 0;
	int n_args = 3;

	/* no deserialize, checking for common error first*/
	for (i = 0; i < n_args; i++) {
		if (i == 0) {
			seri_geoms[i] = geom1;
		} else if (i == 1) {
			seri_geoms[i] = geom2;
		} else if (i == 2) {
			seri_geoms[i] = geom3;
		}
		if (gserialized_is_empty(seri_geoms[i])) { /* empty geom */
			if (i == 3) {
				n_args = 3;
			} else {
				err_code = 1;
				break;
			}
		} else {
			if (gserialized_get_type(seri_geoms[i]) != POINTTYPE) { /* geom type */
				err_code = 2;
				break;
			} else {
				srids[i] = gserialized_get_srid(seri_geoms[i]);
				if (srids[0] != srids[i]) { /* error on srid*/
					err_code = 3;
					break;
				}
			}
		}
	}
	if (err_code > 0)
		switch (err_code) {
		/*FALLTHROUGH*/
		case 1:
			lwerror("Empty geometry");
			return 0.0;
			break;

		case 2:
			lwerror("Argument must be POINT geometries");
			return 0.0;
			break;

		case 3:
			lwerror("Operation on mixed SRID geometries");
			return 0.0;
			break;
		}
	/* extract points */
	for (i = 0; i < n_args; i++) {
		geom_unser = lwgeom_from_gserialized(seri_geoms[i]);
		lwpoint = lwgeom_as_lwpoint(geom_unser);
		if (!lwpoint) {
			lwerror("Error unserializing geometry");
			return 0.0;
		}

		if (!getPoint2d_p(lwpoint->point, 0, &points[i])) {
			/* // can't free serialized geom, it might be needed by lw
			for (j=0;j<n_args;j++)
			    PG_FREE_IF_COPY(seri_geoms[j], j); */
			lwerror("Error extracting point");
			return 0.0;
		}
		/* lwfree(geom_unser);don't do, lw may rely on this memory
		lwpoint_free(lwpoint); dont do , this memory is needed ! */
	}
	/* // can't free serialized geom, it might be needed by lw
	for (j=0;j<n_args;j++)
	    PG_FREE_IF_COPY(seri_geoms[j], j); */

	/* compute azimuth for the 2 pairs of points
	 * note that angle is not defined identically for 3 points or 4 points*/
	if (n_args == 3) { /* we rely on azimuth to complain if points are identical */
		if (!azimuth_pt_pt(&points[0], &points[1], &az1))
			return 0.0;
		if (!azimuth_pt_pt(&points[2], &points[1], &az2))
			return 0.0;
	} else {
		if (!azimuth_pt_pt(&points[0], &points[1], &az1))
			return 0.0;
		if (!azimuth_pt_pt(&points[2], &points[3], &az2))
			return 0.0;
	}
	result = az2 - az1;
	result += (result < 0) * 2 * M_PI; /* we dont want negative angle*/
	return result;
}

/**
 *  @brief find the "perimeter of a geometry"
 *  	perimeter(point) = 0
 *  	perimeter(line) = 0
 *  	perimeter(polygon) = sum of ring perimeters
 *  	uses euclidian 2d computation even if input is 3d
 */
double LWGEOM_perimeter2d_poly(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	double perimeter = 0.0;

	perimeter = lwgeom_perimeter_2d(lwgeom);
	return perimeter;
}

/**
 * Compute the azimuth of segment defined by the two
 * given Point geometries.
 * @return NULL on exception (same point).
 * 		Return radians otherwise.
 */
double LWGEOM_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	GSERIALIZED *geom;
	LWPOINT *lwpoint;
	POINT2D p1, p2;
	double result;
	int32_t srid;

	/* Extract first point */
	geom = geom1;
	lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom));
	if (!lwpoint) {
		throw "Argument must be POINT geometries";
		return 0.0;
	}
	srid = lwpoint->srid;
	if (!getPoint2d_p(lwpoint->point, 0, &p1)) {
		throw "Error extracting point";
		throw 0.0;
	}
	lwpoint_free(lwpoint);

	/* Extract second point */
	geom = geom2;
	lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(geom));
	if (!lwpoint) {
		throw "Argument must be POINT geometries";
		return 0.0;
	}
	if (lwpoint->srid != srid) {
		throw "Operation on mixed SRID geometries";
		return 0.0;
	}
	if (!getPoint2d_p(lwpoint->point, 0, &p2)) {
		throw "Error extracting point";
		return 0.0;
	}
	lwpoint_free(lwpoint);

	/* Standard return value for equality case */
	if ((p1.x == p2.x) && (p1.y == p2.y)) {
		return 0.0;
	}

	/* Compute azimuth */
	if (!azimuth_pt_pt(&p1, &p2, &result)) {
		return 0.0;
	}

	return result;
}

/**
 * @brief find the "length of a geometry"
 *  	length2d(point) = 0
 *  	length2d(line) = length of line
 *  	length2d(polygon) = 0  -- could make sense to return sum(ring perimeter)
 *  	uses euclidian 2d length (even if input is 3d)
 */
double LWGEOM_length2d_linestring(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	double dist = lwgeom_length_2d(lwgeom);
	lwgeom_free(lwgeom);
	return dist;
}

/**
 *  makes a polygon of the features bvol - 1st point = LL 3rd=UR
 *  2d only. (3d might be worth adding).
 *  create new geometry of type polygon, 1 ring, 5 points
 */
GSERIALIZED *LWGEOM_envelope(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	int32_t srid = lwgeom->srid;
	POINT4D pt;
	GBOX box;
	POINTARRAY *pa;
	GSERIALIZED *result;

	if (lwgeom_is_empty(lwgeom)) {
		/* must be the EMPTY geometry */
		return geom;
	}

	if (lwgeom_calculate_gbox(lwgeom, &box) == LW_FAILURE) {
		/* must be the EMPTY geometry */
		return geom;
	}

	/*
	 * Alter envelope type so that a valid geometry is always
	 * returned depending upon the size of the geometry. The
	 * code makes the following assumptions:
	 *     - If the bounding box is a single point then return a
	 *     POINT geometry
	 *     - If the bounding box represents either a horizontal or
	 *     vertical line, return a LINESTRING geometry
	 *     - Otherwise return a POLYGON
	 */

	if ((box.xmin == box.xmax) && (box.ymin == box.ymax)) {
		/* Construct and serialize point */
		LWPOINT *point = lwpoint_make2d(srid, box.xmin, box.ymin);
		result = geometry_serialize(lwpoint_as_lwgeom(point));
		lwpoint_free(point);
	} else if ((box.xmin == box.xmax) || (box.ymin == box.ymax)) {
		LWLINE *line;
		/* Construct point array */
		pa = ptarray_construct_empty(0, 0, 2);

		/* Assign coordinates to POINT2D array */
		pt.x = box.xmin;
		pt.y = box.ymin;
		ptarray_append_point(pa, &pt, LW_TRUE);
		pt.x = box.xmax;
		pt.y = box.ymax;
		ptarray_append_point(pa, &pt, LW_TRUE);

		/* Construct and serialize linestring */
		line = lwline_construct(srid, NULL, pa);
		result = geometry_serialize(lwline_as_lwgeom(line));
		lwline_free(line);
	} else {
		LWPOLY *poly;
		POINTARRAY **ppa = (POINTARRAY **)lwalloc(sizeof(POINTARRAY *));
		pa = ptarray_construct_empty(0, 0, 5);
		ppa[0] = pa;

		/* Assign coordinates to POINT2D array */
		pt.x = box.xmin;
		pt.y = box.ymin;
		ptarray_append_point(pa, &pt, LW_TRUE);
		pt.x = box.xmin;
		pt.y = box.ymax;
		ptarray_append_point(pa, &pt, LW_TRUE);
		pt.x = box.xmax;
		pt.y = box.ymax;
		ptarray_append_point(pa, &pt, LW_TRUE);
		pt.x = box.xmax;
		pt.y = box.ymin;
		ptarray_append_point(pa, &pt, LW_TRUE);
		pt.x = box.xmin;
		pt.y = box.ymin;
		ptarray_append_point(pa, &pt, LW_TRUE);

		/* Construct polygon  */
		poly = lwpoly_construct(srid, NULL, 1, ppa);
		result = geometry_serialize(lwpoly_as_lwgeom(poly));
		lwpoly_free(poly);
	}

	return result;
}

/**
 Maximum 2d distance between objects in geom1 and geom2.
 */
double LWGEOM_maxdistance2d_linestring(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	double maxdist;
	LWGEOM *lwgeom1 = lwgeom_from_gserialized(geom1);
	LWGEOM *lwgeom2 = lwgeom_from_gserialized(geom2);
	gserialized_error_if_srid_mismatch(geom1, geom2, __func__);

	maxdist = lwgeom_maxdistance2d(lwgeom1, lwgeom2);

	/*if called with empty geometries the ingoing mindistance is untouched, and makes us return NULL*/
	if (maxdist > -1)
		return maxdist;

	return 0.0;
}

} // namespace duckdb
