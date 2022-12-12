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
 * Copyright 2001-2005 Refractions Research Inc.
 *
 **********************************************************************/

#include "postgis/lwgeom_ogc.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/lwgeom_geos.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "postgis/lwgeom_geos.hpp"

namespace duckdb {

/* Matches lwutil.c::lwgeomTypeName */
std::string stTypeName[] = {"Unknown",           "ST_Point",
                            "ST_LineString",     "ST_Polygon",
                            "ST_MultiPoint",     "ST_MultiLineString",
                            "ST_MultiPolygon",   "ST_GeometryCollection",
                            "ST_CircularString", "ST_CompoundCurve",
                            "ST_CurvePolygon",   "ST_MultiCurve",
                            "ST_MultiSurface",   "ST_PolyhedralSurface",
                            "ST_Triangle",       "ST_Tin"};

GSERIALIZED *LWGEOM_from_text(char *wkt, int srid) {
	LWGEOM_PARSER_RESULT lwg_parser_result;
	GSERIALIZED *geom_result = NULL;
	LWGEOM *lwgeom;

	if (lwgeom_parse_wkt(&lwg_parser_result, wkt, LW_PARSER_CHECK_ALL) == LW_FAILURE)
		// PG_PARSER_ERROR(lwg_parser_result);
		return NULL;

	lwgeom = lwg_parser_result.geom;

	if (lwgeom->srid != SRID_UNKNOWN) {
		// elog(WARNING, "OGC WKT expected, EWKT provided - use GeomFromEWKT() for this");
	}

	/* read user-requested SRID if any */
	if (srid != SRID_UNKNOWN)
		lwgeom_set_srid(lwgeom, srid);

	geom_result = geometry_serialize(lwgeom);
	lwgeom_parser_result_free(&lwg_parser_result);

	return geom_result;
}

GSERIALIZED *LWGEOM_from_WKB(const char *bytea_wkb, size_t byte_size, int srid) {
	GSERIALIZED *geom = nullptr;
	LWGEOM *lwgeom;

	lwgeom = lwgeom_from_wkb((const uint8_t *)bytea_wkb, byte_size, LW_PARSER_CHECK_ALL);
	if (!lwgeom) {
		// lwpgerror("Unable to parse WKB");
		return NULL;
	}

	geom = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);

	if (gserialized_get_srid(geom) != SRID_UNKNOWN) {
		// elog(WARNING, "OGC WKB expected, EWKB provided - use GeometryFromEWKB() for this");
	}

	if (srid != SRID_UNKNOWN) {
		if (srid != gserialized_get_srid(geom))
			gserialized_set_srid(geom, srid);
	}

	return geom;
}

GSERIALIZED *LWGEOM_boundary(GSERIALIZED *geom1) {
	GEOSGeometry *g1, *g3;
	GSERIALIZED *result;
	LWGEOM *lwgeom;
	int32_t srid;

	/* Empty.Boundary() == Empty */
	if (gserialized_is_empty(geom1))
		return nullptr;

	srid = gserialized_get_srid(geom1);

	lwgeom = lwgeom_from_gserialized(geom1);
	if (!lwgeom) {
		// lwpgerror("POSTGIS2GEOS: unable to deserialize input");
		return nullptr;
	}

	/* GEOS doesn't do triangle type, so we special case that here */
	if (lwgeom->type == TRIANGLETYPE) {
		lwgeom->type = LINETYPE;
		result = geometry_serialize(lwgeom);
		lwgeom_free(lwgeom);
		return result;
	}

	initGEOS(lwnotice, lwgeom_geos_error);

	g1 = LWGEOM2GEOS(lwgeom, 0);
	lwgeom_free(lwgeom);

	if (!g1)
		throw "First argument geometry could not be converted to GEOS";

	g3 = GEOSBoundary(g1);

	if (!g3) {
		GEOSGeom_destroy(g1);
		throw "GEOSBoundary";
	}

	GEOSSetSRID(g3, srid);

	result = GEOS2POSTGIS(g3, gserialized_has_z(geom1));

	if (!result) {
		GEOSGeom_destroy(g1);
		GEOSGeom_destroy(g3);
		throw "GEOS2POSTGIS threw an error (result postgis geometry formation)!";
		return nullptr;
	}

	GEOSGeom_destroy(g1);
	GEOSGeom_destroy(g3);

	return result;
}

/** @brief
 * 		returns 0 for points, 1 for lines, 2 for polygons, 3 for volume.
 * 		returns max dimension for a collection.
 */
int LWGEOM_dimension(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	int dimension = -1;

	dimension = lwgeom_dimension(lwgeom);
	lwgeom_free(lwgeom);

	if (dimension < 0) {
		// elog(NOTICE, "Could not compute geometry dimensions");
		return -1;
	}

	return dimension;
}

double LWGEOM_x_point(GSERIALIZED *geom) {
	POINT4D pt;

	if (gserialized_get_type(geom) != POINTTYPE) {
		// lwpgerror("Argument to ST_X() must have type POINT");
		throw Exception("Argument to ST_X() must have type POINT");
		return LW_FAILURE;
	}

	if (gserialized_peek_first_point(geom, &pt) == LW_FAILURE) {
		return LW_FAILURE;
	}
	return pt.x;
}

/**
 * Y(GEOMETRY) -- return Y value of the point.
 * 	Raise an error if input is not a point.
 */
double LWGEOM_y_point(GSERIALIZED *geom) {
	POINT4D pt;

	if (gserialized_get_type(geom) != POINTTYPE) {
		// lwpgerror("Argument to ST_Y() must have type POINT");
		throw Exception("Argument to ST_Y() must have type POINT");
		return LW_FAILURE;
	}

	if (gserialized_peek_first_point(geom, &pt) == LW_FAILURE) {
		// PG_RETURN_NULL();
		return LW_FAILURE;
	}
	return pt.y;
}

/** EndPoint(GEOMETRY) -- find the first linestring in GEOMETRY,
 * @return the last point.
 * 	Return NULL if there is no LINESTRING(..) in GEOMETRY
 */
GSERIALIZED *LWGEOM_endpoint_linestring(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	LWPOINT *lwpoint = NULL;
	int type = lwgeom->type;

	if (type == LINETYPE || type == CIRCSTRINGTYPE) {
		LWLINE *line = (LWLINE *)lwgeom;
		if (line->points)
			lwpoint = lwline_get_lwpoint((LWLINE *)lwgeom, line->points->npoints - 1);
	} else if (type == COMPOUNDTYPE) {
		lwpoint = lwcompound_get_endpoint((LWCOMPOUND *)lwgeom);
	}

	lwgeom_free(lwgeom);

	if (!lwpoint)
		return nullptr;

	GSERIALIZED *ret = geometry_serialize(lwpoint_as_lwgeom(lwpoint));
	lwgeom_free(lwpoint_as_lwgeom(lwpoint));

	return ret;
}

/* returns a string representation of this geometry's type */
std::string geometry_geometrytype(GSERIALIZED *gser) {
	/* Build a text type to store things in */
	return stTypeName[gserialized_get_type(gser)];
}

/**
 * @brief IsClosed(GEOMETRY) if geometry is a linestring then returns
 * 		startpoint == endpoint.  If its not a linestring then return NULL.
 * 		If it's a collection containing multiple linestrings,
 * @return true only if all the linestrings have startpoint=endpoint.
 */
bool LWGEOM_isclosed(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	int closed = lwgeom_is_closed(lwgeom);

	lwgeom_free(lwgeom);
	return closed;
}

int LWGEOM_numgeometries_collection(GSERIALIZED *geom) {
	LWGEOM *lwgeom;
	uint32_t ret = 1;

	lwgeom = lwgeom_from_gserialized(geom);
	if (lwgeom_is_empty(lwgeom)) {
		ret = 0;
	} else if (lwgeom_is_collection(lwgeom)) {
		LWCOLLECTION *col = lwgeom_as_lwcollection(lwgeom);
		ret = col->ngeoms;
	}
	lwgeom_free(lwgeom);

	return ret;
}

/**
 * numpoints(LINESTRING) -- return the number of points in the
 * linestring, or NULL if it is not a linestring
 */
int LWGEOM_numpoints_linestring(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	int count = -1;
	int type = lwgeom->type;

	if (type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE)
		count = lwgeom_count_vertices(lwgeom);

	lwgeom_free(lwgeom);

	/* OGC says this functions is only valid on LINESTRING */
	if (count < 0)
		return 0;

	return count;
}

/**
 * PointN(GEOMETRY,INTEGER) -- find the first linestring in GEOMETRY,
 * @return the point at index INTEGER (1 is 1st point).  Return NULL if
 * 		there is no LINESTRING(..) in GEOMETRY or INTEGER is out of bounds.
 */
GSERIALIZED *LWGEOM_pointn_linestring(GSERIALIZED *geom, int where) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	LWPOINT *lwpoint = NULL;
	int type = lwgeom->type;

	/* If index is negative, count backward */
	if (where < 1) {
		int count = -1;
		if (type == LINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE)
			count = lwgeom_count_vertices(lwgeom);
		if (count > 0) {
			/* only work if we found the total point number */
			/* converting where to positive backward indexing, +1 because 1 indexing */
			where = where + count + 1;
		}
		if (where < 1)
			return nullptr;
	}

	if (type == LINETYPE || type == CIRCSTRINGTYPE) {
		/* OGC index starts at one, so we substract first. */
		lwpoint = lwline_get_lwpoint((LWLINE *)lwgeom, where - 1);
	} else if (type == COMPOUNDTYPE) {
		lwpoint = lwcompound_get_lwpoint((LWCOMPOUND *)lwgeom, where - 1);
	}

	lwgeom_free(lwgeom);

	if (!lwpoint)
		return nullptr;

	auto ret = geometry_serialize(lwpoint_as_lwgeom(lwpoint));

	lwgeom_free((LWGEOM *)lwpoint);
	return ret;
}

/**
 * ST_StartPoint(GEOMETRY)
 * @return the first point of a linestring.
 * 		Return NULL if there is no LINESTRING
 */
GSERIALIZED *LWGEOM_startpoint_linestring(GSERIALIZED *geom) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);
	LWPOINT *lwpoint = NULL;
	int type = lwgeom->type;

	if (type == LINETYPE || type == CIRCSTRINGTYPE) {
		lwpoint = lwline_get_lwpoint((LWLINE *)lwgeom, 0);
	} else if (type == COMPOUNDTYPE) {
		lwpoint = lwcompound_get_startpoint((LWCOMPOUND *)lwgeom);
	}

	lwgeom_free(lwgeom);

	if (!lwpoint)
		return nullptr;

	auto ret = geometry_serialize(lwpoint_as_lwgeom(lwpoint));
	lwgeom_free((LWGEOM *)lwpoint);

	return ret;
}

} // namespace duckdb
