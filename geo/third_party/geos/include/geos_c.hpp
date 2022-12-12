/************************************************************************
 *
 * C-Wrapper for GEOS library
 *
 * Copyright (C) 2010 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 ***********************************************************************/

#ifndef GEOS_C_H_INCLUDED
#define GEOS_C_H_INCLUDED

#ifndef __cplusplus
#include <stddef.h> /* for size_t definition */
#else
#include <cstddef>
using std::size_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================== */
/* Version */
/* ====================================================================== */

/** \cond */

#ifndef GEOS_VERSION_MAJOR
#define GEOS_VERSION_MAJOR @VERSION_MAJOR @
#endif
#ifndef GEOS_VERSION_MINOR
#define GEOS_VERSION_MINOR @VERSION_MINOR @
#endif
#ifndef GEOS_VERSION_PATCH
#define GEOS_VERSION_PATCH @VERSION_PATCH @
#endif
#ifndef GEOS_VERSION
#define GEOS_VERSION "@VERSION@"
#endif
#ifndef GEOS_JTS_PORT
#define GEOS_JTS_PORT "@JTS_PORT@"
#endif

#define GEOS_CAPI_VERSION_MAJOR @CAPI_VERSION_MAJOR @
#define GEOS_CAPI_VERSION_MINOR @CAPI_VERSION_MINOR @
#define GEOS_CAPI_VERSION_PATCH @CAPI_VERSION_PATCH @
#define GEOS_CAPI_VERSION       "@VERSION@-CAPI-@CAPI_VERSION@"

#define GEOS_CAPI_FIRST_INTERFACE GEOS_CAPI_VERSION_MAJOR
#define GEOS_CAPI_LAST_INTERFACE  (GEOS_CAPI_VERSION_MAJOR + GEOS_CAPI_VERSION_MINOR)

/** \endcond */

#include "geos/export.hpp"

/**
 * Type returned by GEOS_init_r(), for use in multi-threaded
 * applications.
 *
 * There should be only one GEOSContextHandle_t per thread.
 */
typedef struct GEOSContextHandle_HS *GEOSContextHandle_t;

/**
 * Callback function for passing GEOS error messages to parent process.
 *
 * Set the GEOSMessageHandler for error and notice messages in \ref initGEOS
 * for single-threaded programs, or using \ref initGEOS_r for threaded
 * programs
 *
 * \param fmt the message format template
 */
typedef void (*GEOSMessageHandler)(const char *fmt, ...);

/**
 * A GEOS message handler function.
 *
 * \param message the message contents
 * \param userdata the user data pointer that was passed to GEOS when
 * registering this message handler.
 *
 * \see GEOSContext_setErrorMessageHandler
 * \see GEOSContext_setNoticeMessageHandler
 */
typedef void (*GEOSMessageHandler_r)(const char *message, void *userdata);

/*
 * When we're included by geos_c.cpp, these types are #defined to the
 * C++ definitions via preprocessor. We don't touch them to allow the
 * compiler to cross-check the declarations. However, for all "normal"
 * C-API users, we need to define these types as "opaque" struct pointers, as
 * those clients don't have access to the original C++ headers, by design.
 */
#ifndef GEOSGeometry

/**
 * Geometry generic type. Geometry can be a point, linestring, polygon,
 * multipoint, multilinestring, multipolygon, or geometrycollection.
 * Geometry type can be read with \ref GEOSGeomTypeId. Most functions
 * in GEOS either have GEOSGeometry* as a parameter or a return type.
 * \see GEOSGeom_createPoint
 * \see GEOSGeom_createLineString
 * \see GEOSGeom_createPolygon
 * \see GEOSGeom_createCollection
 * \see GEOSGeom_destroy
 */
typedef struct GEOSGeom_t GEOSGeometry;

/**
 * Coordinate sequence.
 * \see GEOSCoordSeq_create()
 * \see GEOSCoordSeq_destroy()
 */
typedef struct GEOSCoordSeq_t GEOSCoordSequence;

#endif

/** \cond */

/*
 * These are compatibility definitions for source compatibility
 * with GEOS 2.X clients relying on that type.
 */
typedef GEOSGeometry *GEOSGeom;
typedef GEOSCoordSequence *GEOSCoordSeq;

/**
 * Geometry type number, used by functions returning or
 * consuming geometry types.
 *
 * \see GEOSGeomType
 * \see GEOSGeomTypeId
 */
enum GEOSGeomTypes {
	/** Point */
	GEOS_POINT,
	/** Linestring */
	GEOS_LINESTRING,
	/** Linear ring, used within polygons */
	GEOS_LINEARRING,
	/** Polygon */
	GEOS_POLYGON,
	/** Multipoint, a homogeneous collection of points */
	GEOS_MULTIPOINT,
	/** Multilinestring, a homogeneous collection of linestrings */
	GEOS_MULTILINESTRING,
	/** Multipolygon, a homogeneous collection of polygons */
	GEOS_MULTIPOLYGON,
	/** Geometry collection, a heterogeneous collection of geometry */
	GEOS_GEOMETRYCOLLECTION
};

/**
 * Set the notice handler callback function for run-time notice messages.
 * \param extHandle the context returned by \ref GEOS_init_r.
 * \param nf the handler callback
 * \return the previously configured message handler or NULL if no message handler was configured
 */
extern GEOSMessageHandler GEOS_DLL GEOSContext_setNoticeHandler_r(GEOSContextHandle_t extHandle, GEOSMessageHandler nf);

/**
 * Set the notice handler callback function for run-time error messages.
 * \param extHandle the GEOS context from \ref GEOS_init_r
 * \param ef the handler callback
 * \return the previously configured message handler or NULL if no message handler was configured
 */
extern GEOSMessageHandler GEOS_DLL GEOSContext_setErrorHandler_r(GEOSContextHandle_t extHandle, GEOSMessageHandler ef);

/**  */

/* ========== Initialization and Cleanup ========== */

/**
 * Initialize a context for this thread. Pass this context into
 * your other calls of `*_r` functions.
 * \return a GEOS context for this thread
 */
extern GEOSContextHandle_t GEOS_DLL GEOS_init_r(void);

/**
 * \deprecated in 3.5.0. Use GEOS_init_r() and set the message handlers using
 * GEOSContext_setNoticeHandler_r() and/or GEOSContext_setErrorHandler_r()
 */
extern GEOSContextHandle_t GEOS_DLL initGEOS_r(GEOSMessageHandler notice_function, GEOSMessageHandler error_function);

/* ========== Geometry info ========== */

/** \see GEOSGeomTypeId */
extern int GEOS_DLL GEOSGeomTypeId_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSGetSRID */
extern int GEOS_DLL GEOSGetSRID_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSSetSRID */
extern void GEOS_DLL GEOSSetSRID_r(GEOSContextHandle_t handle, GEOSGeometry *g, int SRID);

/** \see GEOSGetNumGeometries */
extern int GEOS_DLL GEOSGetNumGeometries_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSGetGeometryN */
extern const GEOSGeometry GEOS_DLL *GEOSGetGeometryN_r(GEOSContextHandle_t handle, const GEOSGeometry *g, int n);

/** \see GEOSGeom_getCoordSeq */
extern const GEOSCoordSequence GEOS_DLL *GEOSGeom_getCoordSeq_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSGetNumInteriorRings */
extern int GEOS_DLL GEOSGetNumInteriorRings_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSGetExteriorRing */
extern const GEOSGeometry GEOS_DLL *GEOSGetExteriorRing_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSGetInteriorRingN */
extern const GEOSGeometry GEOS_DLL *GEOSGetInteriorRingN_r(GEOSContextHandle_t handle, const GEOSGeometry *g, int n);

/* ========= Unary predicate ========= */

/** \see GEOSisEmpty */
extern char GEOS_DLL GEOSisEmpty_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSisRing */
extern char GEOS_DLL GEOSisRing_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/** \see GEOSHasZ */
extern char GEOS_DLL GEOSHasZ_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/* ========== Coordinate Sequence functions ========== */

/** \see GEOSCoordSeq_create */
extern GEOSCoordSequence GEOS_DLL *GEOSCoordSeq_create_r(GEOSContextHandle_t handle, unsigned int size,
                                                         unsigned int dims);

/** \see GEOSCoordSeq_setXY */
extern int GEOS_DLL GEOSCoordSeq_setXY_r(GEOSContextHandle_t handle, GEOSCoordSequence *s, unsigned int idx, double x,
                                         double y);

/** \see GEOSCoordSeq_setXYZ */
extern int GEOS_DLL GEOSCoordSeq_setXYZ_r(GEOSContextHandle_t handle, GEOSCoordSequence *s, unsigned int idx, double x,
                                          double y, double z);

/** \see GEOSCoordSeq_setOrdinate */
extern int GEOS_DLL GEOSCoordSeq_setOrdinate_r(GEOSContextHandle_t handle, GEOSCoordSequence *s, unsigned int idx,
                                               unsigned int dim, double val);

/** \see GEOSCoordSeq_getSize */
extern int GEOS_DLL GEOSCoordSeq_getSize_r(GEOSContextHandle_t handle, const GEOSCoordSequence *s, unsigned int *size);

/** \see GEOSCoordSeq_getDimensions */
extern int GEOS_DLL GEOSCoordSeq_getDimensions_r(GEOSContextHandle_t handle, const GEOSCoordSequence *s,
                                                 unsigned int *dims);

/** \see GEOSCoordSeq_getXY */
extern int GEOS_DLL GEOSCoordSeq_getXY_r(GEOSContextHandle_t handle, const GEOSCoordSequence *s, unsigned int idx,
                                         double *x, double *y);

/** \see GEOSCoordSeq_getXYZ */
extern int GEOS_DLL GEOSCoordSeq_getXYZ_r(GEOSContextHandle_t handle, const GEOSCoordSequence *s, unsigned int idx,
                                          double *x, double *y, double *z);

/* ========= Geometry Constructors ========= */

/** \see GEOSGeom_createPoint */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createPoint_r(GEOSContextHandle_t handle, GEOSCoordSequence *s);

/** \see GEOSGeom_createPointFromXY */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createPointFromXY_r(GEOSContextHandle_t handle, double x, double y);

/** \see GEOSGeom_createCollection */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createCollection_r(GEOSContextHandle_t handle, int type, GEOSGeometry **geoms,
                                                          unsigned int ngeoms);

/** \see GEOSGeom_createEmptyPolygon */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createEmptyPolygon_r(GEOSContextHandle_t handle);

/** \see GEOSGeom_createLinearRing */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLinearRing_r(GEOSContextHandle_t handle, GEOSCoordSequence *s);

/** \see GEOSGeom_createLineString */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLineString_r(GEOSContextHandle_t handle, GEOSCoordSequence *s);

/** \see GEOSGeom_createPolygon */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createPolygon_r(GEOSContextHandle_t handle, GEOSGeometry *shell,
                                                       GEOSGeometry **holes, unsigned int nholes);

/** \see GEOSUnion */
extern GEOSGeometry GEOS_DLL *GEOSUnion_r(GEOSContextHandle_t handle, const GEOSGeometry *g1, const GEOSGeometry *g2);

/** \see GEOSUnaryUnion */
extern GEOSGeometry GEOS_DLL *GEOSUnaryUnion_r(GEOSContextHandle_t handle, const GEOSGeometry *g);

/* ========= Memory management ========= */

/** \see GEOSGeom_destroy */
extern void GEOS_DLL GEOSGeom_destroy_r(GEOSContextHandle_t handle, GEOSGeometry *g);

/*
 * External code to GEOS can define GEOS_USE_ONLY_R_API
 * to strip the non-reentrant API functions from this header,
 * leaving only the "_r" compatible variants.
 */
#ifndef GEOS_USE_ONLY_R_API

/* ========== Initialization, cleanup ================================= */
/** @name Library and Memory Management
 * Functions to initialize and tear down the library,
 * and deallocate memory.
 */
///@{

/**
 * For non-reentrant code, set up an execution contact, and associate
 * \ref GEOSMessageHandler functions with it, to pass error and notice
 * messages back to the calling application.
 * <pre>
 * typedef void (*GEOSMessageHandler)(const char *fmt, ...);
 * </pre>
 *
 * \param notice_function Handle notice messages
 * \param error_function Handle error messages
 */
extern void GEOS_DLL initGEOS(GEOSMessageHandler notice_function, GEOSMessageHandler error_function);

/* ========= Coordinate Sequence functions ========= */
/** @name Coordinate Sequences
 * A GEOSCoordSequence is an ordered list of coordinates.
 * Coordinates are 2 (XY) or 3 (XYZ) dimensional.
 */
///@{

/**
 * Create a coordinate sequence.
 * \param size number of coordinates in the sequence
 * \param dims dimensionality of the coordinates (2 or 3)
 * \return the sequence or NULL on exception
 */
extern GEOSCoordSequence GEOS_DLL *GEOSCoordSeq_create(unsigned int size, unsigned int dims);

/**
 * Set Z ordinate values in a coordinate sequence.
 * \param s the coordinate sequence
 * \param idx the index of the coordinate to alter, zero based
 * \param val the value to set the ordinate to
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_setZ(GEOSCoordSequence *s, unsigned int idx, double val);

/**
 * Set X and Y ordinate values in a coordinate sequence simultaneously.
 * \param s the coordinate sequence
 * \param idx the index of the coordinate to alter, zero based
 * \param x the value to set the X ordinate to
 * \param y the value to set the Y ordinate to
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_setXY(GEOSCoordSequence *s, unsigned int idx, double x, double y);

/**
 * Set X, Y and Z ordinate values in a coordinate sequence simultaneously.
 * \param s the coordinate sequence
 * \param idx the index of the coordinate to alter, zero based
 * \param x the value to set the X ordinate to
 * \param y the value to set the Y ordinate to
 * \param z the value to set the Z ordinate to
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_setXYZ(GEOSCoordSequence *s, unsigned int idx, double x, double y, double z);

/**
 * Set Nth ordinate value in a coordinate sequence.
 * \param s the coordinate sequence
 * \param idx the index of the coordinate to alter, zero based
 * \param dim the dimension number of the ordinate to alter, zero based
 * \param val the value to set the ordinate to
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_setOrdinate(GEOSCoordSequence *s, unsigned int idx, unsigned int dim, double val);

/* ========= Geometry Constructors ========= */
/** @name Geometry Constructors
 * Functions for creating and destroying geometries.
 * Created geometries must be freed with GEOSGeom_destroy().
 */
///@{

/**
 * Creates a point geometry from a coordinate sequence.
 * \param s Input coordinate sequence, ownership passes to the geometry
 * \return A newly allocated point geometry. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createPoint(GEOSCoordSequence *s);

/**
 * Creates a point geometry from a pair of coordinates.
 * \param x The X coordinate
 * \param y The Y coordinate
 * \return A newly allocated point geometry. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createPointFromXY(double x, double y);

/**
 * Creates a linear ring geometry, for use in a polygon.
 * \param s Input coordinate sequence, ownership passes to the geometry
 * \return A newly allocated linear ring geometry. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLinearRing(GEOSCoordSequence *s);

/**
 * Creates a linestring geometry.
 * \param s Input coordinate sequence, ownership passes to the geometry
 * \return A newly allocated linestring geometry. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLineString(GEOSCoordSequence *s);

/**
 * Creates an empty polygon geometry.
 * \return A newly allocated empty polygon geometry. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createEmptyPolygon(void);

/**
 * Creates a polygon geometry from line ring geometries.
 * \param shell A linear ring that is the exterior ring of the polygon.
 * \param holes An array of linear rings that are the holes.
 * \param nholes The number of rings in the holes array.
 * \return A newly allocated geometry. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 * \note The holes argument is an array of GEOSGeometry* objects.
 *       The caller **retains ownership** of the containing array,
 *       but the ownership of the pointed-to objects is transferred
 *       to the returned \ref GEOSGeometry.
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createPolygon(GEOSGeometry *shell, GEOSGeometry **holes, unsigned int nholes);

/**
 * Create a geometry collection.
 * \param type The geometry type, enumerated by \ref GEOSGeomTypes
 * \param geoms A list of geometries that will form the collection
 * \param ngeoms The number of geometries in the geoms list
 * \return A newly allocated geometry collection. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 * \note The holes argument is an array of GEOSGeometry* objects.
 *       The caller **retains ownership** of the containing array,
 *       but the ownership of the pointed-to objects is transferred
 *       to the returned \ref GEOSGeometry.
 */
extern GEOSGeometry GEOS_DLL *GEOSGeom_createCollection(int type, GEOSGeometry **geoms, unsigned int ngeoms);

/**
 * Release the memory associated with a geometry.
 * \param g The geometry to be destroyed.
 */
extern void GEOS_DLL GEOSGeom_destroy(GEOSGeometry *g);

///@}

/* ========== Geometry info ========== */
/** @name Geometry Accessors
 * Functions to provide information about geometries.
 */
///@{

/**
 * Returns the \ref GEOSGeomTypeId number for this geometry.
 * \param g Input geometry
 * \return The geometry type number, or -1 on exception.
 */
extern int GEOS_DLL GEOSGeomTypeId(const GEOSGeometry *g);

/**
 * Returns the "spatial reference id" (SRID) for this geometry.
 * \param g Input geometry
 * \return SRID number or 0 if unknown / not set.
 */
extern int GEOS_DLL GEOSGetSRID(const GEOSGeometry *g);

/**
 * Set the "spatial reference id" (SRID) for this geometry.
 * \param g Input geometry
 * \param SRID SRID number or 0 for unknown SRID.
 */
extern void GEOS_DLL GEOSSetSRID(GEOSGeometry *g, int SRID);

/**
 * Tests whether the input geometry has z coordinates.
 * \param g The geometry to test
 * \return 1 on true, 0 on false, 2 on exception
 */
extern char GEOS_DLL GEOSHasZ(const GEOSGeometry *g);

/**
 * Tests whether the input geometry is a ring. Rings are
 * linestrings, without self-intersections,
 * with start and end point being identical.
 * \param g The geometry to test
 * \return 1 on true, 0 on false, 2 on exception
 */
extern char GEOS_DLL GEOSisRing(const GEOSGeometry *g);

/**
 * Returns the number of sub-geometries immediately under a
 * multi-geometry or collection or 1 for a simple geometry.
 * For nested collections, remember to check if returned
 * sub-geometries are **themselves** also collections.
 * \param g Input geometry
 * \return Number of direct children in this collection
 * \warning For GEOS < 3.2 this function may crash when fed simple geometries
 */
extern int GEOS_DLL GEOSGetNumGeometries(const GEOSGeometry *g);

/**
* Returns the specified sub-geometry of a collection. For
* a simple geometry, returns a pointer to the input.
* Returned object is a pointer to internal storage:
* it must NOT be destroyed directly.
* \param g Input geometry
* \param n Sub-geometry index, zero-base
* \return A const \ref GEOSGeometry, do not free!
          It will be freed when the parent is freed.
          Returns NULL on exception.
* \note Up to GEOS 3.2.0 the input geometry must be a Collection, in
*       later versions it doesn't matter (getGeometryN(0) for a single will
*       return the input).
*/
extern const GEOSGeometry GEOS_DLL *GEOSGetGeometryN(const GEOSGeometry *g, int n);

/**
 * Return the coordinate sequence underlying the
 * given geometry (Must be a LineString, LinearRing or Point).
 * Do not directly free the coordinate sequence, it is owned by
 * the parent geometry.
 * \param g Input geometry
 * \return Coordinate sequence or NULL on exception.
 */
extern const GEOSCoordSequence GEOS_DLL *GEOSGeom_getCoordSeq(const GEOSGeometry *g);

/**
 * Tests whether the input geometry is empty. If the geometry or any
 * component is non-empty, the geometry is non-empty. An empty geometry
 * has no boundary or interior.
 * \param g The geometry to test
 * \return 1 on true, 0 on false, 2 on exception
 */
extern char GEOS_DLL GEOSisEmpty(const GEOSGeometry *g);

/**
 * Get size info from a coordinate sequence.
 * \param[in] s the coordinate sequence
 * \param[out] size pointer where size value will be placed
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_getSize(const GEOSCoordSequence *s, unsigned int *size);

/**
 * Get dimension info from a coordinate sequence.
 * \param[in] s the coordinate sequence
 * \param[out] dims pointer where dimension value will be placed
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_getDimensions(const GEOSCoordSequence *s, unsigned int *dims);

/**
 * Read X and Y ordinate values from a coordinate sequence.
 * \param s the coordinate sequence
 * \param idx the index of the coordinate to alter, zero based
 * \param x pointer where ordinate X value will be placed
 * \param y pointer where ordinate Y value will be placed
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_getXY(const GEOSCoordSequence *s, unsigned int idx, double *x, double *y);

/**
 * Read X and Y ordinate values from a coordinate sequence.
 * \param s the coordinate sequence
 * \param idx the index of the coordinate to alter, zero based
 * \param x pointer where ordinate X value will be placed
 * \param y pointer where ordinate Y value will be placed
 * \param z pointer where ordinate Z value will be placed
 * \return 0 on exception
 */
extern int GEOS_DLL GEOSCoordSeq_getXYZ(const GEOSCoordSequence *s, unsigned int idx, double *x, double *y, double *z);

/**
 * Returns the number of interior rings, for a Polygon input, or
 * an exception otherwise.
 * \param g Input Polygon geometry
 * \return Number of interior rings, -1 on exception
 */
extern int GEOS_DLL GEOSGetNumInteriorRings(const GEOSGeometry *g);

/**
 * Get the external ring of a Polygon.
 * \note Returned object is a pointer to internal storage:
 *       it must NOT be destroyed directly.
 * \param g Input Polygon geometry
 * \return LinearRing geometry. Owned by parent geometry, do not free. NULL on exception.
 */
extern const GEOSGeometry GEOS_DLL *GEOSGetExteriorRing(const GEOSGeometry *g);

/**
 * Returns the N'th ring for a Polygon input.
 * \note Returned object is a pointer to internal storage:
 *       it must NOT be destroyed directly.
 * \param g Input Polygon geometry
 * \param n Index of the desired ring
 * \return LinearRing geometry. Owned by parent geometry, do not free. NULL on exception.
 */
extern const GEOSGeometry GEOS_DLL *GEOSGetInteriorRingN(const GEOSGeometry *g, int n);

///@}

/* ==================================================================================== */
/** @name Geometry Mutators
 * Functions to change geometry information or content.
 */
///@{

/**
 * Returns the union of two geometries A and B: the set of points
 * that fall in A **or** within B.
 * \param ga geometry A
 * \param gb geometry B
 * \return A newly allocated geometry of the union. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 * \see geos::operation::overlayng::OverlayNG
 */
extern GEOSGeometry GEOS_DLL *GEOSUnion(const GEOSGeometry *ga, const GEOSGeometry *gb);

/**
 * Returns the union of all components of a single geometry. Usually
 * used to convert a collection into the smallest set of polygons
 * that cover the same area.
 * \param g The input geometry
 * \return A newly allocated geometry of the union. NULL on exception.
 * Caller is responsible for freeing with GEOSGeom_destroy().
 * \see geos::operation::overlayng::OverlayNG
 */
extern GEOSGeometry GEOS_DLL *GEOSUnaryUnion(const GEOSGeometry *g);

#endif /* #ifndef GEOS_USE_ONLY_R_API */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* #ifndef GEOS_C_H_INCLUDED */