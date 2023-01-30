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
 * Copyright 2009 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

/**
 * Point in spherical coordinates on the world. Units of radians.
 */
typedef struct {
	double lon;
	double lat;
} GEOGRAPHIC_POINT;

/**
 * Two-point great circle segment from a to b.
 */
typedef struct {
	GEOGRAPHIC_POINT start;
	GEOGRAPHIC_POINT end;
} GEOGRAPHIC_EDGE;

/**
 * Conversion functions
 */
#define deg2rad(d) (M_PI * (d) / 180.0)
#define rad2deg(r) (180.0 * (r) / M_PI)

/**
 * Bitmask elements for edge_intersects() return value.
 */
#define PIR_NO_INTERACT   0x00
#define PIR_INTERSECTS    0x01
#define PIR_COLINEAR      0x02
#define PIR_A_TOUCH_RIGHT 0x04
#define PIR_A_TOUCH_LEFT  0x08
#define PIR_B_TOUCH_RIGHT 0x10
#define PIR_B_TOUCH_LEFT  0x20

#define POW2(x) ((x) * (x))

/*
 * Geodetic calculations
 */
void geog2cart(const GEOGRAPHIC_POINT *g, POINT3D *p);
void cart2geog(const POINT3D *p, GEOGRAPHIC_POINT *g);
void robust_cross_product(const GEOGRAPHIC_POINT *p, const GEOGRAPHIC_POINT *q, POINT3D *a);
int edge_contains_point(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p);
int edge_calculate_gbox(const POINT3D *A1, const POINT3D *A2, GBOX *gbox);
int edge_point_on_plane(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p);
int edge_point_in_cone(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p);
void vector_sum(const POINT3D *a, const POINT3D *b, POINT3D *n);
void normalize(POINT3D *p);
void unit_normal(const POINT3D *P1, const POINT3D *P2, POINT3D *normal);
void vector_scale(POINT3D *a, double s);
double sphere_direction(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e, double d);
void ll2cart(const POINT2D *g, POINT3D *p);
void geographic_point_init(double lon, double lat, GEOGRAPHIC_POINT *g);
double longitude_radians_normalize(double lon);
double latitude_radians_normalize(double lat);
double sphere_distance(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e);
double sphere_distance_cartesian(const POINT3D *s, const POINT3D *e);
int sphere_project(const GEOGRAPHIC_POINT *r, double distance, double azimuth, GEOGRAPHIC_POINT *n);
double edge_distance_to_point(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *gp, GEOGRAPHIC_POINT *closest);
double edge_distance_to_edge(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *closest1,
                             GEOGRAPHIC_POINT *closest2);
double edge_maxdistance_to_point(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *gp, GEOGRAPHIC_POINT *farest);
double edge_maxdistance_to_edge(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *farest1,
                                GEOGRAPHIC_POINT *farest2);
int crosses_dateline(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e);
void point_shift(GEOGRAPHIC_POINT *p, double shift);
int geographic_point_equals(const GEOGRAPHIC_POINT *g1, const GEOGRAPHIC_POINT *g2);
uint32_t edge_intersects(const POINT3D *A1, const POINT3D *A2, const POINT3D *B1, const POINT3D *B2);
int edge_intersection(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *g);
int lwpoly_pt_outside(const LWPOLY *poly, POINT2D *pt_outside);
double ptarray_area_sphere(const POINTARRAY *pa);
double ptarray_length_spheroid(const POINTARRAY *pa, const SPHEROID *s);
int ptarray_contains_point_sphere(const POINTARRAY *pa, const POINT2D *pt_outside, const POINT2D *pt_to_test);
int lwpoly_covers_point2d(const LWPOLY *poly, const POINT2D *pt_to_test);

/*
** Prototypes for spheroid functions.
*/
double spheroid_distance(const GEOGRAPHIC_POINT *a, const GEOGRAPHIC_POINT *b, const SPHEROID *spheroid);
double spheroid_direction(const GEOGRAPHIC_POINT *r, const GEOGRAPHIC_POINT *s, const SPHEROID *spheroid);
int spheroid_project(const GEOGRAPHIC_POINT *r, const SPHEROID *spheroid, double distance, double azimuth,
                     GEOGRAPHIC_POINT *g);

} // namespace duckdb
