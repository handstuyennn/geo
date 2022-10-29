#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

int p3d_same(const POINT3D *p1, const POINT3D *p2) {
	if (FP_EQUALS(p1->x, p2->x) && FP_EQUALS(p1->y, p2->y) && FP_EQUALS(p1->z, p2->z))
		return LW_TRUE;
	else
		return LW_FALSE;
}

/**
 * lw_segment_side()
 *
 * Return -1  if point Q is left of segment P
 * Return  1  if point Q is right of segment P
 * Return  0  if point Q in on segment P
 */
int lw_segment_side(const POINT2D *p1, const POINT2D *p2, const POINT2D *q) {
	double side = ((q->x - p1->x) * (p2->y - p1->y) - (p2->x - p1->x) * (q->y - p1->y));
	return SIGNUM(side);
}

/**
 * Determines the center of the circle defined by the three given points.
 * In the event the circle is complete, the midpoint of the segment defined
 * by the first and second points is returned.  If the points are collinear,
 * as determined by equal slopes, then -1.0 is returned.  If the interior
 * point is coincident with either end point, they are taken as collinear.
 * For non-collinear cases, arc radious is returned.
 */
double lw_arc_center(const POINT2D *p1, const POINT2D *p2, const POINT2D *p3, POINT2D *result) {
	POINT2D c;
	double cx, cy, cr;
	double dx21, dy21, dx31, dy31, h21, h31, d;

	c.x = c.y = 0.0;

	/* Closed circle */
	if (fabs(p1->x - p3->x) < EPSILON_SQLMM && fabs(p1->y - p3->y) < EPSILON_SQLMM) {
		cx = p1->x + (p2->x - p1->x) / 2.0;
		cy = p1->y + (p2->y - p1->y) / 2.0;
		c.x = cx;
		c.y = cy;
		*result = c;
		cr = sqrt(pow(cx - p1->x, 2.0) + pow(cy - p1->y, 2.0));
		return cr;
	}

	/* Using cartesian eguations from page https://en.wikipedia.org/wiki/Circumscribed_circle */
	dx21 = p2->x - p1->x;
	dy21 = p2->y - p1->y;
	dx31 = p3->x - p1->x;
	dy31 = p3->y - p1->y;

	h21 = pow(dx21, 2.0) + pow(dy21, 2.0);
	h31 = pow(dx31, 2.0) + pow(dy31, 2.0);

	/* 2 * |Cross product|, d<0 means clockwise and d>0 counterclockwise sweeping angle */
	d = 2 * (dx21 * dy31 - dx31 * dy21);

	/* Check colinearity, |Cross product| = 0 */
	if (fabs(d) < EPSILON_SQLMM)
		return -1.0;

	/* Calculate centroid coordinates and radius */
	cx = p1->x + (h21 * dy31 - h31 * dy21) / d;
	cy = p1->y - (h21 * dx31 - h31 * dx21) / d;
	c.x = cx;
	c.y = cy;
	*result = c;
	cr = sqrt(pow(cx - p1->x, 2) + pow(cy - p1->y, 2));

	return cr;
}

/*
** Calculate the geohash, iterating downwards and gaining precision.
** From geohash-native.c, (c) 2008 David Troy <dave@roundhousetech.com>
** Released under the MIT License.
*/
unsigned int geohash_point_as_int(POINT2D *pt) {
	int is_even = 1;
	double lat[2], lon[2], mid;
	int bit = 32;
	unsigned int ch = 0;

	double longitude = pt->x;
	double latitude = pt->y;

	lat[0] = -90.0;
	lat[1] = 90.0;
	lon[0] = -180.0;
	lon[1] = 180.0;

	while (--bit >= 0) {
		if (is_even) {
			mid = (lon[0] + lon[1]) / 2;
			if (longitude > mid) {
				ch |= 0x0001u << bit;
				lon[0] = mid;
			} else {
				lon[1] = mid;
			}
		} else {
			mid = (lat[0] + lat[1]) / 2;
			if (latitude > mid) {
				ch |= 0x0001 << bit;
				lat[0] = mid;
			} else {
				lat[1] = mid;
			}
		}

		is_even = !is_even;
	}
	return ch;
}

} // namespace duckdb
