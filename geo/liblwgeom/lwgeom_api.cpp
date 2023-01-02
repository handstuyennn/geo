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
 * Copyright 2017 Darafei Praliaskouski <me@komzpa.net>
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <cassert>
#include <cstring>

namespace duckdb {

float next_float_down(double d) {
	float result;
	if (d > (double)FLT_MAX)
		return FLT_MAX;
	if (d <= (double)-FLT_MAX)
		return -FLT_MAX;
	result = d;

	if (((double)result) <= d)
		return result;

	return nextafterf(result, -1 * FLT_MAX);
}

/*
 * Returns the float that's very close to the input, but >=.
 * handles the funny differences in float4 and float8 reps.
 */
float next_float_up(double d) {
	float result;
	if (d >= (double)FLT_MAX)
		return FLT_MAX;
	if (d < (double)-FLT_MAX)
		return -FLT_MAX;
	result = d;

	if (((double)result) >= d)
		return result;

	return nextafterf(result, FLT_MAX);
}

/*
 * Copy a point from the point array into the parameter point
 * z value (if present) is not returned.
 *
 * NOTE: this will modify the point2d pointed to by 'point'.
 */
int getPoint2d_p(const POINTARRAY *pa, uint32_t n, POINT2D *point) {
	if (!pa) {
		lwerror("%s [%d] NULL POINTARRAY input", __FILE__, __LINE__);
		return 0;
	}

	if (n >= pa->npoints) {
		// lwnotice("%s [%d] called with n=%d and npoints=%d", __FILE__, __LINE__, n, pa->npoints);
		return 0;
	}

	/* this does x,y */
	memcpy(point, getPoint_internal(pa, n), sizeof(POINT2D));
	return 1;
}

/*
 * set point N to the given value
 * NOTE that the pointarray can be of any
 * dimension, the appropriate ordinate values
 * will be extracted from it
 *
 */
void ptarray_set_point4d(POINTARRAY *pa, uint32_t n, const POINT4D *p4d) {
	uint8_t *ptr;
	assert(n < pa->npoints);
	ptr = getPoint_internal(pa, n);
	switch (FLAGS_GET_ZM(pa->flags)) {
	case 3:
		memcpy(ptr, p4d, sizeof(POINT4D));
		break;
	case 2:
		memcpy(ptr, p4d, sizeof(POINT3DZ));
		break;
	case 1:
		memcpy(ptr, p4d, sizeof(POINT2D));
		ptr += sizeof(POINT2D);
		memcpy(ptr, &(p4d->m), sizeof(double));
		break;
	case 0:
		memcpy(ptr, p4d, sizeof(POINT2D));
		break;
	}
}

void ptarray_copy_point(POINTARRAY *pa, uint32_t from, uint32_t to) {
	int ndims = FLAGS_NDIMS(pa->flags);
	switch (ndims) {
	case 2: {
		POINT2D *p_from = (POINT2D *)(getPoint_internal(pa, from));
		POINT2D *p_to = (POINT2D *)(getPoint_internal(pa, to));
		*p_to = *p_from;
		return;
	}
	case 3: {
		POINT3D *p_from = (POINT3D *)(getPoint_internal(pa, from));
		POINT3D *p_to = (POINT3D *)(getPoint_internal(pa, to));
		*p_to = *p_from;
		return;
	}
	case 4: {
		POINT4D *p_from = (POINT4D *)(getPoint_internal(pa, from));
		POINT4D *p_to = (POINT4D *)(getPoint_internal(pa, to));
		*p_to = *p_from;
		return;
	}
	default: {
		lwerror("%s: unsupported number of dimensions - %d", __func__, ndims);
		return;
	}
	}
	return;
}

/************************************************************************
 * POINTARRAY support functions
 *
 * TODO: should be moved to ptarray.c probably
 *
 ************************************************************************/

/*
 * Copies a point from the point array into the parameter point
 * will set point's z=NO_Z_VALUE if pa is 2d
 * will set point's m=NO_M_VALUE if pa is 3d or 2d
 *
 * NOTE: point is a real POINT3D *not* a pointer
 */
POINT4D
getPoint4d(const POINTARRAY *pa, uint32_t n) {
	POINT4D result;
	getPoint4d_p(pa, n, &result);
	return result;
}

/*
 * Copies a point from the point array into the parameter point
 * will set point's z=NO_Z_VALUE  if pa is 2d
 * will set point's m=NO_M_VALUE  if pa is 3d or 2d
 *
 * NOTE: this will modify the point4d pointed to by 'point'.
 *
 * @return 0 on error, 1 on success
 */
int getPoint4d_p(const POINTARRAY *pa, uint32_t n, POINT4D *op) {
	uint8_t *ptr;
	int zmflag;

	if (!pa) {
		lwerror("%s [%d] NULL POINTARRAY input", __FILE__, __LINE__);
		return 0;
	}

	if (n >= pa->npoints) {
		// lwnotice("%s [%d] called with n=%d and npoints=%d", __FILE__, __LINE__, n, pa->npoints);
		return 0;
	}

	/* Get a pointer to nth point offset and zmflag */
	ptr = getPoint_internal(pa, n);
	zmflag = FLAGS_GET_ZM(pa->flags);

	switch (zmflag) {
	case 0: /* 2d  */
		memcpy(op, ptr, sizeof(POINT2D));
		op->m = NO_M_VALUE;
		op->z = NO_Z_VALUE;
		break;

	case 3: /* ZM */
		memcpy(op, ptr, sizeof(POINT4D));
		break;

	case 2: /* Z */
		memcpy(op, ptr, sizeof(POINT3DZ));
		op->m = NO_M_VALUE;
		break;

	case 1: /* M */
		memcpy(op, ptr, sizeof(POINT3DM));
		op->m = op->z; /* we use Z as temporary storage */
		op->z = NO_Z_VALUE;
		break;

	default:
		lwerror("Unknown ZM flag ??");
		return 0;
	}
	return 1;
}

} // namespace duckdb
