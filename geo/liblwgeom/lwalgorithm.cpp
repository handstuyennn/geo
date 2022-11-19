#include "liblwgeom/liblwgeom_internal.hpp"

#include <cstring>

namespace duckdb {

int p3d_same(const POINT3D *p1, const POINT3D *p2) {
	if (FP_EQUALS(p1->x, p2->x) && FP_EQUALS(p1->y, p2->y) && FP_EQUALS(p1->z, p2->z))
		return LW_TRUE;
	else
		return LW_FALSE;
}

int p2d_same(const POINT2D *p1, const POINT2D *p2) {
	if (FP_EQUALS(p1->x, p2->x) && FP_EQUALS(p1->y, p2->y))
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

static char const *base32 = "0123456789bcdefghjkmnpqrstuvwxyz";

/*
** Calculate the geohash, iterating downwards and gaining precision.
** From geohash-native.c, (c) 2008 David Troy <dave@roundhousetech.com>
** Released under the MIT License.
*/
lwvarlena_t *geohash_point(double longitude, double latitude, int precision) {
	int is_even = 1, i = 0;
	double lat[2], lon[2], mid;
	char bits[] = {16, 8, 4, 2, 1};
	int bit = 0, ch = 0;
	lwvarlena_t *v = (lwvarlena_t *)lwalloc(precision + LWVARHDRSZ);
	LWSIZE_SET(v->size, precision + LWVARHDRSZ);
	char *geohash = v->data;

	lat[0] = -90.0;
	lat[1] = 90.0;
	lon[0] = -180.0;
	lon[1] = 180.0;

	while (i < precision) {
		if (is_even) {
			mid = (lon[0] + lon[1]) / 2;
			if (longitude >= mid) {
				ch |= bits[bit];
				lon[0] = mid;
			} else {
				lon[1] = mid;
			}
		} else {
			mid = (lat[0] + lat[1]) / 2;
			if (latitude >= mid) {
				ch |= bits[bit];
				lat[0] = mid;
			} else {
				lat[1] = mid;
			}
		}

		is_even = !is_even;
		if (bit < 4) {
			bit++;
		} else {
			geohash[i++] = base32[ch];
			bit = 0;
			ch = 0;
		}
	}

	return v;
}

/*
** Decode a GeoHash into a bounding box. The lat and lon arguments should
** both be passed as double arrays of length 2 at a minimum where the values
** set in them will be the southwest and northeast coordinates of the bounding
** box accordingly. A precision less than 0 indicates that the entire length
** of the GeoHash should be used.
** It will call `lwerror` if an invalid character is found
*/
void decode_geohash_bbox(char *geohash, double *lat, double *lon, int precision) {
	bool is_even = 1;

	lat[0] = -90.0;
	lat[1] = 90.0;
	lon[0] = -180.0;
	lon[1] = 180.0;

	size_t hashlen = strlen(geohash);
	if (precision < 0 || (size_t)precision > hashlen) {
		precision = (int)hashlen;
	}

	for (int i = 0; i < precision; i++) {
		char c = tolower(geohash[i]);

		/* Valid characters are all digits in base32 */
		char *base32_pos = strchr(const_cast <char *>(base32), c);
		if (!base32_pos) {
			// lwerror("%s: Invalid character '%c'", __func__, geohash[i]);
			return;
		}
		char cd = base32_pos - base32;

		for (size_t j = 0; j < 5; j++) {
			const char bits[] = {16, 8, 4, 2, 1};
			char mask = bits[j];
			if (is_even) {
				lon[!(cd & mask)] = (lon[0] + lon[1]) / 2;
			} else {
				lat[!(cd & mask)] = (lat[0] + lat[1]) / 2;
			}
			is_even = !is_even;
		}
	}
}

int lwgeom_geohash_precision(GBOX bbox, GBOX *bounds) {
	double minx, miny, maxx, maxy;
	double latmax, latmin, lonmax, lonmin;
	double lonwidth, latwidth;
	double latmaxadjust, lonmaxadjust, latminadjust, lonminadjust;
	int precision = 0;

	/* Get the bounding box, return error if things don't work out. */
	minx = bbox.xmin;
	miny = bbox.ymin;
	maxx = bbox.xmax;
	maxy = bbox.ymax;

	if (minx == maxx && miny == maxy) {
		/* It's a point. Doubles have 51 bits of precision.
		** 2 * 51 / 5 == 20 */
		return 20;
	}

	lonmin = -180.0;
	latmin = -90.0;
	lonmax = 180.0;
	latmax = 90.0;

	/* Shrink a world bounding box until one of the edges interferes with the
	** bounds of our rectangle. */
	while (1) {
		lonwidth = lonmax - lonmin;
		latwidth = latmax - latmin;
		latmaxadjust = lonmaxadjust = latminadjust = lonminadjust = 0.0;

		if (minx > lonmin + lonwidth / 2.0) {
			lonminadjust = lonwidth / 2.0;
		} else if (maxx < lonmax - lonwidth / 2.0) {
			lonmaxadjust = -1 * lonwidth / 2.0;
		}
		if (lonminadjust || lonmaxadjust) {
			lonmin += lonminadjust;
			lonmax += lonmaxadjust;
			/* Each adjustment cycle corresponds to 2 bits of storage in the
			** geohash.	*/
			precision++;
		} else {
			break;
		}

		if (miny > latmin + latwidth / 2.0) {
			latminadjust = latwidth / 2.0;
		} else if (maxy < latmax - latwidth / 2.0) {
			latmaxadjust = -1 * latwidth / 2.0;
		}
		/* Only adjust if adjustments are legal (we haven't crossed any edges). */
		if (latminadjust || latmaxadjust) {
			latmin += latminadjust;
			latmax += latmaxadjust;
			/* Each adjustment cycle corresponds to 2 bits of storage in the
			** geohash.	*/
			precision++;
		} else {
			break;
		}
	}

	/* Save the edges of our bounds, in case someone cares later. */
	bounds->xmin = lonmin;
	bounds->xmax = lonmax;
	bounds->ymin = latmin;
	bounds->ymax = latmax;

	/* Each geohash character (base32) can contain 5 bits of information.
	** We are returning the precision in characters, so here we divide. */
	return precision / 5;
}

/*
** Return a geohash string for the geometry. <http://geohash.org>
** Where the precision is non-positive, calculate a precision based on the
** bounds of the feature. Big features have loose precision.
** Small features have tight precision.
*/
lwvarlena_t *lwgeom_geohash(const LWGEOM *lwgeom, int precision) {
	GBOX gbox = {0};
	GBOX gbox_bounds = {0};
	double lat, lon;
	int result;

	gbox_init(&gbox);
	gbox_init(&gbox_bounds);

	result = lwgeom_calculate_gbox_cartesian(lwgeom, &gbox);
	if (result == LW_FAILURE)
		return NULL;

	/* Return error if we are being fed something outside our working bounds */
	if (gbox.xmin < -180 || gbox.ymin < -90 || gbox.xmax > 180 || gbox.ymax > 90) {
		// lwerror("Geohash requires inputs in decimal degrees, got (%g %g, %g %g).", gbox.xmin, gbox.ymin, gbox.xmax,
		//         gbox.ymax);
		return NULL;
	}

	/* What is the center of our geometry bounds? We'll use that to
	** approximate location. */
	lon = gbox.xmin + (gbox.xmax - gbox.xmin) / 2;
	lat = gbox.ymin + (gbox.ymax - gbox.ymin) / 2;

	if (precision <= 0) {
		precision = lwgeom_geohash_precision(gbox, &gbox_bounds);
	}

	/*
	** Return the geohash of the center, with a precision determined by the
	** extent of the bounds.
	** Possible change: return the point at the center of the precision bounds?
	*/
	return geohash_point(lon, lat, precision);
}

} // namespace duckdb
