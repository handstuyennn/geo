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
 * Copyright (C) 2011-2012 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2011 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2007-2008 Mark Cave-Ayland
 * Copyright (C) 2001-2006 Refractions Research Inc.
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

#include <float.h>
#include <math.h>

namespace duckdb {

#ifndef _LIBLWGEOM_INTERNAL_H
#define _LIBLWGEOM_INTERNAL_H 1

/**
 * Macro that returns:
 * -1 if n < 0,
 *  1 if n > 0,
 *  0 if n == 0
 */
#define SIGNUM(n) (((n) > 0) - ((n) < 0))

/**
 * Tolerance used to determine equality.
 */
#define EPSILON_SQLMM 1e-8

/**
 * Floating point comparators.
 */
#define FP_TOLERANCE                1e-12
#define FP_IS_ZERO(A)               (fabs(A) <= FP_TOLERANCE)
#define FP_MAX(A, B)                (((A) > (B)) ? (A) : (B))
#define FP_MIN(A, B)                (((A) < (B)) ? (A) : (B))
#define FP_ABS(a)                   ((a) < (0) ? -(a) : (a))
#define FP_EQUALS(A, B)             (fabs((A) - (B)) <= FP_TOLERANCE)
#define FP_NEQUALS(A, B)            (fabs((A) - (B)) > FP_TOLERANCE)
#define FP_LT(A, B)                 (((A) + FP_TOLERANCE) < (B))
#define FP_LTEQ(A, B)               (((A)-FP_TOLERANCE) <= (B))
#define FP_GT(A, B)                 (((A)-FP_TOLERANCE) > (B))
#define FP_GTEQ(A, B)               (((A) + FP_TOLERANCE) >= (B))
#define FP_CONTAINS_TOP(A, X, B)    (FP_LT(A, X) && FP_LTEQ(X, B))
#define FP_CONTAINS_BOTTOM(A, X, B) (FP_LTEQ(A, X) && FP_LT(X, B))
#define FP_CONTAINS_INCL(A, X, B)   (FP_LTEQ(A, X) && FP_LTEQ(X, B))
#define FP_CONTAINS_EXCL(A, X, B)   (FP_LT(A, X) && FP_LT(X, B))
#define FP_CONTAINS(A, X, B)        FP_CONTAINS_EXCL(A, X, B)

/*
 * this will change to NaN when I figure out how to
 * get NaN in a platform-independent way
 */
#define NO_VALUE   0.0
#define NO_Z_VALUE NO_VALUE
#define NO_M_VALUE NO_VALUE

/**
 * Well-Known Text (WKT) Output Variant Types
 */
#define WKT_NO_TYPE   0x08 /* Internal use only */
#define WKT_NO_PARENS 0x10 /* Internal use only */
#define WKT_IS_CHILD  0x20 /* Internal use only */

/**
 * Well-Known Binary (WKB) Output Variant Types
 */

#define WKB_DOUBLE_SIZE 8 /* Internal use only */
#define WKB_INT_SIZE    4 /* Internal use only */
#define WKB_BYTE_SIZE   1 /* Internal use only */

/**
 * Well-Known Binary (WKB) Geometry Types
 */
#define WKB_POINT_TYPE              1
#define WKB_LINESTRING_TYPE         2
#define WKB_POLYGON_TYPE            3
#define WKB_MULTIPOINT_TYPE         4
#define WKB_MULTILINESTRING_TYPE    5
#define WKB_MULTIPOLYGON_TYPE       6
#define WKB_GEOMETRYCOLLECTION_TYPE 7
#define WKB_CIRCULARSTRING_TYPE     8
#define WKB_COMPOUNDCURVE_TYPE      9
#define WKB_CURVEPOLYGON_TYPE       10
#define WKB_MULTICURVE_TYPE         11
#define WKB_MULTISURFACE_TYPE       12
#define WKB_CURVE_TYPE              13 /* from ISO draft, not sure is real */
#define WKB_SURFACE_TYPE            14 /* from ISO draft, not sure is real */
#define WKB_POLYHEDRALSURFACE_TYPE  15
#define WKB_TIN_TYPE                16
#define WKB_TRIANGLE_TYPE           17

/*
 * Export functions
 */

/* Any (absolute) values outside this range will be printed in scientific notation */
#define OUT_MIN_DOUBLE             1E-8
#define OUT_MAX_DOUBLE             1E15
#define OUT_DEFAULT_DECIMAL_DIGITS 15

/* 17 digits are sufficient for round-tripping
 * Then we might add up to 8 (from OUT_MIN_DOUBLE) max leading zeroes (or 2 digits for "e+") */
#define OUT_MAX_DIGITS 17 + 8

/* Limit for the max amount of characters that a double can use, including dot and sign */
/* */
#define OUT_MAX_BYTES_DOUBLE   (1 /* Sign */ + 2 /* 0.x */ + OUT_MAX_DIGITS)
#define OUT_DOUBLE_BUFFER_SIZE OUT_MAX_BYTES_DOUBLE + 1 /* +1 including NULL */

/**
 * Constants for point-in-polygon return values
 */
#define LW_INSIDE   1
#define LW_BOUNDARY 0
#define LW_OUTSIDE  -1

/* Utilities */
int lwprint_double(double d, int maxdd, char *buf);

int p3d_same(const POINT3D *p1, const POINT3D *p2);
int p2d_same(const POINT2D *p1, const POINT2D *p2);

/*
 * What side of the line formed by p1 and p2 does q fall?
 * Returns -1 for left and 1 for right and 0 for co-linearity
 */
int lw_segment_side(const POINT2D *p1, const POINT2D *p2, const POINT2D *q);
int lw_arc_side(const POINT2D *A1, const POINT2D *A2, const POINT2D *A3, const POINT2D *Q);
int lw_arc_calculate_gbox_cartesian_2d(const POINT2D *A1, const POINT2D *A2, const POINT2D *A3, GBOX *gbox);
double lw_arc_center(const POINT2D *p1, const POINT2D *p2, const POINT2D *p3, POINT2D *result);
int lw_pt_in_seg(const POINT2D *P, const POINT2D *A1, const POINT2D *A2);
int lw_pt_in_arc(const POINT2D *P, const POINT2D *A1, const POINT2D *A2, const POINT2D *A3);
int ptarray_contains_point(const POINTARRAY *pa, const POINT2D *pt);
int ptarray_contains_point_partial(const POINTARRAY *pa, const POINT2D *pt, int check_closed, int *winding_number);
int ptarrayarc_contains_point(const POINTARRAY *pa, const POINT2D *pt);
int ptarrayarc_contains_point_partial(const POINTARRAY *pa, const POINT2D *pt, int check_closed, int *winding_number);
int lwgeom_contains_point(const LWGEOM *geom, const POINT2D *pt);
int lw_arc_is_pt(const POINT2D *A1, const POINT2D *A2, const POINT2D *A3);
int lwcompound_contains_point(const LWCOMPOUND *comp, const POINT2D *pt);
double lw_arc_length(const POINT2D *A1, const POINT2D *A2, const POINT2D *A3);

/*
 * Force dims
 */
LWGEOM *lwgeom_force_dims(const LWGEOM *lwgeom, int hasz, int hasm, double zval, double mval);
LWPOINT *lwpoint_force_dims(const LWPOINT *lwpoint, int hasz, int hasm, double zval, double mval);
LWLINE *lwline_force_dims(const LWLINE *lwline, int hasz, int hasm, double zval, double mval);
LWPOLY *lwpoly_force_dims(const LWPOLY *lwpoly, int hasz, int hasm, double zval, double mval);
LWCOLLECTION *lwcollection_force_dims(const LWCOLLECTION *lwcol, int hasz, int hasm, double zval, double mval);
POINTARRAY *ptarray_force_dims(const POINTARRAY *pa, int hasz, int hasm, double zval, double mval);

/*
 * Geohash
 */
int lwgeom_geohash_precision(GBOX bbox, GBOX *bounds);
lwvarlena_t *geohash_point(double longitude, double latitude, int precision);
void decode_geohash_bbox(char *geohash, double *lat, double *lon, int precision);

/*
 * Area calculations
 */
double lwpoly_area(const LWPOLY *poly);
double lwcurvepoly_area(const LWCURVEPOLY *curvepoly);
double lwtriangle_area(const LWTRIANGLE *triangle);

/*
 * Length calculations
 */
double lwcompound_length_2d(const LWCOMPOUND *comp);
double lwline_length_2d(const LWLINE *line);
double lwcircstring_length_2d(const LWCIRCSTRING *circ);
double lwpoly_perimeter_2d(const LWPOLY *poly);
double lwcurvepoly_perimeter_2d(const LWCURVEPOLY *poly);
double lwtriangle_perimeter_2d(const LWTRIANGLE *triangle);

/*
 * Segmentization
 */
LWPOLY *lwcurvepoly_stroke(const LWCURVEPOLY *curvepoly, uint32_t perQuad);

/*
 * PointArray
 */
int ptarray_has_z(const POINTARRAY *pa);
int ptarray_has_m(const POINTARRAY *pa);
double ptarray_signed_area(const POINTARRAY *pa);

/*
 * Length
 */
double ptarray_arc_length_2d(const POINTARRAY *pts);

/*
 * Clone support
 */
LWLINE *lwline_clone_deep(const LWLINE *lwgeom);
LWPOLY *lwpoly_clone_deep(const LWPOLY *lwgeom);
LWCOLLECTION *lwcollection_clone_deep(const LWCOLLECTION *lwgeom);
POINTARRAY *ptarray_clone(const POINTARRAY *ptarray);
GBOX *gbox_clone(const GBOX *gbox);

/*
 * Support for in place modification of point arrays, fast
 * function to move coordinate values around
 */
void ptarray_copy_point(POINTARRAY *pa, uint32_t from, uint32_t to);

/*
 * Startpoint
 */
int lwpoly_startpoint(const LWPOLY *lwpoly, POINT4D *pt);
int ptarray_startpoint(const POINTARRAY *pa, POINT4D *pt);
int lwcollection_startpoint(const LWCOLLECTION *col, POINT4D *pt);

void ptarray_remove_repeated_points_in_place(POINTARRAY *pa, double tolerance, uint32_t min_points);

/*
 * Closure test
 */
int lwline_is_closed(const LWLINE *line);
int lwpoly_is_closed(const LWPOLY *poly);
int lwcircstring_is_closed(const LWCIRCSTRING *curve);
int lwcompound_is_closed(const LWCOMPOUND *curve);
int lwpsurface_is_closed(const LWPSURFACE *psurface);
int lwtin_is_closed(const LWTIN *tin);

/**
 * Snap to grid
 */
void ptarray_grid_in_place(POINTARRAY *pa, const gridspec *grid);

/*
 * Number of vertices?
 */
uint32_t lwline_count_vertices(LWLINE *line);
uint32_t lwpoly_count_vertices(LWPOLY *poly);
uint32_t lwcollection_count_vertices(LWCOLLECTION *col);

/** Ensure the collection can hold at least up to ngeoms geometries */
void lwcollection_reserve(LWCOLLECTION *col, uint32_t ngeoms);

/** Check if subtype is allowed in collectiontype */
int lwcollection_allows_subtype(int collectiontype, int subtype);

/*
 * DP simplification
 */

/**
 * @param minpts minimum number of points to retain, if possible.
 */
void ptarray_simplify_in_place(POINTARRAY *pa, double tolerance, uint32_t minpts);

#endif /* !defined _LIBLWGEOM_INTERNAL_H  */

} // namespace duckdb
