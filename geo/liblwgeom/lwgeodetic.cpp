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
 * Copyright 2009 David Skea <David.Skea@gov.bc.ca>
 *
 **********************************************************************/

#include "liblwgeom/lwgeodetic.hpp"

#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <cassert>
#include <cstring>
#include <math.h>

namespace duckdb {

/**
 * Utility function for ptarray_contains_point_sphere()
 */
static int point3d_equals(const POINT3D *p1, const POINT3D *p2) {
	return FP_EQUALS(p1->x, p2->x) && FP_EQUALS(p1->y, p2->y) && FP_EQUALS(p1->z, p2->z);
}

/**
 * Calculate the dot product of two unit vectors
 * (-1 == opposite, 0 == orthogonal, 1 == identical)
 */
static double dot_product(const POINT3D *p1, const POINT3D *p2) {
	return (p1->x * p2->x) + (p1->y * p2->y) + (p1->z * p2->z);
}

/**
 * Utility function for edge_intersects(), signum with a tolerance
 * in determining if the value is zero.
 */
static int dot_product_side(const POINT3D *p, const POINT3D *q) {
	double dp = dot_product(p, q);

	if (FP_IS_ZERO(dp))
		return 0;

	return dp < 0.0 ? -1 : 1;
}

/**
 * Calculate the cross product of two vectors
 */
static void cross_product(const POINT3D *a, const POINT3D *b, POINT3D *n) {
	n->x = a->y * b->z - a->z * b->y;
	n->y = a->z * b->x - a->x * b->z;
	n->z = a->x * b->y - a->y * b->x;
	return;
}

/**
 * Calculate the difference of two vectors
 */
static void vector_difference(const POINT3D *a, const POINT3D *b, POINT3D *n) {
	n->x = a->x - b->x;
	n->y = a->y - b->y;
	n->z = a->z - b->z;
	return;
}

/**
 * Calculate the sum of two vectors
 */
void vector_sum(const POINT3D *a, const POINT3D *b, POINT3D *n) {
	n->x = a->x + b->x;
	n->y = a->y + b->y;
	n->z = a->z + b->z;
	return;
}

/**
 * Normalize to a unit vector.
 */
void normalize(POINT3D *p) {
	double d = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
	if (FP_IS_ZERO(d)) {
		p->x = p->y = p->z = 0.0;
		return;
	}
	p->x = p->x / d;
	p->y = p->y / d;
	p->z = p->z / d;
	return;
}

/**
 * Normalize to a unit vector.
 */
static void normalize2d(POINT2D *p) {
	double d = sqrt(p->x * p->x + p->y * p->y);
	if (FP_IS_ZERO(d)) {
		p->x = p->y = 0.0;
		return;
	}
	p->x = p->x / d;
	p->y = p->y / d;
	return;
}

/**
 * Check to see if this geocentric gbox is wrapped around a pole.
 * Only makes sense if this gbox originated from a polygon, as it's assuming
 * the box is generated from external edges and there's an "interior" which
 * contains the pole.
 *
 * This function is overdetermined, for very large polygons it might add an
 * unwarranted pole. STILL NEEDS WORK!
 */
static int gbox_check_poles(GBOX *gbox) {
	int rv = LW_FALSE;
#if POSTGIS_DEBUG_LEVEL >= 4
	char *gbox_str = gbox_to_string(gbox);
	lwfree(gbox_str);
#endif
	/* Z axis */
	if (gbox->xmin < 0.0 && gbox->xmax > 0.0 && gbox->ymin < 0.0 && gbox->ymax > 0.0) {
		/* Extrema lean positive */
		if ((gbox->zmin > 0.0) && (gbox->zmax > 0.0)) {
			gbox->zmax = 1.0;
		}
		/* Extrema lean negative */
		else if ((gbox->zmin < 0.0) && (gbox->zmax < 0.0)) {
			gbox->zmin = -1.0;
		}
		/* Extrema both sides! */
		else {
			gbox->zmin = -1.0;
			gbox->zmax = 1.0;
		}
		rv = LW_TRUE;
	}

	/* Y axis */
	if (gbox->xmin < 0.0 && gbox->xmax > 0.0 && gbox->zmin < 0.0 && gbox->zmax > 0.0) {
		if ((gbox->ymin > 0.0) && (gbox->ymax > 0.0)) {
			gbox->ymax = 1.0;
		} else if ((gbox->ymin < 0.0) && (gbox->ymax < 0.0)) {
			gbox->ymin = -1.0;
		} else {
			gbox->ymax = 1.0;
			gbox->ymin = -1.0;
		}
		rv = LW_TRUE;
	}

	/* X axis */
	if (gbox->ymin < 0.0 && gbox->ymax > 0.0 && gbox->zmin < 0.0 && gbox->zmax > 0.0) {
		if ((gbox->xmin > 0.0) && (gbox->xmax > 0.0)) {
			gbox->xmax = 1.0;
		} else if ((gbox->xmin < 0.0) && (gbox->xmax < 0.0)) {
			gbox->xmin = -1.0;
		} else {
			gbox->xmax = 1.0;
			gbox->xmin = -1.0;
		}

		rv = LW_TRUE;
	}

	return rv;
}

/**
 * Convert spherical coordinates to cartesian coordinates on unit sphere
 */
void geog2cart(const GEOGRAPHIC_POINT *g, POINT3D *p) {
	p->x = cos(g->lat) * cos(g->lon);
	p->y = cos(g->lat) * sin(g->lon);
	p->z = sin(g->lat);
}

/**
 * Convert cartesian coordinates on unit sphere to spherical coordinates
 */
void cart2geog(const POINT3D *p, GEOGRAPHIC_POINT *g) {
	g->lon = atan2(p->y, p->x);
	g->lat = asin(p->z);
}

/**
 * Computes the cross product of two vectors using their lat, lng representations.
 * Good even for small distances between p and q.
 */
void robust_cross_product(const GEOGRAPHIC_POINT *p, const GEOGRAPHIC_POINT *q, POINT3D *a) {
	double lon_qpp = (q->lon + p->lon) / -2.0;
	double lon_qmp = (q->lon - p->lon) / 2.0;
	double sin_p_lat_minus_q_lat = sin(p->lat - q->lat);
	double sin_p_lat_plus_q_lat = sin(p->lat + q->lat);
	double sin_lon_qpp = sin(lon_qpp);
	double sin_lon_qmp = sin(lon_qmp);
	double cos_lon_qpp = cos(lon_qpp);
	double cos_lon_qmp = cos(lon_qmp);
	a->x = sin_p_lat_minus_q_lat * sin_lon_qpp * cos_lon_qmp - sin_p_lat_plus_q_lat * cos_lon_qpp * sin_lon_qmp;
	a->y = sin_p_lat_minus_q_lat * cos_lon_qpp * cos_lon_qmp + sin_p_lat_plus_q_lat * sin_lon_qpp * sin_lon_qmp;
	a->z = cos(p->lat) * cos(q->lat) * sin(q->lon - p->lon);
}

int crosses_dateline(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e) {
	double sign_s = SIGNUM(s->lon);
	double sign_e = SIGNUM(e->lon);
	double ss = fabs(s->lon);
	double ee = fabs(e->lon);
	if (sign_s == sign_e) {
		return LW_FALSE;
	} else {
		double dl = ss + ee;
		if (dl < M_PI)
			return LW_FALSE;
		else if (FP_EQUALS(dl, M_PI))
			return LW_FALSE;
		else
			return LW_TRUE;
	}
}

/**
 * Shift a point around by a number of radians
 */
void point_shift(GEOGRAPHIC_POINT *p, double shift) {
	double lon = p->lon + shift;
	if (lon > M_PI)
		p->lon = -1.0 * M_PI + (lon - M_PI);
	else
		p->lon = lon;
	return;
}

/**
 * Given two points on a unit sphere, calculate the direction from s to e.
 */
double sphere_direction(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e, double d) {
	double heading = 0.0;
	double f;

	/* Starting from the poles? Special case. */
	if (FP_IS_ZERO(cos(s->lat)))
		return (s->lat > 0.0) ? M_PI : 0.0;

	f = (sin(e->lat) - sin(s->lat) * cos(d)) / (sin(d) * cos(s->lat));
	if (FP_EQUALS(f, 1.0))
		heading = 0.0;
	else if (FP_EQUALS(f, -1.0))
		heading = M_PI;
	else if (fabs(f) > 1.0) {
		heading = acos(f);
	} else
		heading = acos(f);

	if (sin(e->lon - s->lon) < 0.0)
		heading = -1 * heading;

	return heading;
}

/**
 * Calculates the unit normal to two vectors, trying to avoid
 * problems with over-narrow or over-wide cases.
 */
void unit_normal(const POINT3D *P1, const POINT3D *P2, POINT3D *normal) {
	double p_dot = dot_product(P1, P2);
	POINT3D P3;

	/* If edge is really large, calculate a narrower equivalent angle A1/A3. */
	if (p_dot < 0) {
		vector_sum(P1, P2, &P3);
		normalize(&P3);
	}
	/* If edge is narrow, calculate a wider equivalent angle A1/A3. */
	else if (p_dot > 0.95) {
		vector_difference(P2, P1, &P3);
		normalize(&P3);
	}
	/* Just keep the current angle in A1/A3. */
	else {
		P3 = *P2;
	}

	/* Normals to the A-plane and B-plane */
	cross_product(P1, &P3, normal);
	normalize(normal);
}

/**
 * Returns -1 if the point is to the left of the plane formed
 * by the edge, 1 if the point is to the right, and 0 if the
 * point is on the plane.
 */
static int edge_point_side(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p) {
	POINT3D normal, pt;
	double w;
	/* Normal to the plane defined by e */
	robust_cross_product(&(e->start), &(e->end), &normal);
	normalize(&normal);
	geog2cart(p, &pt);
	/* We expect the dot product of with normal with any vector in the plane to be zero */
	w = dot_product(&normal, &pt);
	if (FP_IS_ZERO(w)) {
		return 0;
	}

	if (w < 0)
		return -1;
	else
		return 1;
}

/**
 * Utility function for checking if P is within the cone defined by A1/A2.
 */
static int point_in_cone(const POINT3D *A1, const POINT3D *A2, const POINT3D *P) {
	POINT3D AC; /* Center point of A1/A2 */
	double min_similarity, similarity;

	/* Boundary case */
	if (point3d_equals(A1, P) || point3d_equals(A2, P))
		return LW_TRUE;

	/* The normalized sum bisects the angle between start and end. */
	vector_sum(A1, A2, &AC);
	normalize(&AC);

	/* The projection of start onto the center defines the minimum similarity */
	min_similarity = dot_product(A1, &AC);

	/* If the edge is sufficiently curved, use the dot product test */
	if (fabs(1.0 - min_similarity) > 1e-10) {
		/* The projection of candidate p onto the center */
		similarity = dot_product(P, &AC);

		/* If the projection of the candidate is larger than */
		/* the projection of the start point, the candidate */
		/* must be closer to the center than the start, so */
		/* therefor inside the cone */
		if (similarity > min_similarity) {
			return LW_TRUE;
		} else {
			return LW_FALSE;
		}
	} else {
		/* Where the edge is very narrow, the dot product test */
		/* fails, but we can use the almost-planar nature of the */
		/* problem space then to test if the vector from the */
		/* candidate to the start point in a different direction */
		/* to the vector from candidate to end point */
		/* If so, then candidate is between start and end */
		POINT3D PA1, PA2;
		vector_difference(P, A1, &PA1);
		vector_difference(P, A2, &PA2);
		normalize(&PA1);
		normalize(&PA2);
		if (dot_product(&PA1, &PA2) < 0.0) {
			return LW_TRUE;
		} else {
			return LW_FALSE;
		}
	}
	return LW_FALSE;
}

/**
 * Returns true if the point p is on the great circle plane.
 * Forms the scalar triple product of A,B,p and if the volume of the
 * resulting parallelepiped is near zero the point p is on the
 * great circle plane.
 */
int edge_point_on_plane(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p) {
	int side = edge_point_side(e, p);
	if (side == 0)
		return LW_TRUE;

	return LW_FALSE;
}

/**
 * Returns true if the point p is inside the cone defined by the
 * two ends of the edge e.
 */
int edge_point_in_cone(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p) {
	POINT3D vcp, vs, ve, vp;
	double vs_dot_vcp, vp_dot_vcp;
	geog2cart(&(e->start), &vs);
	geog2cart(&(e->end), &ve);
	/* Antipodal case, everything is inside. */
	if (vs.x == -1.0 * ve.x && vs.y == -1.0 * ve.y && vs.z == -1.0 * ve.z)
		return LW_TRUE;
	geog2cart(p, &vp);
	/* The normalized sum bisects the angle between start and end. */
	vector_sum(&vs, &ve, &vcp);
	normalize(&vcp);
	/* The projection of start onto the center defines the minimum similarity */
	vs_dot_vcp = dot_product(&vs, &vcp);
	/* The projection of candidate p onto the center */
	vp_dot_vcp = dot_product(&vp, &vcp);

	/*
	** We want to test that vp_dot_vcp is >= vs_dot_vcp but there are
	** numerical stability issues for values that are very very nearly
	** equal. Unfortunately there are also values of vp_dot_vcp that are legitimately
	** very close to but still less than vs_dot_vcp which we also need to catch.
	** The tolerance of 10-17 seems to do the trick on 32-bit and 64-bit architectures,
	** for the test cases here.
	** However, tuning the tolerance value feels like a dangerous hack.
	** Fundamentally, the problem is that this test is so sensitive.
	*/

	/* 1.1102230246251565404236316680908203125e-16 */

	if (vp_dot_vcp > vs_dot_vcp || fabs(vp_dot_vcp - vs_dot_vcp) < 2e-16) {
		return LW_TRUE;
	}
	return LW_FALSE;
}

/**
 * Scale a vector out by a factor
 */
void vector_scale(POINT3D *n, double scale) {
	n->x *= scale;
	n->y *= scale;
	n->z *= scale;
	return;
}

/**
 * Returns true if the point p is on the minor edge defined by the
 * end points of e.
 */
int edge_contains_point(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *p) {
	if (edge_point_in_cone(e, p) && edge_point_on_plane(e, p))
	/*	if ( edge_contains_coplanar_point(e, p) && edge_point_on_plane(e, p) ) */
	{
		return LW_TRUE;
	}
	return LW_FALSE;
}

/**
 * The magic function, given an edge in spherical coordinates, calculate a
 * 3D bounding box that fully contains it, taking into account the curvature
 * of the sphere on which it is inscribed.
 *
 * Any arc on the sphere defines a plane that bisects the sphere. In this plane,
 * the arc is a portion of a unit circle.
 * Projecting the end points of the axes (1,0,0), (-1,0,0) etc, into the plane
 * and normalizing yields potential extrema points. Those points on the
 * side of the plane-dividing line formed by the end points that is opposite
 * the origin of the plane are extrema and should be added to the bounding box.
 */
int edge_calculate_gbox(const POINT3D *A1, const POINT3D *A2, GBOX *gbox) {
	POINT2D R1, R2, RX, O;
	POINT3D AN, A3;
	POINT3D X[6];
	int i, o_side;

	/* Initialize the box with the edge end points */
	gbox_init_point3d(A1, gbox);
	gbox_merge_point3d(A2, gbox);

	/* Zero length edge, just return! */
	if (p3d_same(A1, A2))
		return LW_SUCCESS;

	/* Error out on antipodal edge */
	if (FP_EQUALS(A1->x, -1 * A2->x) && FP_EQUALS(A1->y, -1 * A2->y) && FP_EQUALS(A1->z, -1 * A2->z)) {
		lwerror("Antipodal (180 degrees long) edge detected!");
		return LW_FAILURE;
	}

	/* Create A3, a vector in the plane of A1/A2, orthogonal to A1  */
	unit_normal(A1, A2, &AN);
	unit_normal(&AN, A1, &A3);

	/* Project A1 and A2 into the 2-space formed by the plane A1/A3 */
	R1.x = 1.0;
	R1.y = 0.0;
	R2.x = dot_product(A2, A1);
	R2.y = dot_product(A2, &A3);

	/* Initialize our 3-space axis points (x+, x-, y+, y-, z+, z-) */
	memset(X, 0, sizeof(POINT3D) * 6);
	X[0].x = X[2].y = X[4].z = 1.0;
	X[1].x = X[3].y = X[5].z = -1.0;

	/* Initialize a 2-space origin point. */
	O.x = O.y = 0.0;
	/* What side of the line joining R1/R2 is O? */
	o_side = lw_segment_side(&R1, &R2, &O);

	/* Add any extrema! */
	for (i = 0; i < 6; i++) {
		/* Convert 3-space axis points to 2-space unit vectors */
		RX.x = dot_product(&(X[i]), A1);
		RX.y = dot_product(&(X[i]), &A3);
		normalize2d(&RX);

		/* Any axis end on the side of R1/R2 opposite the origin */
		/* is an extreme point in the arc, so we add the 3-space */
		/* version of the point on R1/R2 to the gbox */
		if (lw_segment_side(&R1, &R2, &RX) != o_side) {
			POINT3D Xn;
			Xn.x = RX.x * A1->x + RX.y * A3.x;
			Xn.y = RX.x * A1->y + RX.y * A3.y;
			Xn.z = RX.x * A1->z + RX.y * A3.z;

			gbox_merge_point3d(&Xn, gbox);
		}
	}

	return LW_SUCCESS;
}

/**
 * Convert lon/lat coordinates to cartesian coordinates on unit sphere
 */
void ll2cart(const POINT2D *g, POINT3D *p) {
	double x_rad = M_PI * g->x / 180.0;
	double y_rad = M_PI * g->y / 180.0;
	double cos_y_rad = cos(y_rad);
	p->x = cos_y_rad * cos(x_rad);
	p->y = cos_y_rad * sin(x_rad);
	p->z = sin(y_rad);
}

int ptarray_calculate_gbox_geodetic(const POINTARRAY *pa, GBOX *gbox) {
	uint32_t i;
	int first = LW_TRUE;
	const POINT2D *p;
	POINT3D A1, A2;
	GBOX edge_gbox;

	assert(gbox);
	assert(pa);

	gbox_init(&edge_gbox);
	edge_gbox.flags = gbox->flags;

	if (pa->npoints == 0)
		return LW_FAILURE;

	if (pa->npoints == 1) {
		p = getPoint2d_cp(pa, 0);
		ll2cart(p, &A1);
		gbox->xmin = gbox->xmax = A1.x;
		gbox->ymin = gbox->ymax = A1.y;
		gbox->zmin = gbox->zmax = A1.z;
		return LW_SUCCESS;
	}

	p = getPoint2d_cp(pa, 0);
	ll2cart(p, &A1);

	for (i = 1; i < pa->npoints; i++) {

		p = getPoint2d_cp(pa, i);
		ll2cart(p, &A2);

		edge_calculate_gbox(&A1, &A2, &edge_gbox);

		/* Initialize the box */
		if (first) {
			gbox_duplicate(&edge_gbox, gbox);
			first = LW_FALSE;
		}
		/* Expand the box where necessary */
		else {
			gbox_merge(&edge_gbox, gbox);
		}

		A1 = A2;
	}

	return LW_SUCCESS;
}

static int lwpoint_calculate_gbox_geodetic(const LWPOINT *point, GBOX *gbox) {
	assert(point);
	return ptarray_calculate_gbox_geodetic(point->point, gbox);
}

static int lwline_calculate_gbox_geodetic(const LWLINE *line, GBOX *gbox) {
	assert(line);
	return ptarray_calculate_gbox_geodetic(line->points, gbox);
}

static int lwpolygon_calculate_gbox_geodetic(const LWPOLY *poly, GBOX *gbox) {
	GBOX ringbox;
	uint32_t i;
	int first = LW_TRUE;
	assert(poly);
	if (poly->nrings == 0)
		return LW_FAILURE;
	ringbox.flags = gbox->flags;
	for (i = 0; i < poly->nrings; i++) {
		if (ptarray_calculate_gbox_geodetic(poly->rings[i], &ringbox) == LW_FAILURE)
			return LW_FAILURE;
		if (first) {
			gbox_duplicate(&ringbox, gbox);
			first = LW_FALSE;
		} else {
			gbox_merge(&ringbox, gbox);
		}
	}

	/* If the box wraps a poly, push that axis to the absolute min/max as appropriate */
	gbox_check_poles(gbox);

	return LW_SUCCESS;
}

static int lwtriangle_calculate_gbox_geodetic(const LWTRIANGLE *triangle, GBOX *gbox) {
	assert(triangle);
	return ptarray_calculate_gbox_geodetic(triangle->points, gbox);
}

static int lwcollection_calculate_gbox_geodetic(const LWCOLLECTION *coll, GBOX *gbox) {
	GBOX subbox = {0};
	uint32_t i;
	int result = LW_FAILURE;
	int first = LW_TRUE;
	assert(coll);
	if (coll->ngeoms == 0)
		return LW_FAILURE;

	subbox.flags = gbox->flags;

	for (i = 0; i < coll->ngeoms; i++) {
		if (lwgeom_calculate_gbox_geodetic((LWGEOM *)(coll->geoms[i]), &subbox) == LW_SUCCESS) {
			/* Keep a copy of the sub-bounding box for later */
			if (coll->geoms[i]->bbox)
				lwfree(coll->geoms[i]->bbox);
			coll->geoms[i]->bbox = gbox_copy(&subbox);
			if (first) {
				gbox_duplicate(&subbox, gbox);
				first = LW_FALSE;
			} else {
				gbox_merge(&subbox, gbox);
			}
			result = LW_SUCCESS;
		}
	}
	return result;
}

int lwgeom_calculate_gbox_geodetic(const LWGEOM *geom, GBOX *gbox) {
	int result = LW_FAILURE;

	/* Add a geodetic flag to the incoming gbox */
	gbox->flags = lwflags(FLAGS_GET_Z(geom->flags), FLAGS_GET_M(geom->flags), 1);

	switch (geom->type) {
	case POINTTYPE:
		result = lwpoint_calculate_gbox_geodetic((LWPOINT *)geom, gbox);
		break;
	case LINETYPE:
		result = lwline_calculate_gbox_geodetic((LWLINE *)geom, gbox);
		break;
	case POLYGONTYPE:
		result = lwpolygon_calculate_gbox_geodetic((LWPOLY *)geom, gbox);
		break;
	case TRIANGLETYPE:
		result = lwtriangle_calculate_gbox_geodetic((LWTRIANGLE *)geom, gbox);
		break;
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		result = lwcollection_calculate_gbox_geodetic((LWCOLLECTION *)geom, gbox);
		break;
	default:
		lwerror("lwgeom_calculate_gbox_geodetic: unsupported input geometry type: %d - %s", geom->type,
		        lwtype_name(geom->type));
		break;
	}
	return result;
}

/**
 * Initialize a geographic point
 * @param lon longitude in degrees
 * @param lat latitude in degrees
 */
void geographic_point_init(double lon, double lat, GEOGRAPHIC_POINT *g) {
	g->lat = latitude_radians_normalize(deg2rad(lat));
	g->lon = longitude_radians_normalize(deg2rad(lon));
}

/**
 * Convert a longitude to the range of -PI,PI
 */
double longitude_radians_normalize(double lon) {
	if (lon == -1.0 * M_PI)
		return M_PI;
	if (lon == -2.0 * M_PI)
		return 0.0;

	if (lon > 2.0 * M_PI)
		lon = remainder(lon, 2.0 * M_PI);

	if (lon < -2.0 * M_PI)
		lon = remainder(lon, -2.0 * M_PI);

	if (lon > M_PI)
		lon = -2.0 * M_PI + lon;

	if (lon < -1.0 * M_PI)
		lon = 2.0 * M_PI + lon;

	if (lon == -2.0 * M_PI)
		lon *= -1.0;

	return lon;
}

/**
 * Convert a latitude to the range of -PI/2,PI/2
 */
double latitude_radians_normalize(double lat) {

	if (lat > 2.0 * M_PI)
		lat = remainder(lat, 2.0 * M_PI);

	if (lat < -2.0 * M_PI)
		lat = remainder(lat, -2.0 * M_PI);

	if (lat > M_PI)
		lat = M_PI - lat;

	if (lat < -1.0 * M_PI)
		lat = -1.0 * M_PI - lat;

	if (lat > M_PI_2)
		lat = M_PI - lat;

	if (lat < -1.0 * M_PI_2)
		lat = -1.0 * M_PI - lat;

	return lat;
}

/**
 * Given two points on a unit sphere, calculate their distance apart in radians.
 */
double sphere_distance(const GEOGRAPHIC_POINT *s, const GEOGRAPHIC_POINT *e) {
	double d_lon = e->lon - s->lon;
	double cos_d_lon = cos(d_lon);
	double cos_lat_e = cos(e->lat);
	double sin_lat_e = sin(e->lat);
	double cos_lat_s = cos(s->lat);
	double sin_lat_s = sin(s->lat);

	double a1 = POW2(cos_lat_e * sin(d_lon));
	double a2 = POW2(cos_lat_s * sin_lat_e - sin_lat_s * cos_lat_e * cos_d_lon);
	double a = sqrt(a1 + a2);
	double b = sin_lat_s * sin_lat_e + cos_lat_s * cos_lat_e * cos_d_lon;
	return atan2(a, b);
}

/**
 * Given two unit vectors, calculate their distance apart in radians.
 */
double sphere_distance_cartesian(const POINT3D *s, const POINT3D *e) {
	return acos(FP_MIN(1.0, dot_product(s, e)));
}

/**
 * Given a starting location r, a distance and an azimuth
 * to the new point, compute the location of the projected point on the unit sphere.
 */
int sphere_project(const GEOGRAPHIC_POINT *r, double distance, double azimuth, GEOGRAPHIC_POINT *n) {
	double d = distance;
	double lat1 = r->lat;
	double lon1 = r->lon;
	double lat2, lon2;

	lat2 = asin(sin(lat1) * cos(d) + cos(lat1) * sin(d) * cos(azimuth));

	/* If we're going straight up or straight down, we don't need to calculate the longitude */
	/* TODO: this isn't quite true, what if we're going over the pole? */
	if (FP_EQUALS(azimuth, M_PI) || FP_EQUALS(azimuth, 0.0)) {
		lon2 = r->lon;
	} else {
		lon2 = lon1 + atan2(sin(azimuth) * sin(d) * cos(lat1), cos(d) - sin(lat1) * sin(lat2));
	}

	if (std::isnan(lat2) || std::isnan(lon2))
		return LW_FAILURE;

	n->lat = lat2;
	n->lon = lon2;

	return LW_SUCCESS;
}

double edge_distance_to_point(const GEOGRAPHIC_EDGE *e, const GEOGRAPHIC_POINT *gp, GEOGRAPHIC_POINT *closest) {
	double d1 = 1000000000.0, d2, d3, d_nearest;
	POINT3D n, p, k;
	GEOGRAPHIC_POINT gk, g_nearest;

	/* Zero length edge, */
	if (geographic_point_equals(&(e->start), &(e->end))) {
		*closest = e->start;
		return sphere_distance(&(e->start), gp);
	}

	robust_cross_product(&(e->start), &(e->end), &n);
	normalize(&n);
	geog2cart(gp, &p);
	vector_scale(&n, dot_product(&p, &n));
	vector_difference(&p, &n, &k);
	normalize(&k);
	cart2geog(&k, &gk);
	if (edge_contains_point(e, &gk)) {
		d1 = sphere_distance(gp, &gk);
	}
	d2 = sphere_distance(gp, &(e->start));
	d3 = sphere_distance(gp, &(e->end));

	d_nearest = d1;
	g_nearest = gk;

	if (d2 < d_nearest) {
		d_nearest = d2;
		g_nearest = e->start;
	}
	if (d3 < d_nearest) {
		d_nearest = d3;
		g_nearest = e->end;
	}
	if (closest)
		*closest = g_nearest;

	return d_nearest;
}

/**
 * Calculate the distance between two edges.
 * IMPORTANT: this test does not check for edge intersection!!! (distance == 0)
 * You have to check for intersection before calling this function.
 */
double edge_distance_to_edge(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *closest1,
                             GEOGRAPHIC_POINT *closest2) {
	double d;
	GEOGRAPHIC_POINT gcp1s, gcp1e, gcp2s, gcp2e, c1, c2;
	double d1s = edge_distance_to_point(e1, &(e2->start), &gcp1s);
	double d1e = edge_distance_to_point(e1, &(e2->end), &gcp1e);
	double d2s = edge_distance_to_point(e2, &(e1->start), &gcp2s);
	double d2e = edge_distance_to_point(e2, &(e1->end), &gcp2e);

	d = d1s;
	c1 = gcp1s;
	c2 = e2->start;

	if (d1e < d) {
		d = d1e;
		c1 = gcp1e;
		c2 = e2->end;
	}

	if (d2s < d) {
		d = d2s;
		c1 = e1->start;
		c2 = gcp2s;
	}

	if (d2e < d) {
		d = d2e;
		c1 = e1->end;
		c2 = gcp2e;
	}

	if (closest1)
		*closest1 = c1;
	if (closest2)
		*closest2 = c2;

	return d;
}

int geographic_point_equals(const GEOGRAPHIC_POINT *g1, const GEOGRAPHIC_POINT *g2) {
	return FP_EQUALS(g1->lat, g2->lat) && FP_EQUALS(g1->lon, g2->lon);
}

/**
 * Returns non-zero if edges A and B interact. The type of interaction is given in the
 * return value with the bitmask elements defined above.
 */
uint32_t edge_intersects(const POINT3D *A1, const POINT3D *A2, const POINT3D *B1, const POINT3D *B2) {
	POINT3D AN, BN, VN; /* Normals to plane A and plane B */
	double ab_dot;
	int a1_side, a2_side, b1_side, b2_side;
	int rv = PIR_NO_INTERACT;

	/* Normals to the A-plane and B-plane */
	unit_normal(A1, A2, &AN);
	unit_normal(B1, B2, &BN);

	/* Are A-plane and B-plane basically the same? */
	ab_dot = dot_product(&AN, &BN);

	if (FP_EQUALS(fabs(ab_dot), 1.0)) {
		/* Co-linear case */
		if (point_in_cone(A1, A2, B1) || point_in_cone(A1, A2, B2) || point_in_cone(B1, B2, A1) ||
		    point_in_cone(B1, B2, A2)) {
			rv |= PIR_INTERSECTS;
			rv |= PIR_COLINEAR;
		}
		return rv;
	}

	/* What side of plane-A and plane-B do the end points */
	/* of A and B fall? */
	a1_side = dot_product_side(&BN, A1);
	a2_side = dot_product_side(&BN, A2);
	b1_side = dot_product_side(&AN, B1);
	b2_side = dot_product_side(&AN, B2);

	/* Both ends of A on the same side of plane B. */
	if (a1_side == a2_side && a1_side != 0) {
		/* No intersection. */
		return PIR_NO_INTERACT;
	}

	/* Both ends of B on the same side of plane A. */
	if (b1_side == b2_side && b1_side != 0) {
		/* No intersection. */
		return PIR_NO_INTERACT;
	}

	/* A straddles B and B straddles A, so... */
	if (a1_side != a2_side && (a1_side + a2_side) == 0 && b1_side != b2_side && (b1_side + b2_side) == 0) {
		/* Have to check if intersection point is inside both arcs */
		unit_normal(&AN, &BN, &VN);
		if (point_in_cone(A1, A2, &VN) && point_in_cone(B1, B2, &VN)) {
			return PIR_INTERSECTS;
		}

		/* Have to check if intersection point is inside both arcs */
		vector_scale(&VN, -1);
		if (point_in_cone(A1, A2, &VN) && point_in_cone(B1, B2, &VN)) {
			return PIR_INTERSECTS;
		}

		return PIR_NO_INTERACT;
	}

	/* The rest are all intersects variants... */
	rv |= PIR_INTERSECTS;

	/* A touches B */
	if (a1_side == 0) {
		/* Touches at A1, A2 is on what side? */
		rv |= (a2_side < 0 ? PIR_A_TOUCH_RIGHT : PIR_A_TOUCH_LEFT);
	} else if (a2_side == 0) {
		/* Touches at A2, A1 is on what side? */
		rv |= (a1_side < 0 ? PIR_A_TOUCH_RIGHT : PIR_A_TOUCH_LEFT);
	}

	/* B touches A */
	if (b1_side == 0) {
		/* Touches at B1, B2 is on what side? */
		rv |= (b2_side < 0 ? PIR_B_TOUCH_RIGHT : PIR_B_TOUCH_LEFT);
	} else if (b2_side == 0) {
		/* Touches at B2, B1 is on what side? */
		rv |= (b1_side < 0 ? PIR_B_TOUCH_RIGHT : PIR_B_TOUCH_LEFT);
	}

	return rv;
}

/**
 * Returns true if an intersection can be calculated, and places it in *g.
 * Returns false otherwise.
 */
int edge_intersection(const GEOGRAPHIC_EDGE *e1, const GEOGRAPHIC_EDGE *e2, GEOGRAPHIC_POINT *g) {
	POINT3D ea, eb, v;

	if (geographic_point_equals(&(e1->start), &(e2->start))) {
		*g = e1->start;
		return LW_TRUE;
	}
	if (geographic_point_equals(&(e1->end), &(e2->end))) {
		*g = e1->end;
		return LW_TRUE;
	}
	if (geographic_point_equals(&(e1->end), &(e2->start))) {
		*g = e1->end;
		return LW_TRUE;
	}
	if (geographic_point_equals(&(e1->start), &(e2->end))) {
		*g = e1->start;
		return LW_TRUE;
	}

	robust_cross_product(&(e1->start), &(e1->end), &ea);
	normalize(&ea);
	robust_cross_product(&(e2->start), &(e2->end), &eb);
	normalize(&eb);
	if (FP_EQUALS(fabs(dot_product(&ea, &eb)), 1.0)) {
		/* Parallel (maybe equal) edges! */
		/* Hack alert, only returning ONE end of the edge right now, most do better later. */
		/* Hack alert #2, returning a value of 2 to indicate a co-linear crossing event. */
		if (edge_contains_point(e1, &(e2->start))) {
			*g = e2->start;
			return 2;
		}
		if (edge_contains_point(e1, &(e2->end))) {
			*g = e2->end;
			return 2;
		}
		if (edge_contains_point(e2, &(e1->start))) {
			*g = e1->start;
			return 2;
		}
		if (edge_contains_point(e2, &(e1->end))) {
			*g = e1->end;
			return 2;
		}
	}
	unit_normal(&ea, &eb, &v);
	g->lat = atan2(v.z, sqrt(v.x * v.x + v.y * v.y));
	g->lon = atan2(v.y, v.x);
	if (edge_contains_point(e1, g) && edge_contains_point(e2, g)) {
		return LW_TRUE;
	} else {
		g->lat = -1.0 * g->lat;
		g->lon = g->lon + M_PI;
		if (g->lon > M_PI) {
			g->lon = -1.0 * (2.0 * M_PI - g->lon);
		}
		if (edge_contains_point(e1, g) && edge_contains_point(e2, g)) {
			return LW_TRUE;
		}
	}
	return LW_FALSE;
}

/*
 * When we have a globe-covering gbox but we still want an outside
 * point, we do this Very Bad Hack, which is look at the first two points
 * in the ring and then nudge a point to the left of that arc.
 * There is an assumption of convexity built in there, as well as that
 * the shape doesn't have a sharp reversal in it. It's ugly, but
 * it fixes some common cases (large selection polygons) that users
 * are generating. At some point all of geodetic needs a clean-room
 * rewrite.
 * There is also an assumption of CCW exterior ring, which is how the
 * GeoJSON spec defined geographic ring orientation.
 */
static int lwpoly_pt_outside_hack(const LWPOLY *poly, POINT2D *pt_outside) {
	GEOGRAPHIC_POINT g1, g2, gSum;
	POINT4D p1, p2;
	POINT3D q1, q2, qMid, qCross, qSum;
	POINTARRAY *pa;
	if (lwgeom_is_empty((LWGEOM *)poly))
		return LW_FAILURE;
	if (poly->nrings < 1)
		return LW_FAILURE;
	pa = poly->rings[0];
	if (pa->npoints < 2)
		return LW_FAILURE;

	/* First two points of ring */
	getPoint4d_p(pa, 0, &p1);
	getPoint4d_p(pa, 1, &p2);
	/* Convert to XYZ unit vectors */
	geographic_point_init(p1.x, p1.y, &g1);
	geographic_point_init(p2.x, p2.y, &g2);
	geog2cart(&g1, &q1);
	geog2cart(&g2, &q2);
	/* Mid-point of first two points */
	vector_sum(&q1, &q2, &qMid);
	normalize(&qMid);
	/* Cross product of first two points (perpendicular) */
	cross_product(&q1, &q2, &qCross);
	normalize(&qCross);
	/* Invert it to put it outside, and scale down */
	vector_scale(&qCross, -0.2);
	/* Project midpoint to the right */
	vector_sum(&qMid, &qCross, &qSum);
	normalize(&qSum);
	/* Convert back to lon/lat */
	cart2geog(&qSum, &gSum);
	pt_outside->x = rad2deg(gSum.lon);
	pt_outside->y = rad2deg(gSum.lat);
	return LW_SUCCESS;
}

int lwpoly_pt_outside(const LWPOLY *poly, POINT2D *pt_outside) {
	int rv;
	/* Make sure we have boxes */
	if (poly->bbox) {
		rv = gbox_pt_outside(poly->bbox, pt_outside);
	} else {
		GBOX gbox;
		lwgeom_calculate_gbox_geodetic((LWGEOM *)poly, &gbox);
		rv = gbox_pt_outside(&gbox, pt_outside);
	}

	if (rv == LW_FALSE)
		return lwpoly_pt_outside_hack(poly, pt_outside);

	return rv;
}

/**
 * Given a unit geocentric gbox, return a lon/lat (degrees) coordinate point point that is
 * guaranteed to be outside the box (and therefore anything it contains).
 */
int gbox_pt_outside(const GBOX *gbox, POINT2D *pt_outside) {
	double grow = M_PI / 180.0 / 60.0; /* one arc-minute */
	int i;
	GBOX ge;
	POINT3D corners[8];
	POINT3D pt;
	GEOGRAPHIC_POINT g;

	while (grow < M_PI) {
		/* Assign our box and expand it slightly. */
		ge = *gbox;
		if (ge.xmin > -1)
			ge.xmin -= grow;
		if (ge.ymin > -1)
			ge.ymin -= grow;
		if (ge.zmin > -1)
			ge.zmin -= grow;
		if (ge.xmax < 1)
			ge.xmax += grow;
		if (ge.ymax < 1)
			ge.ymax += grow;
		if (ge.zmax < 1)
			ge.zmax += grow;

		/* Build our eight corner points */
		corners[0].x = ge.xmin;
		corners[0].y = ge.ymin;
		corners[0].z = ge.zmin;

		corners[1].x = ge.xmin;
		corners[1].y = ge.ymax;
		corners[1].z = ge.zmin;

		corners[2].x = ge.xmin;
		corners[2].y = ge.ymin;
		corners[2].z = ge.zmax;

		corners[3].x = ge.xmax;
		corners[3].y = ge.ymin;
		corners[3].z = ge.zmin;

		corners[4].x = ge.xmax;
		corners[4].y = ge.ymax;
		corners[4].z = ge.zmin;

		corners[5].x = ge.xmax;
		corners[5].y = ge.ymin;
		corners[5].z = ge.zmax;

		corners[6].x = ge.xmin;
		corners[6].y = ge.ymax;
		corners[6].z = ge.zmax;

		corners[7].x = ge.xmax;
		corners[7].y = ge.ymax;
		corners[7].z = ge.zmax;

		for (i = 0; i < 8; i++) {
			normalize(&(corners[i]));
			if (!gbox_contains_point3d(gbox, &(corners[i]))) {
				pt = corners[i];
				normalize(&pt);
				cart2geog(&pt, &g);
				pt_outside->x = rad2deg(g.lon);
				pt_outside->y = rad2deg(g.lat);
				return LW_SUCCESS;
			}
		}

		/* Try a wider growth to push the corners outside the original box. */
		grow *= 2.0;
	}

	/* This should never happen! */
	lwerror("BOOM! Could not generate outside point!");
	return LW_FAILURE;
}

static int ptarray_check_geodetic(const POINTARRAY *pa) {
	uint32_t t;
	POINT2D pt;

	assert(pa);

	for (t = 0; t < pa->npoints; t++) {
		getPoint2d_p(pa, t, &pt);
		/* printf( "%d (%g, %g)\n", t, pt.x, pt.y); */
		if (pt.x < -180.0 || pt.y < -90.0 || pt.x > 180.0 || pt.y > 90.0)
			return LW_FALSE;
	}

	return LW_TRUE;
}

static int lwpoint_check_geodetic(const LWPOINT *point) {
	assert(point);
	return ptarray_check_geodetic(point->point);
}

static int lwline_check_geodetic(const LWLINE *line) {
	assert(line);
	return ptarray_check_geodetic(line->points);
}

static int lwpoly_check_geodetic(const LWPOLY *poly) {
	uint32_t i = 0;
	assert(poly);

	for (i = 0; i < poly->nrings; i++) {
		if (ptarray_check_geodetic(poly->rings[i]) == LW_FALSE)
			return LW_FALSE;
	}
	return LW_TRUE;
}

static int lwtriangle_check_geodetic(const LWTRIANGLE *triangle) {
	assert(triangle);
	return ptarray_check_geodetic(triangle->points);
}

static int lwcollection_check_geodetic(const LWCOLLECTION *col) {
	uint32_t i = 0;
	assert(col);

	for (i = 0; i < col->ngeoms; i++) {
		if (lwgeom_check_geodetic(col->geoms[i]) == LW_FALSE)
			return LW_FALSE;
	}
	return LW_TRUE;
}

int lwgeom_check_geodetic(const LWGEOM *geom) {
	if (lwgeom_is_empty(geom))
		return LW_TRUE;

	switch (geom->type) {
	case POINTTYPE:
		return lwpoint_check_geodetic((LWPOINT *)geom);
	case LINETYPE:
		return lwline_check_geodetic((LWLINE *)geom);
	case POLYGONTYPE:
		return lwpoly_check_geodetic((LWPOLY *)geom);
	case TRIANGLETYPE:
		return lwtriangle_check_geodetic((LWTRIANGLE *)geom);
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return lwcollection_check_geodetic((LWCOLLECTION *)geom);
	default: {
		lwerror("lwgeom_check_geodetic: unsupported input geometry type: %d - %s", geom->type, lwtype_name(geom->type));
		return LW_FAILURE;
	}
	}
	return LW_FALSE;
}

/**
 * Returns the angle in radians at point B of the triangle formed by A-B-C
 */
static double sphere_angle(const GEOGRAPHIC_POINT *a, const GEOGRAPHIC_POINT *b, const GEOGRAPHIC_POINT *c) {
	POINT3D normal1, normal2;
	robust_cross_product(b, a, &normal1);
	robust_cross_product(b, c, &normal2);
	normalize(&normal1);
	normalize(&normal2);
	return sphere_distance_cartesian(&normal1, &normal2);
}

/**
 * Computes the spherical area of a triangle. If C is to the left of A/B,
 * the area is negative. If C is to the right of A/B, the area is positive.
 *
 * @param a The first triangle vertex.
 * @param b The second triangle vertex.
 * @param c The last triangle vertex.
 * @return the signed area in radians.
 */
static double sphere_signed_area(const GEOGRAPHIC_POINT *a, const GEOGRAPHIC_POINT *b, const GEOGRAPHIC_POINT *c) {
	double angle_a, angle_b, angle_c;
	double area_radians = 0.0;
	int side;
	GEOGRAPHIC_EDGE e;

	angle_a = sphere_angle(b, a, c);
	angle_b = sphere_angle(a, b, c);
	angle_c = sphere_angle(b, c, a);

	area_radians = angle_a + angle_b + angle_c - M_PI;

	/* What's the direction of the B/C edge? */
	e.start = *a;
	e.end = *b;
	side = edge_point_side(&e, c);

	/* Co-linear points implies no area */
	if (side == 0)
		return 0.0;

	/* Add the sign to the area */
	return side * area_radians;
}

/**
 * Returns the area of the ring (ring must be closed) in square radians (surface of
 * the sphere is 4*PI).
 */
double ptarray_area_sphere(const POINTARRAY *pa) {
	uint32_t i;
	const POINT2D *p;
	GEOGRAPHIC_POINT a, b, c;
	double area = 0.0;

	/* Return zero on nonsensical inputs */
	if (!pa || pa->npoints < 4)
		return 0.0;

	p = getPoint2d_cp(pa, 0);
	geographic_point_init(p->x, p->y, &a);
	p = getPoint2d_cp(pa, 1);
	geographic_point_init(p->x, p->y, &b);

	for (i = 2; i < pa->npoints - 1; i++) {
		p = getPoint2d_cp(pa, i);
		geographic_point_init(p->x, p->y, &c);
		area += sphere_signed_area(&a, &b, &c);
		b = c;
	}

	return fabs(area);
}

/**
 * Calculate the area of an LWGEOM. Anything except POLYGON, MULTIPOLYGON
 * and GEOMETRYCOLLECTION return zero immediately. Multi's recurse, polygons
 * calculate external ring area and subtract internal ring area. A GBOX is
 * required to calculate an outside point.
 */
double lwgeom_area_sphere(const LWGEOM *lwgeom, const SPHEROID *spheroid) {
	int type;
	double radius2 = spheroid->radius * spheroid->radius;

	assert(lwgeom);

	/* No area in nothing */
	if (lwgeom_is_empty(lwgeom))
		return 0.0;

	/* Read the geometry type number */
	type = lwgeom->type;

	/* Anything but polygons and collections returns zero */
	if (!(type == POLYGONTYPE || type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE))
		return 0.0;

	/* Actually calculate area */
	if (type == POLYGONTYPE) {
		LWPOLY *poly = (LWPOLY *)lwgeom;
		uint32_t i;
		double area = 0.0;

		/* Just in case there's no rings */
		if (poly->nrings < 1)
			return 0.0;

		/* First, the area of the outer ring */
		area += radius2 * ptarray_area_sphere(poly->rings[0]);

		/* Subtract areas of inner rings */
		for (i = 1; i < poly->nrings; i++) {
			area -= radius2 * ptarray_area_sphere(poly->rings[i]);
		}
		return area;
	}

	/* Recurse into sub-geometries to get area */
	if (type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE) {
		LWCOLLECTION *col = (LWCOLLECTION *)lwgeom;
		uint32_t i;
		double area = 0.0;

		for (i = 0; i < col->ngeoms; i++) {
			area += lwgeom_area_sphere(col->geoms[i], spheroid);
		}
		return area;
	}

	/* Shouldn't get here. */
	return 0.0;
}

double ptarray_length_spheroid(const POINTARRAY *pa, const SPHEROID *s) {
	GEOGRAPHIC_POINT a, b;
	double za = 0.0, zb = 0.0;
	POINT4D p;
	uint32_t i;
	int hasz = LW_FALSE;
	double length = 0.0;
	double seglength = 0.0;

	/* Return zero on non-sensical inputs */
	if (!pa || pa->npoints < 2)
		return 0.0;

	/* See if we have a third dimension */
	hasz = FLAGS_GET_Z(pa->flags);

	/* Initialize first point */
	getPoint4d_p(pa, 0, &p);
	geographic_point_init(p.x, p.y, &a);
	if (hasz)
		za = p.z;

	/* Loop and sum the length for each segment */
	for (i = 1; i < pa->npoints; i++) {
		seglength = 0.0;
		getPoint4d_p(pa, i, &p);
		geographic_point_init(p.x, p.y, &b);
		if (hasz)
			zb = p.z;

		/* Special sphere case */
		if (s->a == s->b)
			seglength = s->radius * sphere_distance(&a, &b);
		/* Spheroid case */
		else
			seglength = spheroid_distance(&a, &b, s);

		/* Add in the vertical displacement if we're in 3D */
		if (hasz)
			seglength = sqrt((zb - za) * (zb - za) + seglength * seglength);

		/* Add this segment length to the total */
		length += seglength;

		/* B gets incremented in the next loop, so we save the value here */
		a = b;
		za = zb;
	}
	return length;
}

double lwgeom_length_spheroid(const LWGEOM *geom, const SPHEROID *s) {
	int type;
	uint32_t i = 0;
	double length = 0.0;

	assert(geom);

	/* No area in nothing */
	if (lwgeom_is_empty(geom))
		return 0.0;

	type = geom->type;

	if (type == POINTTYPE || type == MULTIPOINTTYPE)
		return 0.0;

	if (type == LINETYPE)
		return ptarray_length_spheroid(((LWLINE *)geom)->points, s);

	if (type == POLYGONTYPE) {
		LWPOLY *poly = (LWPOLY *)geom;
		for (i = 0; i < poly->nrings; i++) {
			length += ptarray_length_spheroid(poly->rings[i], s);
		}
		return length;
	}

	if (type == TRIANGLETYPE)
		return ptarray_length_spheroid(((LWTRIANGLE *)geom)->points, s);

	if (lwtype_is_collection(type)) {
		LWCOLLECTION *col = (LWCOLLECTION *)geom;

		for (i = 0; i < col->ngeoms; i++) {
			length += lwgeom_length_spheroid(col->geoms[i], s);
		}
		return length;
	}

	lwerror("unsupported type passed to lwgeom_length_sphere");
	return 0.0;
}

/**
 * Calculate a bearing (azimuth) given a source and destination point.
 * @param r - location of first point.
 * @param s - location of second point.
 * @param spheroid - spheroid definition.
 * @return azimuth - azimuth in radians.
 *
 */
double lwgeom_azumith_spheroid(const LWPOINT *r, const LWPOINT *s, const SPHEROID *spheroid) {
	GEOGRAPHIC_POINT g1, g2;
	double x1, y1, x2, y2, az;

	/* Convert r to a geodetic point */
	x1 = lwpoint_get_x(r);
	y1 = lwpoint_get_y(r);
	geographic_point_init(x1, y1, &g1);

	/* Convert s to a geodetic point */
	x2 = lwpoint_get_x(s);
	y2 = lwpoint_get_y(s);
	geographic_point_init(x2, y2, &g2);

	/* Same point, return NaN */
	if (FP_EQUALS(x1, x2) && FP_EQUALS(y1, y2)) {
		return NAN;
	}

	/* Do the direction calculation */
	az = spheroid_direction(&g1, &g2, spheroid);
	/* Ensure result is positive */
	return az > 0 ? az : M_PI - az;
}

} // namespace duckdb
