#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

#define CHECK_LWGEOM_ZM 1

LWCOLLECTION *lwcollection_construct(uint8_t type, int32_t srid, GBOX *bbox, uint32_t ngeoms, LWGEOM **geoms) {
	LWCOLLECTION *ret;
	int hasz, hasm;
#ifdef CHECK_LWGEOM_ZM
	char zm;
	uint32_t i;
#endif

	if (!lwtype_is_collection(type))
		// lwerror("Non-collection type specified in collection constructor!");
		return nullptr;

	hasz = 0;
	hasm = 0;
	if (ngeoms > 0) {
		hasz = FLAGS_GET_Z(geoms[0]->flags);
		hasm = FLAGS_GET_M(geoms[0]->flags);
#ifdef CHECK_LWGEOM_ZM
		zm = FLAGS_GET_ZM(geoms[0]->flags);

		for (i = 1; i < ngeoms; i++) {
			if (zm != FLAGS_GET_ZM(geoms[i]->flags))
				// lwerror("lwcollection_construct: mixed dimension geometries: %d/%d", zm,
				// FLAGS_GET_ZM(geoms[i]->flags));
				return nullptr;
		}
#endif
	}

	ret = (LWCOLLECTION *)lwalloc(sizeof(LWCOLLECTION));
	ret->type = type;
	ret->flags = lwflags(hasz, hasm, 0);
	FLAGS_SET_BBOX(ret->flags, bbox ? 1 : 0);
	ret->srid = srid;
	ret->ngeoms = ngeoms;
	ret->maxgeoms = ngeoms;
	ret->geoms = geoms;
	ret->bbox = bbox;

	return ret;
}

LWCOLLECTION *lwcollection_construct_empty(uint8_t type, int32_t srid, char hasz, char hasm) {
	LWCOLLECTION *ret;
	if (!lwtype_is_collection(type)) {
		// lwerror("Non-collection type specified in collection constructor!");
		return NULL;
	}

	ret = (LWCOLLECTION *)lwalloc(sizeof(LWCOLLECTION));
	ret->type = type;
	ret->flags = lwflags(hasz, hasm, 0);
	ret->srid = srid;
	ret->ngeoms = 0;
	ret->maxgeoms = 1; /* Allocate room for sub-members, just in case. */
	ret->geoms = (LWGEOM **)lwalloc(ret->maxgeoms * sizeof(LWGEOM *));
	ret->bbox = NULL;

	return ret;
}

/**
 * Appends geom to the collection managed by col. Does not copy or
 * clone, simply takes a reference on the passed geom.
 */
LWCOLLECTION *lwcollection_add_lwgeom(LWCOLLECTION *col, const LWGEOM *geom) {
	if (!col || !geom)
		return NULL;

	if (!col->geoms && (col->ngeoms || col->maxgeoms)) {
		// lwerror("Collection is in inconsistent state. Null memory but non-zero collection counts.");
		return NULL;
	}

	/* Check type compatibility */
	if (!lwcollection_allows_subtype(col->type, geom->type)) {
		// lwerror("%s cannot contain %s element", lwtype_name(col->type), lwtype_name(geom->type));
		return NULL;
	}

	/* In case this is a truly empty, make some initial space  */
	if (!col->geoms) {
		col->maxgeoms = 2;
		col->ngeoms = 0;
		col->geoms = (LWGEOM **)lwalloc(col->maxgeoms * sizeof(LWGEOM *));
	}

	/* Allocate more space if we need it */
	lwcollection_reserve(col, col->ngeoms + 1);

#if PARANOIA_LEVEL > 1
	/* See http://trac.osgeo.org/postgis/ticket/2933 */
	/* Make sure we don't already have a reference to this geom */
	{
		uint32_t i = 0;
		for (i = 0; i < col->ngeoms; i++) {
			if (col->geoms[i] == geom) {
				// lwerror("%s [%d] found duplicate geometry in collection %p == %p", __FILE__, __LINE__, col->geoms[i],
				// geom);
				return col;
			}
		}
	}
#endif

	col->geoms[col->ngeoms] = (LWGEOM *)geom;
	col->ngeoms++;
	return col;
}

/**
 * Ensure the collection can hold up at least ngeoms
 */
void lwcollection_reserve(LWCOLLECTION *col, uint32_t ngeoms) {
	if (ngeoms <= col->maxgeoms)
		return;

	/* Allocate more space if we need it */
	do {
		col->maxgeoms *= 2;
	} while (col->maxgeoms < ngeoms);
	col->geoms = (LWGEOM **)lwrealloc(col->geoms, sizeof(LWGEOM *) * col->maxgeoms);
}

int lwcollection_allows_subtype(int collectiontype, int subtype) {
	if (collectiontype == COLLECTIONTYPE)
		return LW_TRUE;
	if (collectiontype == MULTIPOINTTYPE && subtype == POINTTYPE)
		return LW_TRUE;
	if (collectiontype == MULTILINETYPE && subtype == LINETYPE)
		return LW_TRUE;
	if (collectiontype == MULTIPOLYGONTYPE && subtype == POLYGONTYPE)
		return LW_TRUE;
	if (collectiontype == COMPOUNDTYPE && (subtype == LINETYPE || subtype == CIRCSTRINGTYPE))
		return LW_TRUE;
	if (collectiontype == CURVEPOLYTYPE &&
	    (subtype == CIRCSTRINGTYPE || subtype == LINETYPE || subtype == COMPOUNDTYPE))
		return LW_TRUE;
	if (collectiontype == MULTICURVETYPE &&
	    (subtype == CIRCSTRINGTYPE || subtype == LINETYPE || subtype == COMPOUNDTYPE))
		return LW_TRUE;
	if (collectiontype == MULTISURFACETYPE && (subtype == POLYGONTYPE || subtype == CURVEPOLYTYPE))
		return LW_TRUE;
	if (collectiontype == POLYHEDRALSURFACETYPE && subtype == POLYGONTYPE)
		return LW_TRUE;
	if (collectiontype == TINTYPE && subtype == TRIANGLETYPE)
		return LW_TRUE;

	/* Must be a bad combination! */
	return LW_FALSE;
}

void lwcollection_free(LWCOLLECTION *col) {
	uint32_t i;
	if (!col)
		return;

	if (col->bbox) {
		lwfree(col->bbox);
	}
	for (i = 0; i < col->ngeoms; i++) {
		if (col->geoms && col->geoms[i])
			lwgeom_free(col->geoms[i]);
	}
	if (col->geoms) {
		lwfree(col->geoms);
	}
	lwfree(col);
}

LWCOLLECTION *lwcollection_force_dims(const LWCOLLECTION *col, int hasz, int hasm, double zval, double mval) {
	LWCOLLECTION *colout;

	/* Return 2D empty */
	if (lwcollection_is_empty(col)) {
		colout = lwcollection_construct_empty(col->type, col->srid, hasz, hasm);
	} else {
		uint32_t i;
		LWGEOM **geoms = NULL;
		geoms = (LWGEOM **)lwalloc(sizeof(LWGEOM *) * col->ngeoms);
		for (i = 0; i < col->ngeoms; i++) {
			geoms[i] = lwgeom_force_dims(col->geoms[i], hasz, hasm, zval, mval);
		}
		colout = lwcollection_construct(col->type, col->srid, NULL, col->ngeoms, geoms);
	}
	return colout;
}

uint32_t lwcollection_count_vertices(LWCOLLECTION *col) {
	uint32_t i = 0;
	uint32_t v = 0; /* vertices */
	assert(col);
	for (i = 0; i < col->ngeoms; i++) {
		v += lwgeom_count_vertices(col->geoms[i]);
	}
	return v;
}

int lwcollection_startpoint(const LWCOLLECTION *col, POINT4D *pt) {
	if (col->ngeoms < 1)
		return LW_FAILURE;

	return lwgeom_startpoint(col->geoms[0], pt);
}

} // namespace duckdb
