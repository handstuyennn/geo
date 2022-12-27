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
 * Copyright (C) 2017      Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2018      Daniel Baston <dbaston@gmail.com>
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

/*
 * Determines (recursively in the case of collections) whether the geometry
 * contains at least on arc geometry or segment.
 */
int lwgeom_has_arc(const LWGEOM *geom) {
	LWCOLLECTION *col;
	uint32_t i;

	switch (geom->type) {
	case POINTTYPE:
	case LINETYPE:
	case POLYGONTYPE:
	case TRIANGLETYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
		return LW_FALSE;
	case CIRCSTRINGTYPE:
	case CURVEPOLYTYPE:
	case COMPOUNDTYPE:
		return LW_TRUE;
	/* It's a collection that MAY contain an arc */
	default:
		col = (LWCOLLECTION *)geom;
		for (i = 0; i < col->ngeoms; i++) {
			if (lwgeom_has_arc(col->geoms[i]) == LW_TRUE)
				return LW_TRUE;
		}
		return LW_FALSE;
	}
}

/*******************************************************************************
 * Begin curve segmentize functions
 ******************************************************************************/

static double interpolate_arc(double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3) {
	/* Counter-clockwise sweep */
	if (a1 < a2) {
		if (angle <= a2)
			return zm1 + (zm2 - zm1) * (angle - a1) / (a2 - a1);
		else
			return zm2 + (zm3 - zm2) * (angle - a2) / (a3 - a2);
	}
	/* Clockwise sweep */
	else {
		if (angle >= a2)
			return zm1 + (zm2 - zm1) * (a1 - angle) / (a1 - a2);
		else
			return zm2 + (zm3 - zm2) * (a2 - angle) / (a2 - a3);
	}
}

/* Compute the angle covered by a single segment such that
 * a given number of segments per quadrant is achieved. */
static double angle_increment_using_segments_per_quad(double tol) {
	double increment;
	int perQuad = rint(tol);
	// error out if tol != perQuad ? (not-round)
	if (perQuad != tol) {
		lwerror("lwarc_linearize: segments per quadrant must be an integer value, got %.15g", tol, perQuad);
		return -1;
	}
	if (perQuad < 1) {
		lwerror("lwarc_linearize: segments per quadrant must be at least 1, got %d", perQuad);
		return -1;
	}
	increment = fabs(M_PI_2 / perQuad);

	return increment;
}

/* Compute the angle covered by a single quadrant such that
 * the segment deviates from the arc by no more than a given
 * amount. */
static double angle_increment_using_max_deviation(double max_deviation, double radius) {
	double increment, halfAngle, maxErr;
	if (max_deviation <= 0) {
		lwerror("lwarc_linearize: max deviation must be bigger than 0, got %.15g", max_deviation);
		return -1;
	}

	/*
	 * Ref: https://en.wikipedia.org/wiki/Sagitta_(geometry)
	 *
	 * An arc "sagitta" (distance between middle point of arc and
	 * middle point of corresponding chord) is defined as:
	 *
	 *   sagitta = radius * ( 1 - cos( angle ) );
	 *
	 * We want our sagitta to be at most "tolerance" long,
	 * and we want to find out angle, so we use the inverse
	 * formula:
	 *
	 *   tol = radius * ( 1 - cos( angle ) );
	 *   1 - cos( angle ) =  tol/radius
	 *   - cos( angle ) =  tol/radius - 1
	 *   cos( angle ) =  - tol/radius + 1
	 *   angle = acos( 1 - tol/radius )
	 *
	 * Constraints: 1.0 - tol/radius must be between -1 and 1
	 * which means tol must be between 0 and 2 times
	 * the radius, which makes sense as you cannot have a
	 * sagitta bigger than twice the radius!
	 *
	 */
	maxErr = max_deviation;
	if (maxErr > radius * 2) {
		maxErr = radius * 2;
	}
	do {
		halfAngle = acos(1.0 - maxErr / radius);
		/* TODO: avoid a loop here, going rather straight to
		 *       a minimum angle value */
		if (halfAngle != 0)
			break;
		maxErr *= 2;
	} while (1);
	increment = 2 * halfAngle;

	return increment;
}

/* Check that a given angle is positive and, if so, take
 * it to be the angle covered by a single segment. */
static double angle_increment_using_max_angle(double tol) {
	if (tol <= 0) {
		lwerror("lwarc_linearize: max angle must be bigger than 0, got %.15g", tol);
		return -1;
	}

	return tol;
}

/**
 * Segmentize an arc
 *
 * Does not add the final vertex
 *
 * @param to POINTARRAY to append segmentized vertices to
 * @param p1 first point defining the arc
 * @param p2 second point defining the arc
 * @param p3 third point defining the arc
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags LW_LINEARIZE_FLAGS
 *
 * @return number of points appended (0 if collinear),
 *         or -1 on error (lwerror would be called).
 *
 */
static int lwarc_linearize(POINTARRAY *to, const POINT4D *p1, const POINT4D *p2, const POINT4D *p3, double tol,
                           LW_LINEARIZE_TOLERANCE_TYPE tolerance_type, int flags) {
	POINT2D center;
	POINT2D *t1 = (POINT2D *)p1;
	POINT2D *t2 = (POINT2D *)p2;
	POINT2D *t3 = (POINT2D *)p3;
	POINT4D pt;
	int p2_side = 0;
	int clockwise = LW_TRUE;
	double radius;    /* Arc radius */
	double increment; /* Angle per segment */
	double angle_shift = 0;
	double a1, a2, a3;
	POINTARRAY *pa;
	int is_circle = LW_FALSE;
	int points_added = 0;
	int reverse = 0;
	int segments = 0;

	p2_side = lw_segment_side(t1, t3, t2);

	/* Force counterclockwise scan if SYMMETRIC operation is requested */
	if (p2_side == -1 && flags & LW_LINEARIZE_FLAG_SYMMETRIC) {
		/* swap p1-p3 */
		t1 = (POINT2D *)p3;
		t3 = (POINT2D *)p1;
		p1 = (POINT4D *)t1;
		p3 = (POINT4D *)t3;
		p2_side = 1;
		reverse = 1;
	}

	radius = lw_arc_center(t1, t2, t3, &center);

	/* Matched start/end points imply circle */
	if (p1->x == p3->x && p1->y == p3->y)
		is_circle = LW_TRUE;

	/* Negative radius signals straight line, p1/p2/p3 are collinear */
	if ((radius < 0.0 || p2_side == 0) && !is_circle)
		return 0;

	/* The side of the p1/p3 line that p2 falls on dictates the sweep
	   direction from p1 to p3. */
	if (p2_side == -1)
		clockwise = LW_TRUE;
	else
		clockwise = LW_FALSE;

	/* Compute the increment (angle per segment) depending on
	 * our tolerance type. */
	switch (tolerance_type) {
	case LW_LINEARIZE_TOLERANCE_TYPE_SEGS_PER_QUAD:
		increment = angle_increment_using_segments_per_quad(tol);
		break;
	case LW_LINEARIZE_TOLERANCE_TYPE_MAX_DEVIATION:
		increment = angle_increment_using_max_deviation(tol, radius);
		break;
	case LW_LINEARIZE_TOLERANCE_TYPE_MAX_ANGLE:
		increment = angle_increment_using_max_angle(tol);
		break;
	default:
		lwerror("lwarc_linearize: unsupported tolerance type %d", tolerance_type);
		return -1;
	}

	if (increment < 0) {
		/* Error occurred in increment calculation somewhere
		 * (lwerror already called)
		 */
		return -1;
	}

	/* Angles of each point that defines the arc section */
	a1 = atan2(p1->y - center.y, p1->x - center.x);
	a2 = atan2(p2->y - center.y, p2->x - center.x);
	a3 = atan2(p3->y - center.y, p3->x - center.x);

	/* Calculate total arc angle, in radians */
	double total_angle = clockwise ? a1 - a3 : a3 - a1;
	if (total_angle <= 0)
		total_angle += M_PI * 2;

	/* At extreme tolerance values (very low or very high, depending on
	 * the semantic) we may cause our arc to collapse. In this case,
	 * we want shrink the increment enough so that we get two segments
	 * for a standard arc, or three segments for a complete circle. */
	int min_segs = is_circle ? 3 : 2;
	segments = ceil(total_angle / increment);
	if (segments < min_segs) {
		segments = min_segs;
		increment = total_angle / min_segs;
	}

	if (flags & LW_LINEARIZE_FLAG_SYMMETRIC) {
		if (flags & LW_LINEARIZE_FLAG_RETAIN_ANGLE) {
			/* Number of complete steps */
			segments = trunc(total_angle / increment);

			/* Figure out the angle remainder, i.e. the amount of the angle
			 * that is left after we can take no more complete angle
			 * increments. */
			double angle_remainder = total_angle - (increment * segments);

			/* Shift the starting angle by half of the remainder. This
			 * will have the effect of evenly distributing the remainder
			 * among the first and last segments in the arc. */
			angle_shift = angle_remainder / 2.0;
		} else {
			/* Number of segments in output */
			segments = ceil(total_angle / increment);
			/* Tweak increment to be regular for all the arc */
			increment = total_angle / segments;
		}
	}

	/* p2 on left side => clockwise sweep */
	if (clockwise) {
		increment *= -1;
		angle_shift *= -1;
		/* Adjust a3 down so we can decrement from a1 to a3 cleanly */
		if (a3 > a1)
			a3 -= 2.0 * M_PI;
		if (a2 > a1)
			a2 -= 2.0 * M_PI;
	}
	/* p2 on right side => counter-clockwise sweep */
	else {
		/* Adjust a3 up so we can increment from a1 to a3 cleanly */
		if (a3 < a1)
			a3 += 2.0 * M_PI;
		if (a2 < a1)
			a2 += 2.0 * M_PI;
	}

	/* Override angles for circle case */
	if (is_circle) {
		increment = fabs(increment);
		segments = ceil(total_angle / increment);
		if (segments < 3) {
			segments = 3;
			increment = total_angle / 3;
		}
		a3 = a1 + 2.0 * M_PI;
		a2 = a1 + M_PI;
		clockwise = LW_FALSE;
		angle_shift = 0.0;
	}

	if (reverse) {
		/* Append points in order to a temporary POINTARRAY and
		 * reverse them before writing to the output POINTARRAY. */
		const int capacity = 8; /* TODO: compute exactly ? */
		pa = ptarray_construct_empty(ptarray_has_z(to), ptarray_has_m(to), capacity);
	} else {
		/* Append points directly to the output POINTARRAY,
		 * starting with p1. */
		pa = to;

		ptarray_append_point(pa, p1, LW_FALSE);
		++points_added;
	}

	/* Sweep from a1 to a3 */
	int seg_start = 1; /* First point is added manually */
	int seg_end = segments;
	if (angle_shift != 0.0) {
		/* When we have extra angles we need to add the extra segments at the
		 * start and end that cover those parts of the arc */
		seg_start = 0;
		seg_end = segments + 1;
	}
	for (int s = seg_start; s < seg_end; s++) {
		double angle = a1 + increment * s + angle_shift;
		pt.x = center.x + radius * cos(angle);
		pt.y = center.y + radius * sin(angle);
		pt.z = interpolate_arc(angle, a1, a2, a3, p1->z, p2->z, p3->z);
		pt.m = interpolate_arc(angle, a1, a2, a3, p1->m, p2->m, p3->m);
		ptarray_append_point(pa, &pt, LW_FALSE);
		++points_added;
	}

	/* Ensure the final point is EXACTLY the same as the first for the circular case */
	if (is_circle) {
		ptarray_remove_point(pa, pa->npoints - 1);
		ptarray_append_point(pa, p1, LW_FALSE);
	}

	if (reverse) {
		int i;
		ptarray_append_point(to, p3, LW_FALSE);
		for (i = pa->npoints; i > 0; i--) {
			getPoint4d_p(pa, i - 1, &pt);
			ptarray_append_point(to, &pt, LW_FALSE);
		}
		ptarray_free(pa);
	}

	return points_added;
}

/*
 * @param icurve input curve
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags see flags in lwarc_linearize
 *
 * @return a newly allocated LWLINE
 */
static LWLINE *lwcircstring_linearize(const LWCIRCSTRING *icurve, double tol,
                                      LW_LINEARIZE_TOLERANCE_TYPE tolerance_type, int flags) {
	LWLINE *oline;
	POINTARRAY *ptarray;
	uint32_t i, j;
	POINT4D p1, p2, p3, p4;
	int ret;

	ptarray = ptarray_construct_empty(FLAGS_GET_Z(icurve->points->flags), FLAGS_GET_M(icurve->points->flags), 64);

	for (i = 2; i < icurve->points->npoints; i += 2) {
		getPoint4d_p(icurve->points, i - 2, &p1);
		getPoint4d_p(icurve->points, i - 1, &p2);
		getPoint4d_p(icurve->points, i, &p3);

		ret = lwarc_linearize(ptarray, &p1, &p2, &p3, tol, tolerance_type, flags);
		if (ret > 0) {
			// LWDEBUGF(3, "lwcircstring_linearize: generated %d points", ptarray->npoints);
		} else if (ret == 0) {
			for (j = i - 2; j < i; j++) {
				getPoint4d_p(icurve->points, j, &p4);
				ptarray_append_point(ptarray, &p4, LW_TRUE);
			}
		} else {
			/* An error occurred, lwerror should have been called by now */
			ptarray_free(ptarray);
			return NULL;
		}
	}
	getPoint4d_p(icurve->points, icurve->points->npoints - 1, &p1);
	ptarray_append_point(ptarray, &p1, LW_FALSE);

	oline = lwline_construct(icurve->srid, NULL, ptarray);
	return oline;
}

/*
 * @param icompound input compound curve
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags see flags in lwarc_linearize
 *
 * @return a newly allocated LWLINE
 */
static LWLINE *lwcompound_linearize(const LWCOMPOUND *icompound, double tol, LW_LINEARIZE_TOLERANCE_TYPE tolerance_type,
                                    int flags) {
	LWGEOM *geom;
	POINTARRAY *ptarray = NULL;
	LWLINE *tmp = NULL;
	uint32_t i, j;
	POINT4D p;

	ptarray = ptarray_construct_empty(FLAGS_GET_Z(icompound->flags), FLAGS_GET_M(icompound->flags), 64);

	for (i = 0; i < icompound->ngeoms; i++) {
		geom = icompound->geoms[i];
		if (geom->type == CIRCSTRINGTYPE) {
			tmp = lwcircstring_linearize((LWCIRCSTRING *)geom, tol, tolerance_type, flags);
			for (j = 0; j < tmp->points->npoints; j++) {
				getPoint4d_p(tmp->points, j, &p);
				ptarray_append_point(ptarray, &p, LW_TRUE);
			}
			lwline_free(tmp);
		} else if (geom->type == LINETYPE) {
			tmp = (LWLINE *)geom;
			for (j = 0; j < tmp->points->npoints; j++) {
				getPoint4d_p(tmp->points, j, &p);
				ptarray_append_point(ptarray, &p, LW_TRUE);
			}
		} else {
			lwerror("%s: Unsupported geometry type: %s", __func__, lwtype_name(geom->type));
			return NULL;
		}
	}

	ptarray_remove_repeated_points_in_place(ptarray, 0.0, 2);
	return lwline_construct(icompound->srid, NULL, ptarray);
}

/*
 * @param icompound input curve polygon
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags see flags in lwarc_linearize
 *
 * @return a newly allocated LWPOLY
 */
static LWPOLY *lwcurvepoly_linearize(const LWCURVEPOLY *curvepoly, double tol,
                                     LW_LINEARIZE_TOLERANCE_TYPE tolerance_type, int flags) {
	LWPOLY *ogeom;
	LWGEOM *tmp;
	LWLINE *line;
	POINTARRAY **ptarray;
	uint32_t i;

	ptarray = (POINTARRAY **)lwalloc(sizeof(POINTARRAY *) * curvepoly->nrings);

	for (i = 0; i < curvepoly->nrings; i++) {
		tmp = curvepoly->rings[i];
		if (tmp->type == CIRCSTRINGTYPE) {
			line = lwcircstring_linearize((LWCIRCSTRING *)tmp, tol, tolerance_type, flags);
			ptarray[i] = ptarray_clone_deep(line->points);
			lwline_free(line);
		} else if (tmp->type == LINETYPE) {
			line = (LWLINE *)tmp;
			ptarray[i] = ptarray_clone_deep(line->points);
		} else if (tmp->type == COMPOUNDTYPE) {
			line = lwcompound_linearize((LWCOMPOUND *)tmp, tol, tolerance_type, flags);
			ptarray[i] = ptarray_clone_deep(line->points);
			lwline_free(line);
		} else {
			lwerror("Invalid ring type found in CurvePoly.");
			return NULL;
		}
	}

	ogeom = lwpoly_construct(curvepoly->srid, NULL, curvepoly->nrings, ptarray);
	return ogeom;
}

/**
 * @param mcurve input compound curve
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags see flags in lwarc_linearize
 *
 * @return a newly allocated LWMLINE
 */
static LWMLINE *lwmcurve_linearize(const LWMCURVE *mcurve, double tol, LW_LINEARIZE_TOLERANCE_TYPE type, int flags) {
	LWMLINE *ogeom;
	LWGEOM **lines;
	uint32_t i;

	lines = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * mcurve->ngeoms);

	for (i = 0; i < mcurve->ngeoms; i++) {
		const LWGEOM *tmp = mcurve->geoms[i];
		if (tmp->type == CIRCSTRINGTYPE) {
			lines[i] = (LWGEOM *)lwcircstring_linearize((LWCIRCSTRING *)tmp, tol, type, flags);
		} else if (tmp->type == LINETYPE) {
			lines[i] = (LWGEOM *)lwline_construct(mcurve->srid, NULL, ptarray_clone_deep(((LWLINE *)tmp)->points));
		} else if (tmp->type == COMPOUNDTYPE) {
			lines[i] = (LWGEOM *)lwcompound_linearize((LWCOMPOUND *)tmp, tol, type, flags);
		} else {
			lwerror("Unsupported geometry found in MultiCurve.");
			return NULL;
		}
	}

	ogeom = (LWMLINE *)lwcollection_construct(MULTILINETYPE, mcurve->srid, NULL, mcurve->ngeoms, lines);
	return ogeom;
}

/**
 * @param msurface input multi surface
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags see flags in lwarc_linearize
 *
 * @return a newly allocated LWMPOLY
 */
static LWMPOLY *lwmsurface_linearize(const LWMSURFACE *msurface, double tol, LW_LINEARIZE_TOLERANCE_TYPE type,
                                     int flags) {
	LWMPOLY *ogeom;
	LWGEOM *tmp;
	LWPOLY *poly;
	LWGEOM **polys;
	POINTARRAY **ptarray;
	uint32_t i, j;

	polys = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * msurface->ngeoms);

	for (i = 0; i < msurface->ngeoms; i++) {
		tmp = msurface->geoms[i];
		if (tmp->type == CURVEPOLYTYPE) {
			polys[i] = (LWGEOM *)lwcurvepoly_linearize((LWCURVEPOLY *)tmp, tol, type, flags);
		} else if (tmp->type == POLYGONTYPE) {
			poly = (LWPOLY *)tmp;
			ptarray = (POINTARRAY **)lwalloc(sizeof(POINTARRAY *) * poly->nrings);
			for (j = 0; j < poly->nrings; j++) {
				ptarray[j] = ptarray_clone_deep(poly->rings[j]);
			}
			polys[i] = (LWGEOM *)lwpoly_construct(msurface->srid, NULL, poly->nrings, ptarray);
		}
	}
	ogeom = (LWMPOLY *)lwcollection_construct(MULTIPOLYGONTYPE, msurface->srid, NULL, msurface->ngeoms, polys);
	return ogeom;
}

/**
 * @param collection input geometry collection
 * @param tol tolerance, semantic driven by tolerance_type
 * @param tolerance_type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags see flags in lwarc_linearize
 *
 * @return a newly allocated LWCOLLECTION
 */
static LWCOLLECTION *lwcollection_linearize(const LWCOLLECTION *collection, double tol,
                                            LW_LINEARIZE_TOLERANCE_TYPE type, int flags) {
	LWCOLLECTION *ocol;
	LWGEOM *tmp;
	LWGEOM **geoms;
	uint32_t i;

	geoms = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * collection->ngeoms);

	for (i = 0; i < collection->ngeoms; i++) {
		tmp = collection->geoms[i];
		switch (tmp->type) {
		case CIRCSTRINGTYPE:
			geoms[i] = (LWGEOM *)lwcircstring_linearize((LWCIRCSTRING *)tmp, tol, type, flags);
			break;
		case COMPOUNDTYPE:
			geoms[i] = (LWGEOM *)lwcompound_linearize((LWCOMPOUND *)tmp, tol, type, flags);
			break;
		case CURVEPOLYTYPE:
			geoms[i] = (LWGEOM *)lwcurvepoly_linearize((LWCURVEPOLY *)tmp, tol, type, flags);
			break;
		case MULTICURVETYPE:
		case MULTISURFACETYPE:
		case COLLECTIONTYPE:
			geoms[i] = (LWGEOM *)lwcollection_linearize((LWCOLLECTION *)tmp, tol, type, flags);
			break;
		default:
			geoms[i] = lwgeom_clone_deep(tmp);
			break;
		}
	}
	ocol = lwcollection_construct(COLLECTIONTYPE, collection->srid, NULL, collection->ngeoms, geoms);
	return ocol;
}

LWGEOM *lwcurve_linearize(const LWGEOM *geom, double tol, LW_LINEARIZE_TOLERANCE_TYPE type, int flags) {
	LWGEOM *ogeom = NULL;
	switch (geom->type) {
	case CIRCSTRINGTYPE:
		ogeom = (LWGEOM *)lwcircstring_linearize((LWCIRCSTRING *)geom, tol, type, flags);
		break;
	case COMPOUNDTYPE:
		ogeom = (LWGEOM *)lwcompound_linearize((LWCOMPOUND *)geom, tol, type, flags);
		break;
	case CURVEPOLYTYPE:
		ogeom = (LWGEOM *)lwcurvepoly_linearize((LWCURVEPOLY *)geom, tol, type, flags);
		break;
	case MULTICURVETYPE:
		ogeom = (LWGEOM *)lwmcurve_linearize((LWMCURVE *)geom, tol, type, flags);
		break;
	case MULTISURFACETYPE:
		ogeom = (LWGEOM *)lwmsurface_linearize((LWMSURFACE *)geom, tol, type, flags);
		break;
	case COLLECTIONTYPE:
		ogeom = (LWGEOM *)lwcollection_linearize((LWCOLLECTION *)geom, tol, type, flags);
		break;
	default:
		ogeom = lwgeom_clone_deep(geom);
	}
	return ogeom;
}

/* Kept for backward compatibility - TODO: drop */
LWGEOM *lwgeom_stroke(const LWGEOM *geom, uint32_t perQuad) {
	return lwcurve_linearize(geom, perQuad, LW_LINEARIZE_TOLERANCE_TYPE_SEGS_PER_QUAD, 0);
}

/* Kept for backward compatibility - TODO: drop */
LWPOLY *lwcurvepoly_stroke(const LWCURVEPOLY *curvepoly, uint32_t perQuad) {
	return lwcurvepoly_linearize(curvepoly, perQuad, LW_LINEARIZE_TOLERANCE_TYPE_SEGS_PER_QUAD, 0);
}

} // namespace duckdb
