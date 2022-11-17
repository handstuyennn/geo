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
