#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <cstring>

namespace duckdb {

void ptarray_free(POINTARRAY *pa) {
	if (pa) {
		if (pa->serialized_pointlist && (!FLAGS_GET_READONLY(pa->flags)))
			lwfree(pa->serialized_pointlist);
		lwfree(pa);
	}
}

POINTARRAY *ptarray_construct(char hasz, char hasm, uint32_t npoints) {
	POINTARRAY *pa = ptarray_construct_empty(hasz, hasm, npoints);
	pa->npoints = npoints;
	return pa;
}

POINTARRAY *ptarray_construct_copy_data(char hasz, char hasm, uint32_t npoints, const uint8_t *ptlist) {
	POINTARRAY *pa = (POINTARRAY *)lwalloc(sizeof(POINTARRAY));

	pa->flags = lwflags(hasz, hasm, 0);
	pa->npoints = npoints;
	pa->maxpoints = npoints;

	if (npoints > 0) {
		pa->serialized_pointlist = (uint8_t *)lwalloc(ptarray_point_size(pa) * npoints);
		memcpy(pa->serialized_pointlist, ptlist, ptarray_point_size(pa) * npoints);
	} else {
		pa->serialized_pointlist = NULL;
	}

	return pa;
}

/**
 * Build a new #POINTARRAY, but on top of someone else's ordinate array.
 * Flag as read-only, so that ptarray_free() does not free the serialized_ptlist
 */
POINTARRAY *ptarray_construct_reference_data(char hasz, char hasm, uint32_t npoints, uint8_t *ptlist) {
	POINTARRAY *pa = (POINTARRAY *)lwalloc(sizeof(POINTARRAY));
	pa->flags = lwflags(hasz, hasm, 0);
	FLAGS_SET_READONLY(pa->flags, 1); /* We don't own this memory, so we can't alter or free it. */
	pa->npoints = npoints;
	pa->maxpoints = npoints;
	pa->serialized_pointlist = ptlist;
	return pa;
}

POINTARRAY *ptarray_construct_empty(char hasz, char hasm, uint32_t maxpoints) {
	POINTARRAY *pa = (POINTARRAY *)lwalloc(sizeof(POINTARRAY));
	pa->serialized_pointlist = NULL;

	/* Set our dimensionality info on the bitmap */
	pa->flags = lwflags(hasz, hasm, 0);

	/* We will be allocating a bit of room */
	pa->npoints = 0;
	pa->maxpoints = maxpoints;

	/* Allocate the coordinate array */
	if (maxpoints > 0)
		pa->serialized_pointlist = (uint8_t *)lwalloc(maxpoints * ptarray_point_size(pa));
	else
		pa->serialized_pointlist = NULL;

	return pa;
}

int ptarray_append_point(POINTARRAY *pa, const POINT4D *pt, int repeated_points) {
	/* Check for pathology */
	if (!pa || !pt) {
		// lwerror("ptarray_append_point: null input");
		return LW_FAILURE;
	}

	/* Check for duplicate end point */
	if (repeated_points == LW_FALSE && pa->npoints > 0) {
		POINT4D tmp;
		getPoint4d_p(pa, pa->npoints - 1, &tmp);

		/* Return LW_SUCCESS and do nothing else if previous point in list is equal to this one */
		if ((pt->x == tmp.x) && (pt->y == tmp.y) && (FLAGS_GET_Z(pa->flags) ? pt->z == tmp.z : 1) &&
		    (FLAGS_GET_M(pa->flags) ? pt->m == tmp.m : 1)) {
			return LW_SUCCESS;
		}
	}

	/* Append is just a special case of insert */
	return ptarray_insert_point(pa, pt, pa->npoints);
}

int ptarray_append_ptarray(POINTARRAY *pa1, POINTARRAY *pa2, double gap_tolerance) {
	unsigned int poff = 0;
	unsigned int npoints;
	unsigned int ncap;
	unsigned int ptsize;

	/* Check for pathology */
	if (!pa1 || !pa2) {
		// lwerror("ptarray_append_ptarray: null input");
		return LW_FAILURE;
	}

	npoints = pa2->npoints;

	if (!npoints)
		return LW_SUCCESS; /* nothing more to do */

	if (FLAGS_GET_READONLY(pa1->flags)) {
		// lwerror("ptarray_append_ptarray: target pointarray is read-only");
		return LW_FAILURE;
	}

	if (FLAGS_GET_ZM(pa1->flags) != FLAGS_GET_ZM(pa2->flags)) {
		// lwerror("ptarray_append_ptarray: appending mixed dimensionality is not allowed");
		return LW_FAILURE;
	}

	ptsize = ptarray_point_size(pa1);

	/* Check for duplicate end point */
	if (pa1->npoints) {
		POINT2D tmp1, tmp2;
		getPoint2d_p(pa1, pa1->npoints - 1, &tmp1);
		getPoint2d_p(pa2, 0, &tmp2);

		/* If the end point and start point are the same, then don't copy start point */
		if (p2d_same(&tmp1, &tmp2)) {
			poff = 1;
			--npoints;
		} else if (gap_tolerance == 0 || (gap_tolerance > 0 && distance2d_pt_pt(&tmp1, &tmp2) > gap_tolerance)) {
			// lwerror("Second line start point too far from first line end point");
			return LW_FAILURE;
		}
	}

	/* Check if we need extra space */
	ncap = pa1->npoints + npoints;
	if (pa1->maxpoints < ncap) {
		pa1->maxpoints = ncap > pa1->maxpoints * 2 ? ncap : pa1->maxpoints * 2;
		pa1->serialized_pointlist =
		    static_cast<uint8_t *>(lwrealloc(pa1->serialized_pointlist, ptsize * pa1->maxpoints));
	}

	memcpy(getPoint_internal(pa1, pa1->npoints), getPoint_internal(pa2, poff), ptsize * npoints);

	pa1->npoints = ncap;

	return LW_SUCCESS;
}

/**
 * @brief Deep clone a pointarray (also clones serialized pointlist)
 */
POINTARRAY *ptarray_clone_deep(const POINTARRAY *in) {
	POINTARRAY *out = (POINTARRAY *)lwalloc(sizeof(POINTARRAY));

	out->flags = in->flags;
	out->npoints = in->npoints;
	out->maxpoints = in->npoints;

	FLAGS_SET_READONLY(out->flags, 0);

	if (!in->npoints) {
		// Avoid calling lwalloc of 0 bytes
		out->serialized_pointlist = NULL;
	} else {
		size_t size = in->npoints * ptarray_point_size(in);
		out->serialized_pointlist = (uint8_t *)lwalloc(size);
		memcpy(out->serialized_pointlist, in->serialized_pointlist, size);
	}

	return out;
}

int ptarray_is_closed_2d(const POINTARRAY *in) {
	if (!in) {
		// lwerror("ptarray_is_closed_2d: called with null point array");
		return 0;
	}
	if (in->npoints <= 1)
		return in->npoints; /* single-point are closed, empty not closed */

	return 0 == memcmp(getPoint_internal(in, 0), getPoint_internal(in, in->npoints - 1), sizeof(POINT2D));
}

/**
 * Return 1 if the point is inside the POINTARRAY, -1 if it is outside,
 * and 0 if it is on the boundary.
 */
int ptarray_contains_point(const POINTARRAY *pa, const POINT2D *pt) {
	return ptarray_contains_point_partial(pa, pt, LW_TRUE, NULL);
}

int ptarray_contains_point_partial(const POINTARRAY *pa, const POINT2D *pt, int check_closed, int *winding_number) {
	int wn = 0;
	uint32_t i;
	double side;
	const POINT2D *seg1;
	const POINT2D *seg2;
	double ymin, ymax;

	seg1 = getPoint2d_cp(pa, 0);
	seg2 = getPoint2d_cp(pa, pa->npoints - 1);
	if (check_closed && !p2d_same(seg1, seg2))
		lwerror("ptarray_contains_point called on unclosed ring");

	for (i = 1; i < pa->npoints; i++) {
		seg2 = getPoint2d_cp(pa, i);

		/* Zero length segments are ignored. */
		if (seg1->x == seg2->x && seg1->y == seg2->y) {
			seg1 = seg2;
			continue;
		}

		ymin = FP_MIN(seg1->y, seg2->y);
		ymax = FP_MAX(seg1->y, seg2->y);

		/* Only test segments in our vertical range */
		if (pt->y > ymax || pt->y < ymin) {
			seg1 = seg2;
			continue;
		}

		side = lw_segment_side(seg1, seg2, pt);

		/*
		 * A point on the boundary of a ring is not contained.
		 * WAS: if (fabs(side) < 1e-12), see #852
		 */
		if ((side == 0) && lw_pt_in_seg(pt, seg1, seg2)) {
			return LW_BOUNDARY;
		}

		/*
		 * If the point is to the left of the line, and it's rising,
		 * then the line is to the right of the point and
		 * circling counter-clockwise, so increment.
		 */
		if ((side < 0) && (seg1->y <= pt->y) && (pt->y < seg2->y)) {
			wn++;
		}

		/*
		 * If the point is to the right of the line, and it's falling,
		 * then the line is to the right of the point and circling
		 * clockwise, so decrement.
		 */
		else if ((side > 0) && (seg2->y <= pt->y) && (pt->y < seg1->y)) {
			wn--;
		}

		seg1 = seg2;
	}

	/* Sent out the winding number for calls that are building on this as a primitive */
	if (winding_number)
		*winding_number = wn;

	/* Outside */
	if (wn == 0) {
		return LW_OUTSIDE;
	}

	/* Inside */
	return LW_INSIDE;
}

/**
 * For POINTARRAYs representing CIRCULARSTRINGS. That is, linked triples
 * with each triple being control points of a circular arc. Such
 * POINTARRAYs have an odd number of vertices.
 *
 * Return 1 if the point is inside the POINTARRAY, -1 if it is outside,
 * and 0 if it is on the boundary.
 */

int ptarrayarc_contains_point(const POINTARRAY *pa, const POINT2D *pt) {
	return ptarrayarc_contains_point_partial(pa, pt, LW_TRUE /* Check closed*/, NULL);
}

int ptarrayarc_contains_point_partial(const POINTARRAY *pa, const POINT2D *pt, int check_closed, int *winding_number) {
	int wn = 0;
	uint32_t i;
	int side;
	const POINT2D *seg1;
	const POINT2D *seg2;
	const POINT2D *seg3;
	GBOX gbox;

	/* Check for not an arc ring (always have odd # of points) */
	if ((pa->npoints % 2) == 0) {
		lwerror("ptarrayarc_contains_point called with even number of points");
		return LW_OUTSIDE;
	}

	/* Check for not an arc ring (always have >= 3 points) */
	if (pa->npoints < 3) {
		lwerror("ptarrayarc_contains_point called too-short pointarray");
		return LW_OUTSIDE;
	}

	/* Check for unclosed case */
	seg1 = getPoint2d_cp(pa, 0);
	seg3 = getPoint2d_cp(pa, pa->npoints - 1);
	if (check_closed && !p2d_same(seg1, seg3)) {
		lwerror("ptarrayarc_contains_point called on unclosed ring");
		return LW_OUTSIDE;
	}
	/* OK, it's closed. Is it just one circle? */
	else if (p2d_same(seg1, seg3) && pa->npoints == 3) {
		double radius, d;
		POINT2D c;
		seg2 = getPoint2d_cp(pa, 1);

		/* Wait, it's just a point, so it can't contain anything */
		if (lw_arc_is_pt(seg1, seg2, seg3))
			return LW_OUTSIDE;

		/* See if the point is within the circle radius */
		radius = lw_arc_center(seg1, seg2, seg3, &c);
		d = distance2d_pt_pt(pt, &c);
		if (FP_EQUALS(d, radius))
			return LW_BOUNDARY; /* Boundary of circle */
		else if (d < radius)
			return LW_INSIDE; /* Inside circle */
		else
			return LW_OUTSIDE; /* Outside circle */
	} else if (p2d_same(seg1, pt) || p2d_same(seg3, pt)) {
		return LW_BOUNDARY; /* Boundary case */
	}

	/* Start on the ring */
	seg1 = getPoint2d_cp(pa, 0);
	for (i = 1; i < pa->npoints; i += 2) {
		seg2 = getPoint2d_cp(pa, i);
		seg3 = getPoint2d_cp(pa, i + 1);

		/* Catch an easy boundary case */
		if (p2d_same(seg3, pt))
			return LW_BOUNDARY;

		/* Skip arcs that have no size */
		if (lw_arc_is_pt(seg1, seg2, seg3)) {
			seg1 = seg3;
			continue;
		}

		/* Only test segments in our vertical range */
		lw_arc_calculate_gbox_cartesian_2d(seg1, seg2, seg3, &gbox);
		if (pt->y > gbox.ymax || pt->y < gbox.ymin) {
			seg1 = seg3;
			continue;
		}

		/* Outside of horizontal range, and not between end points we also skip */
		if ((pt->x > gbox.xmax || pt->x < gbox.xmin) &&
		    (pt->y > FP_MAX(seg1->y, seg3->y) || pt->y < FP_MIN(seg1->y, seg3->y))) {
			seg1 = seg3;
			continue;
		}

		side = lw_arc_side(seg1, seg2, seg3, pt);

		/* On the boundary */
		if ((side == 0) && lw_pt_in_arc(pt, seg1, seg2, seg3)) {
			return LW_BOUNDARY;
		}

		/* Going "up"! Point to left of arc. */
		if (side < 0 && (seg1->y <= pt->y) && (pt->y < seg3->y)) {
			wn++;
		}

		/* Going "down"! */
		if (side > 0 && (seg2->y <= pt->y) && (pt->y < seg1->y)) {
			wn--;
		}

		/* Inside the arc! */
		if (pt->x <= gbox.xmax && pt->x >= gbox.xmin) {
			POINT2D C;
			double radius = lw_arc_center(seg1, seg2, seg3, &C);
			double d = distance2d_pt_pt(pt, &C);

			/* On the boundary! */
			if (d == radius)
				return LW_BOUNDARY;

			/* Within the arc! */
			if (d < radius) {
				/* Left side, increment winding number */
				if (side < 0)
					wn++;
				/* Right side, decrement winding number */
				if (side > 0)
					wn--;
			}
		}

		seg1 = seg3;
	}

	/* Sent out the winding number for calls that are building on this as a primitive */
	if (winding_number)
		*winding_number = wn;

	/* Outside */
	if (wn == 0) {
		return LW_OUTSIDE;
	}

	/* Inside */
	return LW_INSIDE;
}

int ptarray_is_closed_3d(const POINTARRAY *in) {
	if (!in) {
		// lwerror("ptarray_is_closed_3d: called with null point array");
		return 0;
	}
	if (in->npoints <= 1)
		return in->npoints; /* single-point are closed, empty not closed */

	return 0 == memcmp(getPoint_internal(in, 0), getPoint_internal(in, in->npoints - 1), sizeof(POINT3D));
}

int ptarray_is_closed_z(const POINTARRAY *in) {
	if (FLAGS_GET_Z(in->flags))
		return ptarray_is_closed_3d(in);
	else
		return ptarray_is_closed_2d(in);
}

/*
 * Add a point into a pointarray. Only adds as many dimensions as the
 * pointarray supports.
 */
int ptarray_insert_point(POINTARRAY *pa, const POINT4D *p, uint32_t where) {
	if (!pa || !p)
		return LW_FAILURE;
	size_t point_size = ptarray_point_size(pa);

	if (FLAGS_GET_READONLY(pa->flags)) {
		// lwerror("ptarray_insert_point: called on read-only point array");
		return LW_FAILURE;
	}

	/* Error on invalid offset value */
	if (where > pa->npoints) {
		// lwerror("ptarray_insert_point: offset out of range (%d)", where);
		return LW_FAILURE;
	}

	/* If we have no storage, let's allocate some */
	if (pa->maxpoints == 0 || !pa->serialized_pointlist) {
		pa->maxpoints = 32;
		pa->npoints = 0;
		pa->serialized_pointlist = (uint8_t *)lwalloc(ptarray_point_size(pa) * pa->maxpoints);
	}

	/* Error out if we have a bad situation */
	if (pa->npoints > pa->maxpoints) {
		// lwerror("npoints (%d) is greater than maxpoints (%d)", pa->npoints, pa->maxpoints);
		return LW_FAILURE;
	}

	/* Check if we have enough storage, add more if necessary */
	if (pa->npoints == pa->maxpoints) {
		pa->maxpoints *= 2;
		pa->serialized_pointlist =
		    (uint8_t *)lwrealloc(pa->serialized_pointlist, ptarray_point_size(pa) * pa->maxpoints);
	}

	/* Make space to insert the new point */
	if (where < pa->npoints) {
		size_t copy_size = point_size * (pa->npoints - where);
		memmove(getPoint_internal(pa, where + 1), getPoint_internal(pa, where), copy_size);
	}

	/* We have one more point */
	++pa->npoints;

	/* Copy the new point into the gap */
	ptarray_set_point4d(pa, where, p);

	return LW_SUCCESS;
}

POINTARRAY *ptarray_force_dims(const POINTARRAY *pa, int hasz, int hasm, double zval, double mval) {
	/* TODO handle zero-length point arrays */
	uint32_t i;
	int in_hasz = FLAGS_GET_Z(pa->flags);
	int in_hasm = FLAGS_GET_M(pa->flags);
	POINT4D pt;
	POINTARRAY *pa_out = ptarray_construct_empty(hasz, hasm, pa->npoints);

	for (i = 0; i < pa->npoints; i++) {
		getPoint4d_p(pa, i, &pt);
		if (hasz && !in_hasz)
			pt.z = zval;
		if (hasm && !in_hasm)
			pt.m = mval;
		ptarray_append_point(pa_out, &pt, LW_TRUE);
	}

	return pa_out;
}

int ptarray_startpoint(const POINTARRAY *pa, POINT4D *pt) {
	return getPoint4d_p(pa, 0, pt);
}

} // namespace duckdb
