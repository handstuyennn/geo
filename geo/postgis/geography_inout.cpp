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
 * Copyright 2009-2011 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 **********************************************************************/

#include "postgis/geography_inout.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "libpgcommon/lwgeom_pg.hpp"
#include "postgis/geography.hpp"

namespace duckdb {

GSERIALIZED *gserialized_geography_from_lwgeom(LWGEOM *lwgeom, int32_t geog_typmod) {
	GSERIALIZED *g_ser = NULL;

	/* Set geodetic flag */
	lwgeom_set_geodetic(lwgeom, true);

	/* Check that this is a type we can handle */
	geography_valid_type(lwgeom->type);

	/* Force the geometry to have valid geodetic coordinate range. */
	lwgeom_nudge_geodetic(lwgeom);
	if (lwgeom_force_geodetic(lwgeom) == LW_TRUE) {
		throw ParserException("Coordinate values were coerced into range [-180 -90, 180 90] for GEOGRAPHY");
		return nullptr;
	}

	/* Force default SRID to the default */
	if ((int)lwgeom->srid <= 0)
		lwgeom->srid = SRID_DEFAULT;

	/*
	** Serialize our lwgeom and set the geodetic flag so subsequent
	** functions do the right thing.
	*/
	g_ser = geography_serialize(lwgeom);

	/* Check for typmod agreement */
	if (geog_typmod >= 0) {
		g_ser = postgis_valid_typmod(g_ser, geog_typmod);
	}

	return g_ser;
}

/**
 * The geography type only support POINT, LINESTRING, POLYGON, MULTI* variants
 * of same, and GEOMETRYCOLLECTION. If the input type is not one of those, shut
 * down the query.
 */
void geography_valid_type(uint8_t type) {
	if (!(type == POINTTYPE || type == LINETYPE || type == POLYGONTYPE || type == MULTIPOINTTYPE ||
	      type == MULTILINETYPE || type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE)) {
		throw("Geography type does not support " + std::string(lwtype_name(type)));
	}
}

/*
** geography_from_text(*char) returns *GSERIALIZED
**
** Convert text (varlena) to cstring and then call geography_in().
*/
GSERIALIZED *geography_from_text(char *wkt) {
	LWGEOM_PARSER_RESULT lwg_parser_result;
	GSERIALIZED *g_ser = NULL;

	/* Pass the cstring to the input parser, and magic occurs! */
	if (lwgeom_parse_wkt(&lwg_parser_result, wkt, LW_PARSER_CHECK_ALL) == LW_FAILURE)
		return NULL;

	/* Clean up string */
	g_ser = gserialized_geography_from_lwgeom(lwg_parser_result.geom, -1);

	/* Clean up temporary object */
	lwgeom_free(lwg_parser_result.geom);

	return g_ser;
}

/*
** geography_from_binary(*char) returns *GSERIALIZED
*/
GSERIALIZED *geography_from_binary(const char *bytea_wkb, size_t byte_size) {
	GSERIALIZED *gser = NULL;
	LWGEOM *lwgeom = lwgeom_from_wkb((const uint8_t *)bytea_wkb, byte_size, LW_PARSER_CHECK_NONE);

	if (!lwgeom)
		throw ConversionException("Unable to parse WKB");

	gser = gserialized_geography_from_lwgeom(lwgeom, -1);
	lwgeom_free(lwgeom);
	return gser;
}

/*
** geography_in(cstring) returns *GSERIALIZED
*/
GSERIALIZED *geography_in(char *str) {
	int32_t geog_typmod = -1;
	LWGEOM_PARSER_RESULT lwg_parser_result;
	LWGEOM *lwgeom = NULL;
	GSERIALIZED *g_ser = NULL;

	lwgeom_parser_result_init(&lwg_parser_result);

	/* Empty string. */
	if (str[0] == '\0')
		throw ConversionException("parse error - invalid geometry");

	/* WKB? Let's find out. */
	if (str[0] == '0') {
		/* TODO: 20101206: No parser checks! This is inline with current 1.5 behavior, but needs discussion */
		lwgeom = lwgeom_from_hexwkb(str, LW_PARSER_CHECK_NONE);
		/* Error out if something went sideways */
		if (!lwgeom)
			return nullptr;
	}
	/* GEOJson */
	else if (str[0] == '{') {
		char *srs = NULL;
		lwgeom = lwgeom_from_geojson(str, &srs);
		if (!lwgeom) {
			return NULL;
		}
	}
	/* WKT then. */
	else {
		if (lwgeom_parse_wkt(&lwg_parser_result, str, LW_PARSER_CHECK_ALL) == LW_FAILURE)
			return nullptr;

		lwgeom = lwg_parser_result.geom;
	}

	/* Convert to gserialized */
	g_ser = gserialized_geography_from_lwgeom(lwgeom, geog_typmod);

	/* Clean up temporary object */
	lwgeom_free(lwgeom);

	return g_ser;
}

} // namespace duckdb
