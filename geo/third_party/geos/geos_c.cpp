/************************************************************************
 *
 *
 * C-Wrapper for GEOS library
 *
 * Copyright (C) 2010 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 * Author: Sandro Santilli <strk@kbt.io>
 *
 ***********************************************************************/

#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/index/strtree/STRtree.hpp>
#include <geos/util/Interrupt.hpp>
#include <new>
#include <stdexcept>

// Some extra magic to make type declarations in geos_c.h work - for cross-checking of types in header.
#define GEOSGeometry      geos::geom::Geometry
#define GEOSCoordSequence geos::geom::CoordinateSequence
#define GEOSSTRtree       geos::index::strtree::STRtree
typedef struct GEOSBufParams_t GEOSBufferParams;

#include "geos_c.hpp"

// import the most frequently used definitions globally
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;

// ## GLOBALS ################################################

// NOTE: SRID will have to be changed after geometry creation
GEOSContextHandle_t handle = NULL;

extern "C" {

void initGEOS(GEOSMessageHandler nf, GEOSMessageHandler ef) {
	if (!handle) {
		handle = initGEOS_r(nf, ef);
	} else {
		GEOSContext_setNoticeHandler_r(handle, nf);
		GEOSContext_setErrorHandler_r(handle, ef);
	}

	geos::util::Interrupt::cancel();
}

// Return postgis geometry type index
int GEOSGeomTypeId(const Geometry *g) {
	return GEOSGeomTypeId_r(handle, g);
}

int GEOSGetSRID(const Geometry *g) {
	return GEOSGetSRID_r(handle, g);
}

void GEOSSetSRID(Geometry *g, int srid) {
	return GEOSSetSRID_r(handle, g, srid);
}

char GEOSHasZ(const Geometry *g) {
	return GEOSHasZ_r(handle, g);
}

// returns -1 on error and 1 for non-multi geometries
int GEOSGetNumGeometries(const Geometry *g) {
	return GEOSGetNumGeometries_r(handle, g);
}

/*
 * Call only on GEOMETRYCOLLECTION or MULTI*.
 * Return a pointer to the internal Geometry.
 */
const Geometry *GEOSGetGeometryN(const Geometry *g, int n) {
	return GEOSGetGeometryN_r(handle, g, n);
}

char GEOSisEmpty(const Geometry *g) {
	return GEOSisEmpty_r(handle, g);
}

char GEOSisRing(const Geometry *g) {
	return GEOSisRing_r(handle, g);
}

int GEOSCoordSeq_getXY(const CoordinateSequence *s, unsigned int idx, double *x, double *y) {
	return GEOSCoordSeq_getXY_r(handle, s, idx, x, y);
}

int GEOSCoordSeq_getXYZ(const CoordinateSequence *s, unsigned int idx, double *x, double *y, double *z) {
	return GEOSCoordSeq_getXYZ_r(handle, s, idx, x, y, z);
}

int GEOSGetNumInteriorRings(const Geometry *g) {
	return GEOSGetNumInteriorRings_r(handle, g);
}

/*
 * Call only on polygon
 * Return a pointer to internal storage, do not destroy it.
 */
const Geometry *GEOSGetInteriorRingN(const Geometry *g, int n) {
	return GEOSGetInteriorRingN_r(handle, g, n);
}

/*
 * Call only on polygon
 * Return a copy of the internal Geometry.
 */
const Geometry *GEOSGetExteriorRing(const Geometry *g) {
	return GEOSGetExteriorRing_r(handle, g);
}

int GEOSCoordSeq_getSize(const CoordinateSequence *s, unsigned int *size) {
	return GEOSCoordSeq_getSize_r(handle, s, size);
}

int GEOSCoordSeq_getDimensions(const CoordinateSequence *s, unsigned int *dims) {
	return GEOSCoordSeq_getDimensions_r(handle, s, dims);
}

const CoordinateSequence *GEOSGeom_getCoordSeq(const Geometry *g) {
	return GEOSGeom_getCoordSeq_r(handle, g);
}

CoordinateSequence *GEOSCoordSeq_create(unsigned int size, unsigned int dims) {
	return GEOSCoordSeq_create_r(handle, size, dims);
}

void GEOSCoordSeq_destroy(CoordinateSequence *s) {
	return GEOSCoordSeq_destroy_r(handle, s);
}

int GEOSCoordSeq_setOrdinate(CoordinateSequence *s, unsigned int idx, unsigned int dim, double val) {
	return GEOSCoordSeq_setOrdinate_r(handle, s, idx, dim, val);
}

int GEOSCoordSeq_setZ(CoordinateSequence *s, unsigned int idx, double val) {
	return GEOSCoordSeq_setOrdinate(s, idx, 2, val);
}

int GEOSCoordSeq_setXY(CoordinateSequence *s, unsigned int idx, double x, double y) {
	return GEOSCoordSeq_setXY_r(handle, s, idx, x, y);
}

int GEOSCoordSeq_setXYZ(CoordinateSequence *s, unsigned int idx, double x, double y, double z) {
	return GEOSCoordSeq_setXYZ_r(handle, s, idx, x, y, z);
}

Geometry *GEOSGeom_createPoint(CoordinateSequence *cs) {
	return GEOSGeom_createPoint_r(handle, cs);
}

Geometry *GEOSGeom_createPointFromXY(double x, double y) {
	return GEOSGeom_createPointFromXY_r(handle, x, y);
}

Geometry *GEOSGeom_createLinearRing(CoordinateSequence *cs) {
	return GEOSGeom_createLinearRing_r(handle, cs);
}

Geometry *GEOSGeom_createLineString(CoordinateSequence *cs) {
	return GEOSGeom_createLineString_r(handle, cs);
}

Geometry *GEOSGeom_createPolygon(Geometry *shell, Geometry **holes, unsigned int nholes) {
	return GEOSGeom_createPolygon_r(handle, shell, holes, nholes);
}

Geometry *GEOSGeom_createCollection(int type, Geometry **geoms, unsigned int ngeoms) {
	return GEOSGeom_createCollection_r(handle, type, geoms, ngeoms);
}

geos::geom::Geometry *GEOSGeom_createEmptyPolygon() {
	return GEOSGeom_createEmptyPolygon_r(handle);
}

Geometry *GEOSUnion(const Geometry *g1, const Geometry *g2) {
	return GEOSUnion_r(handle, g1, g2);
}

Geometry *GEOSUnaryUnion(const Geometry *g) {
	return GEOSUnaryUnion_r(handle, g);
}

//-------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------

void GEOSGeom_destroy(Geometry *a) {
	return GEOSGeom_destroy_r(handle, a);
}

//-------------------------------------------------------------------
// GEOS functions that return geometries
//-------------------------------------------------------------------

Geometry *GEOSDifference(const Geometry *g1, const Geometry *g2) {
	return GEOSDifference_r(handle, g1, g2);
}

Geometry *GEOSDifferencePrec(const Geometry *g1, const Geometry *g2, double gridSize) {
	return GEOSDifferencePrec_r(handle, g1, g2, gridSize);
}

Geometry *GEOSBoundary(const Geometry *g) {
	return GEOSBoundary_r(handle, g);
}

Geometry *GEOSConvexHull(const Geometry *g) {
	return GEOSConvexHull_r(handle, g);
}

Geometry *GEOSIntersection(const Geometry *g1, const Geometry *g2) {
	return GEOSIntersection_r(handle, g1, g2);
}

Geometry *GEOSIntersectionPrec(const Geometry *g1, const Geometry *g2, double gridSize) {
	return GEOSIntersectionPrec_r(handle, g1, g2, gridSize);
}

Geometry *GEOSGetCentroid(const Geometry *g) {
	return GEOSGetCentroid_r(handle, g);
}

//-----------------------------------------------------------------
// Prepared Geometry
//-----------------------------------------------------------------

GEOSBufferParams *GEOSBufferParams_create() {
	return GEOSBufferParams_create_r(handle);
}

void GEOSBufferParams_destroy(GEOSBufferParams *p) {
	return GEOSBufferParams_destroy_r(handle, p);
}

int GEOSBufferParams_setEndCapStyle(GEOSBufferParams *p, int style) {
	return GEOSBufferParams_setEndCapStyle_r(handle, p, style);
}

int GEOSBufferParams_setJoinStyle(GEOSBufferParams *p, int joinStyle) {
	return GEOSBufferParams_setJoinStyle_r(handle, p, joinStyle);
}

int GEOSBufferParams_setMitreLimit(GEOSBufferParams *p, double l) {
	return GEOSBufferParams_setMitreLimit_r(handle, p, l);
}

int GEOSBufferParams_setQuadrantSegments(GEOSBufferParams *p, int joinStyle) {
	return GEOSBufferParams_setQuadrantSegments_r(handle, p, joinStyle);
}

int GEOSBufferParams_setSingleSided(GEOSBufferParams *p, int singleSided) {
	return GEOSBufferParams_setSingleSided_r(handle, p, singleSided);
}

Geometry *GEOSBufferWithParams(const Geometry *g, const GEOSBufferParams *p, double w) {
	return GEOSBufferWithParams_r(handle, g, p, w);
}

GEOSSTRtree *GEOSSTRtree_create(std::size_t nodeCapacity) {
	return GEOSSTRtree_create_r(handle, nodeCapacity);
}

void GEOSSTRtree_insert(GEOSSTRtree *tree, const geos::geom::Geometry *g, void *item) {
	GEOSSTRtree_insert_r(handle, tree, g, item);
}

void GEOSSTRtree_query(GEOSSTRtree *tree, const geos::geom::Geometry *g, GEOSQueryCallback cb, void *userdata) {
	GEOSSTRtree_query_r(handle, tree, g, cb, userdata);
}

void GEOSSTRtree_destroy(GEOSSTRtree *tree) {
	GEOSSTRtree_destroy_r(handle, tree);
}

//-----------------------------------------------------------------
// general purpose
//-----------------------------------------------------------------

// call g1->contains(g2)
// returns 0 = false
//         1 = true
//         2 = error was trapped
char GEOSContains(const Geometry *g1, const Geometry *g2) {
	return GEOSContains_r(handle, g1, g2);
}

char GEOSEquals(const Geometry *g1, const Geometry *g2) {
	return GEOSEquals_r(handle, g1, g2);
}

/****************************************************************
 ** relate()-related functions
 ** return 0 = false, 1 = true, 2 = error occured
 **
 */
char GEOSDisjoint(const Geometry *g1, const Geometry *g2) {
	return GEOSDisjoint_r(handle, g1, g2);
}

char GEOSTouches(const Geometry *g1, const Geometry *g2) {
	return GEOSTouches_r(handle, g1, g2);
}

char GEOSIntersects(const Geometry *g1, const Geometry *g2) {
	return GEOSIntersects_r(handle, g1, g2);
}

//-------------------------------------------------------------------
// low-level relate functions
//------------------------------------------------------------------

char GEOSRelatePattern(const Geometry *g1, const Geometry *g2, const char *pat) {
	return GEOSRelatePattern_r(handle, g1, g2, pat);
}

} /* extern "C" */
