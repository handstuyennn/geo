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

#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <cassert>
#include <cstring>

namespace duckdb {

GBOX *gbox_new(lwflags_t flags) {
	GBOX *g = (GBOX *)lwalloc(sizeof(GBOX));
	gbox_init(g);
	g->flags = flags;
	return g;
}

void gbox_init(GBOX *gbox) {
	memset(gbox, 0, sizeof(GBOX));
}

GBOX *gbox_clone(const GBOX *gbox) {
	GBOX *g = (GBOX *)lwalloc(sizeof(GBOX));
	memcpy(g, gbox, sizeof(GBOX));
	return g;
}

int gbox_merge(const GBOX *new_box, GBOX *merge_box) {
	assert(merge_box);

	if (FLAGS_GET_ZM(merge_box->flags) != FLAGS_GET_ZM(new_box->flags))
		return LW_FAILURE;

	if (new_box->xmin < merge_box->xmin)
		merge_box->xmin = new_box->xmin;
	if (new_box->ymin < merge_box->ymin)
		merge_box->ymin = new_box->ymin;
	if (new_box->xmax > merge_box->xmax)
		merge_box->xmax = new_box->xmax;
	if (new_box->ymax > merge_box->ymax)
		merge_box->ymax = new_box->ymax;

	if (FLAGS_GET_Z(merge_box->flags) || FLAGS_GET_GEODETIC(merge_box->flags)) {
		if (new_box->zmin < merge_box->zmin)
			merge_box->zmin = new_box->zmin;
		if (new_box->zmax > merge_box->zmax)
			merge_box->zmax = new_box->zmax;
	}
	if (FLAGS_GET_M(merge_box->flags)) {
		if (new_box->mmin < merge_box->mmin)
			merge_box->mmin = new_box->mmin;
		if (new_box->mmax > merge_box->mmax)
			merge_box->mmax = new_box->mmax;
	}

	return LW_SUCCESS;
}

GBOX *gbox_copy(const GBOX *box) {
	GBOX *copy = (GBOX *)lwalloc(sizeof(GBOX));
	memcpy(copy, box, sizeof(GBOX));
	return copy;
}

int gbox_init_point3d(const POINT3D *p, GBOX *gbox) {
	gbox->xmin = gbox->xmax = p->x;
	gbox->ymin = gbox->ymax = p->y;
	gbox->zmin = gbox->zmax = p->z;
	return LW_SUCCESS;
}

int gbox_contains_point3d(const GBOX *gbox, const POINT3D *pt) {
	if (gbox->xmin > pt->x || gbox->ymin > pt->y || gbox->zmin > pt->z || gbox->xmax < pt->x || gbox->ymax < pt->y ||
	    gbox->zmax < pt->z) {
		return LW_FALSE;
	}
	return LW_TRUE;
}

int gbox_merge_point3d(const POINT3D *p, GBOX *gbox) {
	if (gbox->xmin > p->x)
		gbox->xmin = p->x;
	if (gbox->ymin > p->y)
		gbox->ymin = p->y;
	if (gbox->zmin > p->z)
		gbox->zmin = p->z;
	if (gbox->xmax < p->x)
		gbox->xmax = p->x;
	if (gbox->ymax < p->y)
		gbox->ymax = p->y;
	if (gbox->zmax < p->z)
		gbox->zmax = p->z;
	return LW_SUCCESS;
}

int gbox_contains_2d(const GBOX *g1, const GBOX *g2) {
	if ((g2->xmin < g1->xmin) || (g2->xmax > g1->xmax) || (g2->ymin < g1->ymin) || (g2->ymax > g1->ymax)) {
		return LW_FALSE;
	}
	return LW_TRUE;
}

int gbox_overlaps_2d(const GBOX *g1, const GBOX *g2) {

	/* Make sure our boxes are consistent */
	if (FLAGS_GET_GEODETIC(g1->flags) != FLAGS_GET_GEODETIC(g2->flags))
		lwerror("gbox_overlaps: cannot compare geodetic and non-geodetic boxes");

	/* Check X/Y first */
	if (g1->xmax < g2->xmin || g1->ymax < g2->ymin || g1->xmin > g2->xmax || g1->ymin > g2->ymax)
		return LW_FALSE;

	return LW_TRUE;
}

void gbox_duplicate(const GBOX *original, GBOX *duplicate) {
	assert(duplicate);
	assert(original);
	memcpy(duplicate, original, sizeof(GBOX));
}

size_t gbox_serialized_size(lwflags_t flags) {
	if (FLAGS_GET_GEODETIC(flags))
		return 6 * sizeof(float);
	else
		return 2 * FLAGS_NDIMS(flags) * sizeof(float);
}

int gbox_same_2d_float(const GBOX *g1, const GBOX *g2) {
	if ((g1->xmax == g2->xmax || next_float_up(g1->xmax) == next_float_up(g2->xmax)) &&
	    (g1->ymax == g2->ymax || next_float_up(g1->ymax) == next_float_up(g2->ymax)) &&
	    (g1->xmin == g2->xmin || next_float_down(g1->xmin) == next_float_down(g1->xmin)) &&
	    (g1->ymin == g2->ymin || next_float_down(g2->ymin) == next_float_down(g2->ymin)))
		return LW_TRUE;
	return LW_FALSE;
}

/* ********************************************************************************
** Compute cartesian bounding GBOX boxes from LWGEOM.
*/

int lw_arc_calculate_gbox_cartesian_2d(const POINT2D *A1, const POINT2D *A2, const POINT2D *A3, GBOX *gbox) {
	POINT2D xmin, ymin, xmax, ymax;
	POINT2D C;
	int A2_side;
	double radius_A;

	radius_A = lw_arc_center(A1, A2, A3, &C);

	/* Negative radius signals straight line, p1/p2/p3 are collinear */
	if (radius_A < 0.0) {
		gbox->xmin = FP_MIN(A1->x, A3->x);
		gbox->ymin = FP_MIN(A1->y, A3->y);
		gbox->xmax = FP_MAX(A1->x, A3->x);
		gbox->ymax = FP_MAX(A1->y, A3->y);
		return LW_SUCCESS;
	}

	/* Matched start/end points imply circle */
	if (A1->x == A3->x && A1->y == A3->y) {
		gbox->xmin = C.x - radius_A;
		gbox->ymin = C.y - radius_A;
		gbox->xmax = C.x + radius_A;
		gbox->ymax = C.y + radius_A;
		return LW_SUCCESS;
	}

	/* First approximation, bounds of start/end points */
	gbox->xmin = FP_MIN(A1->x, A3->x);
	gbox->ymin = FP_MIN(A1->y, A3->y);
	gbox->xmax = FP_MAX(A1->x, A3->x);
	gbox->ymax = FP_MAX(A1->y, A3->y);

	/* Create points for the possible extrema */
	xmin.x = C.x - radius_A;
	xmin.y = C.y;
	ymin.x = C.x;
	ymin.y = C.y - radius_A;
	xmax.x = C.x + radius_A;
	xmax.y = C.y;
	ymax.x = C.x;
	ymax.y = C.y + radius_A;

	/* Divide the circle into two parts, one on each side of a line
	   joining p1 and p3. The circle extrema on the same side of that line
	   as p2 is on, are also the extrema of the bbox. */

	A2_side = lw_segment_side(A1, A3, A2);

	if (A2_side == lw_segment_side(A1, A3, &xmin))
		gbox->xmin = xmin.x;

	if (A2_side == lw_segment_side(A1, A3, &ymin))
		gbox->ymin = ymin.y;

	if (A2_side == lw_segment_side(A1, A3, &xmax))
		gbox->xmax = xmax.x;

	if (A2_side == lw_segment_side(A1, A3, &ymax))
		gbox->ymax = ymax.y;

	return LW_SUCCESS;
}

static int lw_arc_calculate_gbox_cartesian(const POINT4D *p1, const POINT4D *p2, const POINT4D *p3, GBOX *gbox) {
	int rv;

	rv = lw_arc_calculate_gbox_cartesian_2d((POINT2D *)p1, (POINT2D *)p2, (POINT2D *)p3, gbox);
	gbox->zmin = FP_MIN(p1->z, p3->z);
	gbox->mmin = FP_MIN(p1->m, p3->m);
	gbox->zmax = FP_MAX(p1->z, p3->z);
	gbox->mmax = FP_MAX(p1->m, p3->m);
	return rv;
}

static void ptarray_calculate_gbox_cartesian_2d(const POINTARRAY *pa, GBOX *gbox) {
	const POINT2D *p = getPoint2d_cp(pa, 0);

	gbox->xmax = gbox->xmin = p->x;
	gbox->ymax = gbox->ymin = p->y;

	for (uint32_t i = 1; i < pa->npoints; i++) {
		p = getPoint2d_cp(pa, i);
		gbox->xmin = FP_MIN(gbox->xmin, p->x);
		gbox->xmax = FP_MAX(gbox->xmax, p->x);
		gbox->ymin = FP_MIN(gbox->ymin, p->y);
		gbox->ymax = FP_MAX(gbox->ymax, p->y);
	}
}

int ptarray_calculate_gbox_cartesian(const POINTARRAY *pa, GBOX *gbox) {
	if (!pa || pa->npoints == 0)
		return LW_FAILURE;
	if (!gbox)
		return LW_FAILURE;

	int has_z = FLAGS_GET_Z(pa->flags);
	int has_m = FLAGS_GET_M(pa->flags);
	gbox->flags = lwflags(has_z, has_m, 0);
	int coordinates = 2 + has_z + has_m;

	switch (coordinates) {
	case 2: {
		ptarray_calculate_gbox_cartesian_2d(pa, gbox);
		break;
	}
		// case 3:
		// {
		// 	if (has_z)
		// 	{
		// 		ptarray_calculate_gbox_cartesian_3d(pa, gbox);
		// 	}
		// 	else
		// 	{
		// 		double zmin = gbox->zmin;
		// 		double zmax = gbox->zmax;
		// 		ptarray_calculate_gbox_cartesian_3d(pa, gbox);
		// 		gbox->mmin = gbox->zmin;
		// 		gbox->mmax = gbox->zmax;
		// 		gbox->zmin = zmin;
		// 		gbox->zmax = zmax;
		// 	}
		// 	break;
		// }
		// default:
		// {
		// 	ptarray_calculate_gbox_cartesian_4d(pa, gbox);
		// 	break;
		// }
	}
	return LW_SUCCESS;
}

static int lwcircstring_calculate_gbox_cartesian(LWCIRCSTRING *curve, GBOX *gbox) {
	GBOX tmp = {0};
	POINT4D p1, p2, p3;
	uint32_t i;

	if (!curve)
		return LW_FAILURE;
	if (curve->points->npoints < 3)
		return LW_FAILURE;

	tmp.flags = lwflags(FLAGS_GET_Z(curve->flags), FLAGS_GET_M(curve->flags), 0);

	/* Initialize */
	gbox->xmin = gbox->ymin = gbox->zmin = gbox->mmin = FLT_MAX;
	gbox->xmax = gbox->ymax = gbox->zmax = gbox->mmax = -1 * FLT_MAX;

	for (i = 2; i < curve->points->npoints; i += 2) {
		getPoint4d_p(curve->points, i - 2, &p1);
		getPoint4d_p(curve->points, i - 1, &p2);
		getPoint4d_p(curve->points, i, &p3);

		if (lw_arc_calculate_gbox_cartesian(&p1, &p2, &p3, &tmp) == LW_FAILURE)
			continue;

		gbox_merge(&tmp, gbox);
	}

	return LW_SUCCESS;
}

static int lwpoint_calculate_gbox_cartesian(LWPOINT *point, GBOX *gbox) {
	if (!point)
		return LW_FAILURE;
	return ptarray_calculate_gbox_cartesian(point->point, gbox);
}

static int lwline_calculate_gbox_cartesian(LWLINE *line, GBOX *gbox) {
	if (!line)
		return LW_FAILURE;
	return ptarray_calculate_gbox_cartesian(line->points, gbox);
}

static int lwtriangle_calculate_gbox_cartesian(LWTRIANGLE *triangle, GBOX *gbox) {
	if (!triangle)
		return LW_FAILURE;
	return ptarray_calculate_gbox_cartesian(triangle->points, gbox);
}

static int lwpoly_calculate_gbox_cartesian(LWPOLY *poly, GBOX *gbox) {
	if (!poly)
		return LW_FAILURE;
	if (poly->nrings == 0)
		return LW_FAILURE;
	/* Just need to check outer ring */
	return ptarray_calculate_gbox_cartesian(poly->rings[0], gbox);
}

static int lwcollection_calculate_gbox_cartesian(LWCOLLECTION *coll, GBOX *gbox) {
	GBOX subbox = {0};
	uint32_t i;
	int result = LW_FAILURE;
	int first = LW_TRUE;
	assert(coll);
	if ((coll->ngeoms == 0) || !gbox)
		return LW_FAILURE;

	subbox.flags = coll->flags;

	for (i = 0; i < coll->ngeoms; i++) {
		if (lwgeom_calculate_gbox_cartesian((LWGEOM *)(coll->geoms[i]), &subbox) == LW_SUCCESS) {
			/* Keep a copy of the sub-bounding box for later
			if ( coll->geoms[i]->bbox )
			    lwfree(coll->geoms[i]->bbox);
			coll->geoms[i]->bbox = gbox_copy(&subbox); */
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

int lwgeom_calculate_gbox_cartesian(const LWGEOM *lwgeom, GBOX *gbox) {
	if (!lwgeom)
		return LW_FAILURE;

	switch (lwgeom->type) {
	case POINTTYPE:
		return lwpoint_calculate_gbox_cartesian((LWPOINT *)lwgeom, gbox);
	case LINETYPE:
		return lwline_calculate_gbox_cartesian((LWLINE *)lwgeom, gbox);
	case POLYGONTYPE:
		return lwpoly_calculate_gbox_cartesian((LWPOLY *)lwgeom, gbox);
	case CIRCSTRINGTYPE:
		return lwcircstring_calculate_gbox_cartesian((LWCIRCSTRING *)lwgeom, gbox);
	case TRIANGLETYPE:
		return lwtriangle_calculate_gbox_cartesian((LWTRIANGLE *)lwgeom, gbox);
	case COMPOUNDTYPE:
	case CURVEPOLYTYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTICURVETYPE:
	case MULTIPOLYGONTYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return lwcollection_calculate_gbox_cartesian((LWCOLLECTION *)lwgeom, gbox);
		// Need to do with postgis
	}
	/* Never get here, please. */
	lwerror("unsupported type (%d) - %s", lwgeom->type, lwtype_name(lwgeom->type));
	return LW_FAILURE;
}

void gbox_float_round(GBOX *gbox) {
	gbox->xmin = next_float_down(gbox->xmin);
	gbox->xmax = next_float_up(gbox->xmax);

	gbox->ymin = next_float_down(gbox->ymin);
	gbox->ymax = next_float_up(gbox->ymax);

	if (FLAGS_GET_M(gbox->flags)) {
		gbox->mmin = next_float_down(gbox->mmin);
		gbox->mmax = next_float_up(gbox->mmax);
	}

	if (FLAGS_GET_Z(gbox->flags)) {
		gbox->zmin = next_float_down(gbox->zmin);
		gbox->zmax = next_float_up(gbox->zmax);
	}
}

} // namespace duckdb
