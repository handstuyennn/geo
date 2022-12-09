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
 * Copyright (C) 2004-2015 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Mark Leslie <mark.leslie@lisasoft.com>
 * Copyright (C) 2008-2009 Mark Cave-Ayland <mark.cave-ayland@siriusit.co.uk>
 * Copyright (C) 2009-2015 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2010 Olivier Courtin <olivier.courtin@camptocamp.com>
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

static std::string lwgeomTypeName[] = {"Unknown",        "Point",
                                       "LineString",     "Polygon",
                                       "MultiPoint",     "MultiLineString",
                                       "MultiPolygon",   "GeometryCollection",
                                       "CircularString", "CompoundCurve",
                                       "CurvePolygon",   "MultiCurve",
                                       "MultiSurface",   "PolyhedralSurface",
                                       "Triangle",       "Tin"};

const char *lwtype_name(uint8_t type) {
	if (type > 15) {
		/* assert(0); */
		return "Invalid type";
	}
	return lwgeomTypeName[(int)type].c_str();
}

/* Default allocators */
static void *default_allocator(size_t size);
static void default_freeor(void *mem);
static void *default_reallocator(void *mem, size_t size);
lwallocator lwalloc_var = default_allocator;
lwreallocator lwrealloc_var = default_reallocator;
lwfreeor lwfree_var = default_freeor;

void *lwalloc(size_t size) {
	void *mem = lwalloc_var(size);
	return mem;
}

void *lwrealloc(void *mem, size_t size) {
	return lwrealloc_var(mem, size);
}

void lwfree(void *mem) {
	lwfree_var(mem);
}

/*
 * Default allocators
 *
 * We include some default allocators that use malloc/free/realloc
 * along with stdout/stderr since this is the most common use case
 *
 */

static void *default_allocator(size_t size) {
	void *mem = malloc(size);
	return mem;
}

static void default_freeor(void *mem) {
	free(mem);
}

static void *default_reallocator(void *mem, size_t size) {
	void *ret = realloc(mem, size);
	return ret;
}

int32_t clamp_srid(int32_t srid) {
	int newsrid = srid;

	if (newsrid <= 0) {
		if (newsrid != SRID_UNKNOWN) {
			newsrid = SRID_UNKNOWN;
			// lwnotice("SRID value %d converted to the officially unknown SRID value %d", srid, newsrid);
		}
	} else if (srid > SRID_MAXIMUM) {
		newsrid = SRID_USER_MAXIMUM + 1 +
		          /* -1 is to reduce likelyhood of clashes */
		          /* NOTE: must match implementation in postgis_restore.pl */
		          (srid % (SRID_MAXIMUM - SRID_USER_MAXIMUM - 1));
		// lwnotice("SRID value %d > SRID_MAXIMUM converted to %d", srid, newsrid);
	}

	return newsrid;
}

lwflags_t lwflags(int hasz, int hasm, int geodetic) {
	lwflags_t flags = 0;
	if (hasz)
		FLAGS_SET_Z(flags, 1);
	if (hasm)
		FLAGS_SET_M(flags, 1);
	if (geodetic)
		FLAGS_SET_GEODETIC(flags, 1);
	return flags;
}

void lwerror(const char *fmt, ...) {
	va_list ap;
	char buffer[100];
	sprintf(buffer, fmt, ap);
	throw std::runtime_error(buffer);
}

void lwnotice(const char *fmt, ...) {
	va_list ap;

	char buffer[100];
	sprintf(buffer, fmt, ap);
}

} // namespace duckdb
