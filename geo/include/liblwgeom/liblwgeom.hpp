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
 * Copyright 2011 Sandro Santilli <strk@kbt.io>
 * Copyright 2011 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright 2007-2008 Mark Cave-Ayland
 * Copyright 2001-2006 Refractions Research Inc.
 *
 **********************************************************************/

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/postigs_config.hpp"
#include "postgres.hpp"

#include <iostream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

namespace duckdb {

#ifndef _LIBLWGEOM_H
#define _LIBLWGEOM_H 1

// #if POSTGIS_PROJ_VERSION < 49
// /* Use the old (pre-2.2) geodesic functions */
// #undef PROJ_GEODESIC
// #else
// /* Enable new geodesic functions API */
// #define PROJ_GEODESIC
// #endif

// /* For PROJ6 we cache several extra values to avoid calls to proj_get_source_crs
//  * or proj_get_target_crs since those are very costly
//  */
// typedef struct LWPROJ
// {
// 	PJ* pj;
//         /* CRSs are swapped: Used in transformation calls */
// 	uint8_t source_swapped;
// 	uint8_t target_swapped;
//         /* Source crs is geographic: Used in geography calls (source srid == dst srid) */
//         uint8_t source_is_latlong;

//         /* Source ellipsoid parameters */
//         double source_semi_major_metre;
//         double source_semi_minor_metre;
// } LWPROJ;

struct pg_varlena {
	char vl_len_[4]; /* Do not touch this field directly! */
	char vl_dat[1];  /* Data content is here */
};

typedef struct pg_varlena bytea;

/**
 * Return types for functions with status returns.
 */
#define LW_TRUE    1
#define LW_FALSE   0
#define LW_UNKNOWN 2
#define LW_FAILURE 0
#define LW_SUCCESS 1

/**
 * LWTYPE numbers, used internally by PostGIS
 */
#define POINTTYPE             1
#define LINETYPE              2
#define POLYGONTYPE           3
#define MULTIPOINTTYPE        4
#define MULTILINETYPE         5
#define MULTIPOLYGONTYPE      6
#define COLLECTIONTYPE        7
#define CIRCSTRINGTYPE        8
#define COMPOUNDTYPE          9
#define CURVEPOLYTYPE         10
#define MULTICURVETYPE        11
#define MULTISURFACETYPE      12
#define POLYHEDRALSURFACETYPE 13
#define TRIANGLETYPE          14
#define TINTYPE               15

#define NUMTYPES 16

/**
 * Flags applied in EWKB to indicate Z/M dimensions and
 * presence/absence of SRID and bounding boxes
 */
#define WKBZOFFSET  0x80000000
#define WKBMOFFSET  0x40000000
#define WKBSRIDFLAG 0x20000000
#define WKBBBOXFLAG 0x10000000

/**
 * Macros for manipulating the 'flags' byte. A uint8_t used as follows:
 * VVSRGBMZ
 * Version bit, followed by
 * Validty, Solid, ReadOnly, Geodetic, HasBBox, HasM and HasZ flags.
 */
#define LWFLAG_Z        0x01
#define LWFLAG_M        0x02
#define LWFLAG_BBOX     0x04
#define LWFLAG_GEODETIC 0x08
#define LWFLAG_READONLY 0x10
#define LWFLAG_SOLID    0x20

#define FLAGS_GET_Z(flags)        ((flags)&LWFLAG_Z)
#define FLAGS_GET_M(flags)        (((flags)&LWFLAG_M) >> 1)
#define FLAGS_GET_BBOX(flags)     (((flags)&LWFLAG_BBOX) >> 2)
#define FLAGS_GET_GEODETIC(flags) (((flags)&LWFLAG_GEODETIC) >> 3)
#define FLAGS_GET_READONLY(flags) (((flags)&LWFLAG_READONLY) >> 4)
#define FLAGS_GET_SOLID(flags)    (((flags)&LWFLAG_SOLID) >> 5)

#define FLAGS_SET_Z(flags, value)    ((flags) = (value) ? ((flags) | LWFLAG_Z) : ((flags) & ~LWFLAG_Z))
#define FLAGS_SET_M(flags, value)    ((flags) = (value) ? ((flags) | LWFLAG_M) : ((flags) & ~LWFLAG_M))
#define FLAGS_SET_BBOX(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_BBOX) : ((flags) & ~LWFLAG_BBOX))
#define FLAGS_SET_GEODETIC(flags, value)                                                                               \
	((flags) = (value) ? ((flags) | LWFLAG_GEODETIC) : ((flags) & ~LWFLAG_GEODETIC))
#define FLAGS_SET_READONLY(flags, value)                                                                               \
	((flags) = (value) ? ((flags) | LWFLAG_READONLY) : ((flags) & ~LWFLAG_READONLY))
#define FLAGS_SET_SOLID(flags, value) ((flags) = (value) ? ((flags) | LWFLAG_SOLID) : ((flags) & ~LWFLAG_SOLID))

#define FLAGS_NDIMS(flags)     (2 + FLAGS_GET_Z(flags) + FLAGS_GET_M(flags))
#define FLAGS_GET_ZM(flags)    (FLAGS_GET_M(flags) + FLAGS_GET_Z(flags) * 2)
#define FLAGS_NDIMS_BOX(flags) (FLAGS_GET_GEODETIC(flags) ? 3 : FLAGS_NDIMS(flags))

/**
 * Maximum allowed SRID value in serialized geometry.
 * Currently we are using 21 bits (2097152) of storage for SRID.
 */
#define SRID_MAXIMUM 999999

/**
 * Maximum valid SRID value for the user
 * We reserve 1000 values for internal use
 */
#define SRID_USER_MAXIMUM 998999

/** Unknown SRID value */
#define SRID_UNKNOWN       0
#define SRID_IS_UNKNOWN(x) ((int)x <= 0)

/* Invalid SRID value, for internal use */
#define SRID_INVALID (999999 + 2)

/*
** EPSG WGS84 geographics, OGC standard default SRS, better be in
** the SPATIAL_REF_SYS table!
*/
#define SRID_DEFAULT 4326

#ifndef __GNUC__
#define __attribute__(x)
#endif

/* To return a NULL do this: */
#define PG_ERROR_NULL() throw std::runtime_error("NULL VALUE")

/**
 * Return a valid SRID from an arbitrary integer
 * Raises a notice if what comes out is different from
 * what went in.
 * Raises an error if SRID value is out of bounds.
 */
extern int32_t clamp_srid(int32_t srid);

/**********************************************************************
** Spherical radius.
** Moritz, H. (1980). Geodetic Reference System 1980, by resolution of
** the XVII General Assembly of the IUGG in Canberra.
** http://en.wikipedia.org/wiki/Earth_radius
** http://en.wikipedia.org/wiki/World_Geodetic_System
*/

#define WGS84_MAJOR_AXIS         6378137.0
#define WGS84_INVERSE_FLATTENING 298.257223563
#define WGS84_MINOR_AXIS         (WGS84_MAJOR_AXIS - WGS84_MAJOR_AXIS / WGS84_INVERSE_FLATTENING)
#define WGS84_RADIUS             ((2.0 * WGS84_MAJOR_AXIS + WGS84_MINOR_AXIS) / 3.0)
#define WGS84_SRID               4326

/******************************************************************
 * LWGEOM and GBOX both use LWFLAGS bit mask.
 * Serializations (may) use different bit mask schemes.
 */
typedef uint16_t lwflags_t;

/******************************************************************
 * LWGEOM varlena equivalent type that contains both the size and
 * data(see Postgresql c.h)
 */
typedef struct lwvarlena_t {
	uint32_t size; /* Do not touch this field directly! */
	char data[1];  /* Data content is here */
} lwvarlena_t;

#define LWVARHDRSZ ((int32_t)sizeof(int32_t))

/******************************************************************
 * GBOX structure.
 * We include the flags (information about dimensionality),
 * so we don't have to constantly pass them
 * into functions that use the GBOX.
 */
typedef struct {
	lwflags_t flags;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double zmin;
	double zmax;
	double mmin;
	double mmax;
} GBOX;

/******************************************************************
 * SPHEROID
 *
 *  Standard definition of an ellipsoid (what wkt calls a spheroid)
 *    f = (a-b)/a
 *    e_sq = (a*a - b*b)/(a*a)
 *    b = a - fa
 */
typedef struct {
	double a;      /* semimajor axis */
	double b;      /* semiminor axis b = (a - fa) */
	double f;      /* flattening f = (a-b)/a */
	double e;      /* eccentricity (first) */
	double e_sq;   /* eccentricity squared (first) e_sq = (a*a-b*b)/(a*a) */
	double radius; /* spherical average radius = (2*a+b)/3 */
	char name[20]; /* name of ellipse */
} SPHEROID;

/******************************************************************
 * POINT2D, POINT3D, POINT3DM, POINT4D
 */
typedef struct {
	double x, y;
} POINT2D;

typedef struct {
	double x, y, z;
} POINT3DZ;

typedef struct {
	double x, y, z;
} POINT3D;

typedef struct {
	double x, y, m;
} POINT3DM;

typedef struct {
	double x, y, z, m;
} POINT4D;

/******************************************************************
 *  POINTARRAY
 *  Point array abstracts a lot of the complexity of points and point lists.
 *  It handles 2d/3d translation
 *    (2d points converted to 3d will have z=0 or NaN)
 *  DO NOT MIX 2D and 3D POINTS! EVERYTHING* is either one or the other
 */
typedef struct {
	uint32_t npoints;   /* how many points we are currently storing */
	uint32_t maxpoints; /* how many points we have space for in serialized_pointlist */

	/* Use FLAGS_* macros to handle */
	lwflags_t flags;

	/* Array of POINT 2D, 3D or 4D, possibly misaligned. */
	uint8_t *serialized_pointlist;
} POINTARRAY;

/******************************************************************
 * GSERIALIZED
 */

typedef struct GSERIALIZED {
	uint32_t size;   /* For PgSQL use only, use VAR* macros to manipulate. */
	uint8_t srid[3]; /* 24 bits of SRID */
	uint8_t gflags;  /* HasZ, HasM, HasBBox, IsGeodetic */
	uint8_t data[1]; /* See gserialized.txt */
} GSERIALIZED;

/******************************************************************
 * LWGEOM (any geometry type)
 *
 * Abstract type, note that 'type', 'bbox' and 'srid' are available in
 * all geometry variants.
 */
typedef struct {
	GBOX *bbox;
	void *data;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;
	char pad[1]; /* Padding to 24 bytes (unused) */
} LWGEOM;

/* POINTYPE */
typedef struct {
	GBOX *bbox;
	POINTARRAY *point; /* hide 2d/3d (this will be an array of 1 point) */
	int32_t srid;
	lwflags_t flags;
	uint8_t type; /* POINTTYPE */
	char pad[1];  /* Padding to 24 bytes (unused) */
} LWPOINT;        /* "light-weight point" */

/* LINETYPE */
typedef struct {
	GBOX *bbox;
	POINTARRAY *points; /* array of POINT3D */
	int32_t srid;
	lwflags_t flags;
	uint8_t type; /* LINETYPE */
	char pad[1];  /* Padding to 24 bytes (unused) */
} LWLINE;         /* "light-weight line" */

/* TRIANGLE */
typedef struct {
	GBOX *bbox;
	POINTARRAY *points;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;
	char pad[1]; /* Padding to 24 bytes (unused) */
} LWTRIANGLE;

/* CIRCSTRINGTYPE */
typedef struct {
	GBOX *bbox;
	POINTARRAY *points; /* array of POINT(3D/3DM) */
	int32_t srid;
	lwflags_t flags;
	uint8_t type; /* CIRCSTRINGTYPE */
	char pad[1];  /* Padding to 24 bytes (unused) */
} LWCIRCSTRING;   /* "light-weight circularstring" */

/* MULTIPOINTTYPE */
typedef struct {
	GBOX *bbox;
	LWPOINT **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* MULTYPOINTTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWMPOINT;

/* MULTILINETYPE */
typedef struct {
	GBOX *bbox;
	LWLINE **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* MULTILINETYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWMLINE;

/* POLYGONTYPE */
typedef struct {
	GBOX *bbox;
	POINTARRAY **rings; /* list of rings (list of points) */
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* POLYGONTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t nrings;   /* how many rings we are currently storing */
	uint32_t maxrings; /* how many rings we have space for in **rings */
} LWPOLY;              /* "light-weight polygon" */

/* MULTIPOLYGONTYPE */
typedef struct {
	GBOX *bbox;
	LWPOLY **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* MULTIPOLYGONTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWMPOLY;

/* COLLECTIONTYPE */
typedef struct {
	GBOX *bbox;
	LWGEOM **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* COLLECTIONTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWCOLLECTION;

/* COMPOUNDTYPE */
typedef struct {
	GBOX *bbox;
	LWGEOM **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* COLLECTIONTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWCOMPOUND;          /* "light-weight compound line" */

/* CURVEPOLYTYPE */
typedef struct {
	GBOX *bbox;
	LWGEOM **rings;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* CURVEPOLYTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t nrings;   /* how many rings we are currently storing */
	uint32_t maxrings; /* how many rings we have space for in **rings */
} LWCURVEPOLY;         /* "light-weight polygon" */

/* MULTICURVE */
typedef struct {
	GBOX *bbox;
	LWGEOM **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* MULTICURVE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWMCURVE;

/* MULTISURFACETYPE */
typedef struct {
	GBOX *bbox;
	LWGEOM **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* MULTISURFACETYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWMSURFACE;

/* POLYHEDRALSURFACETYPE */
typedef struct {
	GBOX *bbox;
	LWPOLY **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* POLYHEDRALSURFACETYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWPSURFACE;

/* TINTYPE */
typedef struct {
	GBOX *bbox;
	LWTRIANGLE **geoms;
	int32_t srid;
	lwflags_t flags;
	uint8_t type;      /* TINTYPE */
	char pad[1];       /* Padding to 24 bytes (unused) */
	uint32_t ngeoms;   /* how many geometries we are currently storing */
	uint32_t maxgeoms; /* how many geometries we have space for in **geoms */
} LWTIN;

/* Casts LWGEOM->LW* (return NULL if cast is illegal) */
extern LWMLINE *lwgeom_as_lwmline(const LWGEOM *lwgeom);
extern LWMPOLY *lwgeom_as_lwmpoly(const LWGEOM *lwgeom);
extern LWMPOINT *lwgeom_as_lwmpoint(const LWGEOM *lwgeom);
extern LWPOLY *lwgeom_as_lwpoly(const LWGEOM *lwgeom);
extern LWLINE *lwgeom_as_lwline(const LWGEOM *lwgeom);
extern LWCOLLECTION *lwgeom_as_lwcollection(const LWGEOM *lwgeom);

extern LWCIRCSTRING *lwgeom_as_lwcircstring(const LWGEOM *lwgeom);
extern LWCURVEPOLY *lwgeom_as_lwcurvepoly(const LWGEOM *lwgeom);
extern LWCOMPOUND *lwgeom_as_lwcompound(const LWGEOM *lwgeom);
extern LWTRIANGLE *lwgeom_as_lwtriangle(const LWGEOM *lwgeom);

/* Casts LW*->LWGEOM (always cast) */
extern LWGEOM *lwcollection_as_lwgeom(const LWCOLLECTION *obj);
extern LWGEOM *lwtriangle_as_lwgeom(const LWTRIANGLE *obj);
extern LWGEOM *lwpoly_as_lwgeom(const LWPOLY *obj);
extern LWGEOM *lwline_as_lwgeom(const LWLINE *obj);
extern LWGEOM *lwcircstring_as_lwgeom(const LWCIRCSTRING *obj);
extern LWGEOM *lwcurvepoly_as_lwgeom(const LWCURVEPOLY *obj);
extern LWGEOM *lwpoint_as_lwgeom(const LWPOINT *obj);

extern LWCOLLECTION *lwcollection_add_lwgeom(LWCOLLECTION *col, const LWGEOM *geom);
extern LWMPOINT *lwmpoint_add_lwpoint(LWMPOINT *mobj, const LWPOINT *obj);
extern LWMLINE *lwmline_add_lwline(LWMLINE *mobj, const LWLINE *obj);
extern LWMPOLY *lwmpoly_add_lwpoly(LWMPOLY *mobj, const LWPOLY *obj);

/*
 * copies a point from the point array into the parameter point
 * z value (if present is not returned)
 * NOTE: this will modify the point2d pointed to by 'point'.
 */
extern int getPoint2d_p(const POINTARRAY *pa, uint32_t n, POINT2D *point);

/*
 * set point N to the given value
 * NOTE that the pointarray can be of any
 * dimension, the appropriate ordinate values
 * will be extracted from it
 *
 * N must be a valid point index
 */
extern void ptarray_set_point4d(POINTARRAY *pa, uint32_t n, const POINT4D *p4d);

/**
 * Construct an empty pointarray, allocating storage and setting
 * the npoints, but not filling in any information. Should be used in conjunction
 * with ptarray_set_point4d to fill in the information in the array.
 */
extern POINTARRAY *ptarray_construct(char hasz, char hasm, uint32_t npoints);

/**
 * Construct a new #POINTARRAY, <em>copying</em> in the data from ptlist
 */
extern POINTARRAY *ptarray_construct_copy_data(char hasz, char hasm, uint32_t npoints, const uint8_t *ptlist);

/**
 * Construct a new #POINTARRAY, <em>referencing</em> to the data from ptlist
 */
extern POINTARRAY *ptarray_construct_reference_data(char hasz, char hasm, uint32_t npoints, uint8_t *ptlist);

/**
 * Create a new #POINTARRAY with no points. Allocate enough storage
 * to hold maxpoints vertices before having to reallocate the storage
 * area.
 */
extern POINTARRAY *ptarray_construct_empty(char hasz, char hasm, uint32_t maxpoints);

/**
 * Append a point to the end of an existing #POINTARRAY
 * If allow_duplicate is LW_FALSE, then a duplicate point will
 * not be added.
 */
extern int ptarray_append_point(POINTARRAY *pa, const POINT4D *pt, int allow_duplicates);

/**
 * Append a #POINTARRAY, pa2 to the end of an existing #POINTARRAY, pa1.
 *
 * If gap_tolerance is >= 0 then the end point of pa1 will be checked for
 * being within gap_tolerance 2d distance from start point of pa2 or an
 * error will be raised and LW_FAILURE returned.
 * A gap_tolerance < 0 disables the check.
 *
 * If end point of pa1 and start point of pa2 are 2d-equal, then pa2 first
 * point will not be appended.
 */
extern int ptarray_append_ptarray(POINTARRAY *pa1, POINTARRAY *pa2, double gap_tolerance);

/**
 * Insert a point into an existing #POINTARRAY. Zero
 * is the index of the start of the array.
 */
extern int ptarray_insert_point(POINTARRAY *pa, const POINT4D *p, uint32_t where);

/**
 * Remove a point from an existing #POINTARRAY. Zero
 * is the index of the start of the array.
 */
extern int ptarray_remove_point(POINTARRAY *pa, uint32_t where);

/**
 * @brief Add a point in a pointarray.
 *
 * @param pa the source POINTARRAY
 * @param p the point to add
 * @param pdims number of ordinates in p (2..4)
 * @param where to insert the point. 0 prepends, pa->npoints appends
 *
 * @returns a newly constructed POINTARRAY using a newly allocated buffer
 *          for the actual points, or NULL on error.
 */
extern POINTARRAY *ptarray_addPoint(const POINTARRAY *pa, uint8_t *p, size_t pdims, uint32_t where);

extern int ptarray_is_closed_2d(const POINTARRAY *pa);
extern int ptarray_is_closed_3d(const POINTARRAY *pa);
extern int ptarray_is_closed_z(const POINTARRAY *pa);

extern int lwpoint_getPoint4d_p(const LWPOINT *point, POINT4D *out);

/******************************************************************
 * LWPOLY functions
 ******************************************************************/

/**
 * Add a ring, allocating extra space if necessary. The polygon takes
 * ownership of the passed point array.
 */
extern int lwpoly_add_ring(LWPOLY *poly, POINTARRAY *pa);

/**
 * Add a ring, allocating extra space if necessary. The curvepolygon takes
 * ownership of the passed point array.
 */
extern int lwcurvepoly_add_ring(LWCURVEPOLY *poly, LWGEOM *ring);

/**
 * Add a component, allocating extra space if necessary. The compoundcurve
 * takes owership of the passed geometry.
 */
extern int lwcompound_add_lwgeom(LWCOMPOUND *comp, LWGEOM *geom);

/**
 * Construct an equivalent curve polygon from a polygon. Curve polygons
 * can have linear rings as their rings, so this works fine (in theory?)
 */
extern LWCURVEPOLY *lwcurvepoly_construct_from_lwpoly(LWPOLY *lwpoly);

/**
 * Construct a new flags bitmask.
 */
extern lwflags_t lwflags(int hasz, int hasm, int geodetic);

/******************************************************************
 * LWMULTIx and LWCOLLECTION functions
 ******************************************************************/

LWGEOM *lwcollection_getsubgeom(LWCOLLECTION *col, int gnum);

/******************************************************************
 * SERIALIZED FORM functions
 ******************************************************************/

/**
 * Set the SRID on an LWGEOM
 * For collections, only the parent gets an SRID, all
 * the children get SRID_UNKNOWN.
 */
extern void lwgeom_set_srid(LWGEOM *geom, int32_t srid);

/**
 * Return SRID number
 */
extern int32_t lwgeom_get_srid(const LWGEOM *geom);

/**
 * Return #LW_TRUE if geometry has Z ordinates
 */
extern int lwgeom_has_z(const LWGEOM *geom);

/**
 * Return #LW_TRUE if geometry has M ordinates.
 */
extern int lwgeom_has_m(const LWGEOM *geom);

/****************************************************************
 * MEMORY MANAGEMENT
 ****************************************************************/

/*
 * The *_free family of functions frees *all* memory associated
 * with the pointer. When the recursion gets to the level of the
 * POINTARRAY, the POINTARRAY is only freed if it is not flagged
 * as "read only". LWGEOMs constructed on top of GSERIALIZED
 * from PgSQL use read only point arrays.
 */

extern void ptarray_free(POINTARRAY *pa);
extern void lwpoint_free(LWPOINT *pt);
extern void lwline_free(LWLINE *line);
extern void lwpoly_free(LWPOLY *poly);
extern void lwcircstring_free(LWCIRCSTRING *curve);
extern void lwmpoint_free(LWMPOINT *mpt);
extern void lwmline_free(LWMLINE *mline);
extern void lwmpoly_free(LWMPOLY *mpoly);
extern void lwtriangle_free(LWTRIANGLE *triangle);
extern void lwcollection_free(LWCOLLECTION *col);
extern void lwgeom_free(LWGEOM *geom);

/**
 * Strip out the Z/M components of an #LWGEOM
 */
extern LWGEOM *lwgeom_force_2d(const LWGEOM *geom);

extern float next_float_down(double d);
extern float next_float_up(double d);

/* general utilities 2D */
extern double distance2d_pt_pt(const POINT2D *p1, const POINT2D *p2);
extern double lwgeom_mindistance2d(const LWGEOM *lw1, const LWGEOM *lw2);
extern double lwgeom_mindistance2d_tolerance(const LWGEOM *lw1, const LWGEOM *lw2, double tolerance);
extern double lwgeom_maxdistance2d(const LWGEOM *lw1, const LWGEOM *lw2);
extern double lwgeom_maxdistance2d_tolerance(const LWGEOM *lw1, const LWGEOM *lw2, double tolerance);
extern LWGEOM *lwgeom_closest_point(const LWGEOM *lw1, const LWGEOM *lw2);

extern double lwgeom_area(const LWGEOM *geom);
extern double lwgeom_perimeter_2d(const LWGEOM *geom);
extern int lwgeom_dimension(const LWGEOM *geom);

extern LWPOINT *lwline_get_lwpoint(const LWLINE *line, uint32_t where);

extern LWPOINT *lwcompound_get_startpoint(const LWCOMPOUND *lwcmp);
extern LWPOINT *lwcompound_get_endpoint(const LWCOMPOUND *lwcmp);
extern LWPOINT *lwcompound_get_lwpoint(const LWCOMPOUND *lwcmp, uint32_t where);

extern double ptarray_length_2d(const POINTARRAY *pts);
extern double lwgeom_length_2d(const LWGEOM *geom);
extern int azimuth_pt_pt(const POINT2D *p1, const POINT2D *p2, double *ret);

/**
 * @brief Check whether or not a lwgeom is big enough to warrant a bounding box.
 *
 * Check whether or not a lwgeom is big enough to warrant a bounding box
 * when stored in the serialized form on disk. Currently only points are
 * considered small enough to not require a bounding box, because the
 * index operations can generate a large number of box-retrieval operations
 * when scanning keys.
 */
extern int lwgeom_needs_bbox(const LWGEOM *geom);

/**
 * Count the total number of vertices in any #LWGEOM.
 */
extern uint32_t lwgeom_count_vertices(const LWGEOM *geom);

/**
 * Calculate the GeoHash (http://geohash.org) string for a geometry. Caller must free.
 */
lwvarlena_t *lwgeom_geohash(const LWGEOM *lwgeom, int precision);

/**
 * Pull a #GBOX from the header of a #GSERIALIZED, if one is available. If
 * it is not, calculate it from the geometry. If that doesn't work (null
 * or empty) return LW_FAILURE.
 */
extern int gserialized_get_gbox_p(const GSERIALIZED *g, GBOX *box);

/**
 * Call this function to drop BBOX and SRID
 * from LWGEOM. If LWGEOM type is *not* flagged
 * with the HASBBOX flag and has a bbox, it
 * will be released.
 */
extern void lwgeom_drop_bbox(LWGEOM *lwgeom);

/**
 * Compute a bbox if not already computed
 *
 * After calling this function lwgeom->bbox is only
 * NULL if the geometry is empty.
 */
extern void lwgeom_add_bbox(LWGEOM *lwgeom);

/**
 * Drop current bbox and calculate a fresh one.
 */
extern void lwgeom_refresh_bbox(LWGEOM *lwgeom);

/**
 * Get a non-empty geometry bounding box, computing and
 * caching it if not already there
 *
 * NOTE: empty geometries don't have a bounding box so
 *       you'd still get a NULL for them.
 */
extern const GBOX *lwgeom_get_bbox(const LWGEOM *lwgeom);

/**
 * Return true or false depending on whether a geometry has
 * a valid SRID set.
 */
extern int lwgeom_has_srid(const LWGEOM *geom);

/**
 * Return true or false depending on whether a geometry is a linear
 * feature that closes on itself.
 */
extern int lwgeom_is_closed(const LWGEOM *geom);

/*
 * copies a point from the point array into the parameter point
 * will set point's z=0 (or NaN) if pa is 2d
 * will set point's m=0 (or NaN) if pa is 3d or 2d
 * NOTE: point is a real POINT3D *not* a pointer
 */
extern POINT4D getPoint4d(const POINTARRAY *pa, uint32_t n);

/*
 * copies a point from the point array into the parameter point
 * will set point's z=0 (or NaN) if pa is 2d
 * will set point's m=0 (or NaN) if pa is 3d or 2d
 * NOTE: this will modify the point4d pointed to by 'point'.
 */
extern int getPoint4d_p(const POINTARRAY *pa, uint32_t n, POINT4D *point);

/**
 * Deep clone an LWGEOM, everything is copied
 */
extern LWGEOM *lwgeom_clone_deep(const LWGEOM *lwgeom);
extern POINTARRAY *ptarray_clone_deep(const POINTARRAY *ptarray);

/*
 * Geometry constructors. These constructors to not copy the point arrays
 * passed to them, they just take references, so do not free them out
 * from underneath the geometries.
 */
extern LWPOINT *lwpoint_construct(int32_t srid, GBOX *bbox, POINTARRAY *point);
extern LWLINE *lwline_construct(int32_t srid, GBOX *bbox, POINTARRAY *points);
extern LWPOLY *lwpoly_construct(int32_t srid, GBOX *bbox, uint32_t nrings, POINTARRAY **points);
extern LWCIRCSTRING *lwcircstring_construct(int32_t srid, GBOX *bbox, POINTARRAY *points);
extern LWTRIANGLE *lwtriangle_construct(int32_t srid, GBOX *bbox, POINTARRAY *points);
extern LWCOLLECTION *lwcollection_construct(uint8_t type, int32_t srid, GBOX *bbox, uint32_t ngeoms, LWGEOM **geoms);

/*
 * Empty geometry constructors.
 */
extern LWGEOM *lwgeom_construct_empty(uint8_t type, int32_t srid, char hasz, char hasm);
extern LWPOINT *lwpoint_construct_empty(int32_t srid, char hasz, char hasm);
extern LWLINE *lwline_construct_empty(int32_t srid, char hasz, char hasm);
extern LWPOLY *lwpoly_construct_empty(int32_t srid, char hasz, char hasm);
extern LWMLINE *lwmline_construct_empty(int32_t srid, char hasz, char hasm);
extern LWCURVEPOLY *lwcurvepoly_construct_empty(int32_t srid, char hasz, char hasm);
extern LWCIRCSTRING *lwcircstring_construct_empty(int32_t srid, char hasz, char hasm);
extern LWTRIANGLE *lwtriangle_construct_empty(int32_t srid, char hasz, char hasm);
extern LWCOLLECTION *lwcollection_construct_empty(uint8_t type, int32_t srid, char hasz, char hasm);
extern LWMPOLY *lwmpoly_construct_empty(int32_t srid, char hasz, char hasm);

/* Other constructors */
extern LWPOINT *lwpoint_make2d(int32_t srid, double x, double y);
extern LWPOINT *lwpoint_make3dz(int32_t srid, double x, double y, double z);
extern LWLINE *lwline_from_lwgeom_array(int32_t srid, uint32_t ngeoms, LWGEOM **geoms);
extern LWPOLY *lwpoly_from_lwlines(const LWLINE *shell, uint32_t nholes, const LWLINE **holes);
extern LWPOLY *lwpoly_construct_rectangle(char hasz, char hasm, POINT4D *p1, POINT4D *p2, POINT4D *p3, POINT4D *p4);

/* Some point accessors */
extern double lwpoint_get_x(const LWPOINT *point);
extern double lwpoint_get_y(const LWPOINT *point);

unsigned int geohash_point_as_int(POINT2D *pt);

/**
 * Create an LWGEOM object from a GeoJSON representation
 *
 * @param geojson the GeoJSON input
 * @param srs output parameter. Will be set to a newly allocated
 *            string holding the spatial reference string, or NULL
 *            if no such parameter is found in input.
 *            If not null, the pointer must be freed with lwfree.
 */
extern LWGEOM *lwgeom_from_geojson(const char *geojson, char **srs);

/**
 * Initialize a spheroid object for use in geodetic functions.
 */
extern void spheroid_init(SPHEROID *s, double a, double b);

/**
 * Calculate the geodetic distance from lwgeom1 to lwgeom2 on the spheroid.
 * A spheroid with major axis == minor axis will be treated as a sphere.
 * Pass in a tolerance in spheroid units.
 */
extern double lwgeom_distance_spheroid(const LWGEOM *lwgeom1, const LWGEOM *lwgeom2, const SPHEROID *spheroid,
                                       double tolerance);

/**
 * Calculate the bearing between two points on a spheroid.
 */
extern double lwgeom_azumith_spheroid(const LWPOINT *r, const LWPOINT *s, const SPHEROID *spheroid);

/**
 * Calculate the geodetic area of a lwgeom on the sphere. The result
 * will be multiplied by the average radius of the supplied spheroid.
 */
extern double lwgeom_area_sphere(const LWGEOM *lwgeom, const SPHEROID *spheroid);

/**
 * Calculate the geodetic area of a lwgeom on the spheroid. The result
 * will have the squared units of the spheroid axes.
 */
extern double lwgeom_area_spheroid(const LWGEOM *lwgeom, const SPHEROID *spheroid);

/**
 * Calculate the geodetic length of a lwgeom on the unit sphere. The result
 * will have to by multiplied by the real radius to get the real length.
 */
extern double lwgeom_length_spheroid(const LWGEOM *geom, const SPHEROID *s);

/**
 * Global functions for memory/logging handlers.
 */
typedef void *(*lwallocator)(size_t size);
typedef void *(*lwreallocator)(void *mem, size_t size);
typedef void (*lwfreeor)(void *mem);
typedef void (*lwreporter)(const char *fmt, va_list ap) __attribute__((format(printf, 1, 0)));
typedef void (*lwdebuglogger)(int level, const char *fmt, va_list ap) __attribute__((format(printf, 2, 0)));

/**
 * Macro for reading the size from the GSERIALIZED size attribute.
 * Cribbed from PgSQL, top 30 bits are size. Use VARSIZE() when working
 * internally with PgSQL. See SET_VARSIZE_4B / VARSIZE_4B in
 * PGSRC/src/include/postgres.h for details.
 */
#ifdef WORDS_BIGENDIAN
#define LWSIZE_GET(varsize)      ((varsize)&0x3FFFFFFF)
#define LWSIZE_SET(varsize, len) ((varsize) = ((len)&0x3FFFFFFF))
#define IS_BIG_ENDIAN            1
#else
#define LWSIZE_GET(varsize)      (((varsize) >> 2) & 0x3FFFFFFF)
#define LWSIZE_SET(varsize, len) ((varsize) = (((uint32_t)(len)) << 2))
#define IS_BIG_ENDIAN            0
#endif

/**
 * Parser check flags
 *
 *  @see lwgeom_from_wkb
 *  @see lwgeom_from_hexwkb
 *  @see lwgeom_parse_wkt
 */
#define LW_PARSER_CHECK_MINPOINTS 1
#define LW_PARSER_CHECK_ODD       2
#define LW_PARSER_CHECK_CLOSURE   4
#define LW_PARSER_CHECK_ZCLOSURE  8

#define LW_PARSER_CHECK_NONE 0
#define LW_PARSER_CHECK_ALL  (LW_PARSER_CHECK_MINPOINTS | LW_PARSER_CHECK_ODD | LW_PARSER_CHECK_CLOSURE)

/**
 * Parser result structure: returns the result of attempting to convert
 * (E)WKT/(E)WKB to LWGEOM
 */
typedef struct struct_lwgeom_parser_result {
	const char *wkinput;        /* Copy of pointer to input WKT/WKB */
	uint8_t *serialized_lwgeom; /* Pointer to serialized LWGEOM */
	size_t size;                /* Size of serialized LWGEOM in bytes */
	LWGEOM *geom;               /* Pointer to LWGEOM struct */
	const char *message;        /* Error/warning message */
	int errcode;                /* Error/warning number */
	int errlocation;            /* Location of error */
	int parser_check_flags;     /* Bitmask of validity checks run during this parse */
} LWGEOM_PARSER_RESULT;

/*
 * Parser error messages (these must match the message array in lwgparse.c)
 */
#define PARSER_ERROR_MOREPOINTS     1
#define PARSER_ERROR_ODDPOINTS      2
#define PARSER_ERROR_UNCLOSED       3
#define PARSER_ERROR_MIXDIMS        4
#define PARSER_ERROR_INVALIDGEOM    5
#define PARSER_ERROR_INVALIDWKBTYPE 6
#define PARSER_ERROR_INCONTINUOUS   7
#define PARSER_ERROR_TRIANGLEPOINTS 8
#define PARSER_ERROR_LESSPOINTS     9
#define PARSER_ERROR_OTHER          10

/*
** Variants available for WKB and WKT output types
*/

#define WKB_ISO        0x01
#define WKB_SFSQL      0x02
#define WKB_EXTENDED   0x04
#define WKB_NDR        0x08
#define WKB_XDR        0x10
#define WKB_HEX        0x20
#define WKB_NO_NPOINTS 0x40 /* Internal use only */
#define WKB_NO_SRID    0x80 /* Internal use only */

#define WKT_ISO      0x01
#define WKT_SFSQL    0x02
#define WKT_EXTENDED 0x04

/* Number of digits of precision in WKT produced. */
#define WKT_PRECISION 15

struct LWPOINTITERATOR;
typedef struct LWPOINTITERATOR LWPOINTITERATOR;

/**
 * Create a new LWPOINTITERATOR over supplied LWGEOM*
 */
extern LWPOINTITERATOR *lwpointiterator_create(const LWGEOM *g);

/**
 * Create a new LWPOINTITERATOR over supplied LWGEOM*
 * Supports modification of coordinates during iteration.
 */
extern LWPOINTITERATOR *lwpointiterator_create_rw(LWGEOM *g);

/**
 * Free all memory associated with the iterator
 */
extern void lwpointiterator_destroy(LWPOINTITERATOR *s);

/**
 * Returns LW_TRUE if there is another point available in the iterator.
 */
extern int lwpointiterator_has_next(LWPOINTITERATOR *s);

/**
 * Attempts to assigns the next point in the iterator to p.  Does not advance.
 * Returns LW_SUCCESS if the assignment was successful, LW_FAILURE otherwise.
 */
extern int lwpointiterator_peek(LWPOINTITERATOR *s, POINT4D *p);

/**
 * Attempts to assign the next point in the iterator to p, and advances
 * the iterator to the next point.  If p is NULL, the iterator will be
 * advanced without reading a point.
 * Returns LW_SUCCESS if the assignment was successful, LW_FAILURE otherwise.
 * */
extern int lwpointiterator_next(LWPOINTITERATOR *s, POINT4D *p);

/**
 * Check if a #GSERIALIZED has a bounding box without deserializing first.
 */
extern int gserialized_has_bbox(const GSERIALIZED *gser);

/**
 * @param geom geometry to convert to HEXWKB
 * @param variant output format to use
 *                (WKB_ISO, WKB_SFSQL, WKB_EXTENDED, WKB_NDR, WKB_XDR)
 * @param size_out (Out parameter) size of the buffer
 */
extern char *lwgeom_to_hexwkb_buffer(const LWGEOM *geom, uint8_t variant);

/*
 * WKT detailed parsing support
 */
extern int lwgeom_parse_wkt(LWGEOM_PARSER_RESULT *parser_result, char *wktstr, int parse_flags);
void lwgeom_parser_result_init(LWGEOM_PARSER_RESULT *parser_result);
void lwgeom_parser_result_free(LWGEOM_PARSER_RESULT *parser_result);

/*******************************************************************************
 * SQLMM internal functions
 ******************************************************************************/

int lwgeom_has_arc(const LWGEOM *geom);
LWGEOM *lwgeom_stroke(const LWGEOM *geom, uint32_t perQuad);

/**
 * Semantic of the `tolerance` argument passed to
 * lwcurve_linearize
 */
typedef enum {
	/**
	 * Tolerance expresses the number of segments to use
	 * for each quarter of circle (quadrant). Must be
	 * an integer.
	 */
	LW_LINEARIZE_TOLERANCE_TYPE_SEGS_PER_QUAD = 0,
	/**
	 * Tolerance expresses the maximum distance between
	 * an arbitrary point on the curve and the closest
	 * point to it on the resulting approximation, in
	 * cartesian units.
	 */
	LW_LINEARIZE_TOLERANCE_TYPE_MAX_DEVIATION = 1,
	/**
	 * Tolerance expresses the maximum angle between
	 * the radii generating approximation line vertices,
	 * given in radiuses. A value of 1 would result
	 * in an approximation of a semicircle composed by
	 * 180 segments
	 */
	LW_LINEARIZE_TOLERANCE_TYPE_MAX_ANGLE = 2
} LW_LINEARIZE_TOLERANCE_TYPE;

typedef enum {
	/**
	 * Symmetric linearization means that the output
	 * vertices would be the same no matter the order
	 * of the points defining the input curve.
	 */
	LW_LINEARIZE_FLAG_SYMMETRIC = 1 << 0,

	/**
	 * Retain angle instructs the engine to try its best
	 * to retain the requested angle between generating
	 * radii (where angle can be given explicitly with
	 * LW_LINEARIZE_TOLERANCE_TYPE_MAX_ANGLE or implicitly
	 * with LW_LINEARIZE_TOLERANCE_TYPE_SEGS_PER_QUAD or
	 * LW_LINEARIZE_TOLERANCE_TYPE_MAX_DEVIATION).
	 *
	 * It only makes sense with LW_LINEARIZE_FLAG_SYMMETRIC
	 * which would otherwise reduce the angle as needed to
	 * keep it constant among all radiis so that all
	 * segments are of the same length.
	 *
	 * When this flag is set, the first and last generating
	 * angles (and thus the first and last segments) may
	 * instead be smaller (shorter) than the others.
	 *
	 */
	LW_LINEARIZE_FLAG_RETAIN_ANGLE = 1 << 1
} LW_LINEARIZE_FLAGS;

/**
 * @param geom input geometry
 * @param tol tolerance, semantic driven by tolerance_type
 * @param type see LW_LINEARIZE_TOLERANCE_TYPE
 * @param flags bitwise OR of operational flags, see LW_LINEARIZE_FLAGS
 *
 * @return a newly allocated LWGEOM
 */
extern LWGEOM *lwcurve_linearize(const LWGEOM *geom, double tol, LW_LINEARIZE_TOLERANCE_TYPE type, int flags);

/**
 * Return the type name string associated with a type number
 * (e.g. Point, LineString, Polygon)
 */
extern const char *lwtype_name(uint8_t type);

/*
** New parsing and unparsing functions.
*/

/**
 * @param geom geometry to convert to WKT
 * @param variant output format to use (WKT_ISO, WKT_SFSQL, WKT_EXTENDED)
 * @param precision Double precision
 * @param size_out (Out parameter) size of the buffer
 */
extern char *lwgeom_to_wkt(const LWGEOM *geom, uint8_t variant, int precision, size_t *size_out);

/**
 * @param geom geometry to convert to WKT
 * @param variant output format to use (WKT_ISO, WKT_SFSQL, WKT_EXTENDED)
 * @param precision Double precision
 */
extern lwvarlena_t *lwgeom_to_wkt_varlena(const LWGEOM *geom, uint8_t variant, int precision);

extern lwvarlena_t *lwgeom_to_wkb_varlena(const LWGEOM *geom, uint8_t variant);

extern uint8_t *bytes_from_hexbytes(const char *hexbuf, size_t hexsize);

/***********************************************************************
** Functions for managing serialized forms and bounding boxes.
*/

/**
 * Check that coordinates of LWGEOM are all within the geodetic range (-180, -90, 180, 90)
 */
extern int lwgeom_check_geodetic(const LWGEOM *geom);

/**
 * Set the FLAGS geodetic bit on geometry an all sub-geometries and pointlists
 */
extern void lwgeom_set_geodetic(LWGEOM *geom, int value);

/**
 * Calculate the geodetic bounding box for an LWGEOM. Z/M coordinates are
 * ignored for this calculation. Pass in non-null, geodetic bounding box for function
 * to fill out. LWGEOM must have been built from a GSERIALIZED to provide
 * double aligned point arrays.
 */
extern int lwgeom_calculate_gbox_geodetic(const LWGEOM *geom, GBOX *gbox);

/**
 * Calculate the 2-4D bounding box of a geometry. Z/M coordinates are honored
 * for this calculation, though for curves they are not included in calculations
 * of curvature.
 */
extern int lwgeom_calculate_gbox_cartesian(const LWGEOM *lwgeom, GBOX *gbox);

/**
 * Calculate bounding box of a geometry, automatically taking into account
 * whether it is cartesian or geodetic.
 */
extern int lwgeom_calculate_gbox(const LWGEOM *lwgeom, GBOX *gbox);

/**
 * Calculate geodetic (x/y/z) box and add values to gbox. Return #LW_SUCCESS on success.
 */
extern int ptarray_calculate_gbox_geodetic(const POINTARRAY *pa, GBOX *gbox);

/**
 * Calculate box (x/y) and add values to gbox. Return #LW_SUCCESS on success.
 */
extern int ptarray_calculate_gbox_cartesian(const POINTARRAY *pa, GBOX *gbox);

/**
 * @param wkb_size length of WKB byte buffer
 * @param wkb WKB byte buffer
 * @param check parser check flags, see LW_PARSER_CHECK_* macros
 */
extern LWGEOM *lwgeom_from_wkb(const uint8_t *wkb, const size_t wkb_size, const char check);

/**
 * Create a new gbox with the dimensionality indicated by the flags. Caller
 * is responsible for freeing.
 */
extern GBOX *gbox_new(lwflags_t flags);

/**
 * Calculate a spherical point that falls outside the geocentric gbox
 */
int gbox_pt_outside(const GBOX *gbox, POINT2D *pt_outside);

/**
 * Zero out all the entries in the #GBOX. Useful for cleaning
 * statically allocated gboxes.
 */
extern void gbox_init(GBOX *gbox);

/**
 * Update the merged #GBOX to be large enough to include itself and the new box.
 */
extern int gbox_merge(const GBOX *new_box, GBOX *merged_box);

/**
 * Return a copy of the #GBOX, based on dimensionality of flags.
 */
extern GBOX *gbox_copy(const GBOX *gbox);

/**
 * Return #LW_TRUE if the #GBOX overlaps, #LW_FALSE otherwise.
 */
extern int gbox_overlaps(const GBOX *g1, const GBOX *g2);

/**
 * Initialize a #GBOX using the values of the point.
 */
extern int gbox_init_point3d(const POINT3D *p, GBOX *gbox);

/**
 * Update the #GBOX to be large enough to include itself and the new point.
 */
extern int gbox_merge_point3d(const POINT3D *p, GBOX *gbox);

/**
 * Return true if the point is inside the gbox
 */
extern int gbox_contains_point3d(const GBOX *gbox, const POINT3D *pt);

/**
 * Return #LW_TRUE if the #GBOX overlaps on the 2d plane, #LW_FALSE otherwise.
 */
extern int gbox_overlaps_2d(const GBOX *g1, const GBOX *g2);

/**
 * Return #LW_TRUE if the first #GBOX contains the second on the 2d plane, #LW_FALSE otherwise.
 */
extern int gbox_contains_2d(const GBOX *g1, const GBOX *g2);

/**
 * Copy the values of original #GBOX into duplicate.
 */
extern void gbox_duplicate(const GBOX *original, GBOX *duplicate);

/**
 * Return the number of bytes necessary to hold a #GBOX of this dimension in
 * serialized form.
 */
extern size_t gbox_serialized_size(lwflags_t flags);

/**
 * Check if two given GBOX are the same in x and y, or would round to the same
 * GBOX in x and if serialized in GSERIALIZED
 */
extern int gbox_same_2d_float(const GBOX *g1, const GBOX *g2);

/**
 * Round given GBOX to float boundaries
 *
 * This turns a GBOX into the version it would become
 * after a serialize/deserialize round trip.
 */
extern void gbox_float_round(GBOX *gbox);

/**
 * Extract the geometry type from the serialized form (it hides in
 * the anonymous data area, so this is a handy function).
 */
extern uint32_t gserialized_get_type(const GSERIALIZED *g);

/**
 * Pull the first point values of a #GSERIALIZED. Only works for POINTTYPE
 */
extern int gserialized_peek_first_point(const GSERIALIZED *g, POINT4D *out_point);

/**
 * Determine whether a LWGEOM can contain sub-geometries or not
 */
extern int lwgeom_is_collection(const LWGEOM *lwgeom);

/******************************************************************/
/* Functions that work on type numbers */

/**
 * Determine whether a type number is a collection or not
 */
extern int lwtype_is_collection(uint8_t type);

/**
 * Given an lwtype number, what homogeneous collection can hold it?
 */
extern uint32_t lwtype_get_collectiontype(uint8_t type);

/**
 * @param geom geometry to convert to WKB
 * @param variant output format to use
 *                (WKB_ISO, WKB_SFSQL, WKB_EXTENDED, WKB_NDR, WKB_XDR)
 */
extern uint8_t *lwgeom_to_wkb_buffer(const LWGEOM *geom, uint8_t variant);
extern size_t lwgeom_to_wkb_size(const LWGEOM *geom, uint8_t variant);

/* Memory management */
extern void *lwalloc(size_t size);
extern void *lwrealloc(void *mem, size_t size);
extern void lwfree(void *mem);

/**
 * Write a notice out to the notice handler.
 *
 * Uses standard printf() substitutions.
 * Use for messages you always want output.
 * For debugging, use LWDEBUG() or LWDEBUGF().
 * @ingroup logging
 */
void lwnotice(const char *fmt, ...);

/**
 * Write a notice out to the error handler.
 *
 * Uses standard printf() substitutions.
 * Use for errors you always want output.
 * For debugging, use LWDEBUG() or LWDEBUGF().
 * @ingroup logging
 */
void lwerror(const char *fmt, ...);

extern lwvarlena_t *lwgeom_to_geojson(const LWGEOM *geo, const char *srs, int precision, int has_bbox);

extern int lwgeom_startpoint(const LWGEOM *lwgeom, POINT4D *pt);

/**
 * Snap-to-grid
 */
typedef struct gridspec_t {
	double ipx;
	double ipy;
	double ipz;
	double ipm;
	double xsize;
	double ysize;
	double zsize;
	double msize;
} gridspec;

extern LWGEOM *lwgeom_grid(const LWGEOM *lwgeom, const gridspec *grid);
extern void lwgeom_grid_in_place(LWGEOM *lwgeom, const gridspec *grid);

/****************************************************************
 * READ/WRITE FUNCTIONS
 *
 * Coordinate writing functions, which will alter the coordinates
 * and potentially the structure of the input geometry. When
 * called from within PostGIS, the LWGEOM argument should be built
 * on top of a gserialized copy, created using
 * PG_GETARG_GSERIALIZED_P_COPY()
 ****************************************************************/

extern int lwgeom_simplify_in_place(LWGEOM *igeom, double dist, int preserve_collapsed);

/*******************************************************************************
 * GEOS proxy functions on LWGEOM
 ******************************************************************************/

LWGEOM *lwgeom_difference_prec(const LWGEOM *geom1, const LWGEOM *geom2, double gridSize);
LWGEOM *lwgeom_intersection_prec(const LWGEOM *geom1, const LWGEOM *geom2, double gridSize);
LWGEOM *lwgeom_union_prec(const LWGEOM *geom1, const LWGEOM *geom2, double gridSize);
LWGEOM *lwgeom_centroid(const LWGEOM *geom);

#endif /* !defined _LIBLWGEOM_H  */

} // namespace duckdb
