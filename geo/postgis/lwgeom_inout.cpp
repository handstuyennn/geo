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
 * ^copyright^
 *
 **********************************************************************/

#include "postgis/lwgeom_inout.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

#include <cstring>
#include <string>

namespace duckdb {

GSERIALIZED *LWGEOM_getGserialized(const void *base, size_t size) {
	GSERIALIZED *ret;
	LWGEOM *lwgeom = lwgeom_from_wkb(static_cast<const uint8_t *>(base), size, LW_PARSER_CHECK_NONE);
	ret = geometry_serialize(lwgeom);
	lwgeom_free(lwgeom);
	return ret;
}

size_t LWGEOM_size(GSERIALIZED *gser) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(gser);
	if (lwgeom == NULL) {
		return 0;
	}

	auto buf_size = lwgeom_to_wkb_size(lwgeom, WKB_EXTENDED);
	lwgeom_free(lwgeom);
	return buf_size;
}

char *LWGEOM_base(GSERIALIZED *gser) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(gser);
	if (lwgeom == NULL) {
		return nullptr;
	}

	auto buffer = lwgeom_to_wkb_buffer(lwgeom, WKB_EXTENDED);
	lwgeom_free(lwgeom);
	return (char *)buffer;
}

// std::string LWGEOM_asText(const void *base, size_t size, size_t max_digits) {
// 	std::string rstr = "";
// 	LWGEOM *lwgeom = lwgeom_from_wkb(static_cast<const uint8_t *>(base), size, LW_PARSER_CHECK_NONE);
// 	// size_t wkt_size;
// 	// rstr = lwgeom_to_wkt(lwgeom, WKT_EXTENDED, WKT_PRECISION, &wkt_size);
// 	auto text = lwgeom_to_wkt_varlena(lwgeom, WKT_ISO, max_digits);
// 	lwgeom_free(lwgeom);
// 	std::string ret = std::string(text->data);
// 	lwfree(text);
// 	return ret;
// }

std::string LWGEOM_asText(GSERIALIZED *geom, size_t dbl_dig_for_wkt) {
	LWGEOM *lwgeom = lwgeom_from_gserialized(geom);

	std::string rstr = "";
	size_t wkt_size;
	rstr = lwgeom_to_wkt(lwgeom, WKT_ISO, dbl_dig_for_wkt, &wkt_size);

	lwgeom_free(lwgeom);
	return rstr;
}

lwvarlena_t *LWGEOM_asBinary(GSERIALIZED *geom, string text) {
	LWGEOM *lwgeom;
	uint8_t variant = WKB_ISO;

	/* Get a 2D version of the geometry */
	lwgeom = lwgeom_from_gserialized(geom);

	/* If user specified endianness, respect it */
	if (text != "") {
		if (!strncmp(text.c_str(), "xdr", 3) || !strncmp(text.c_str(), "XDR", 3)) {
			variant = variant | WKB_XDR;
		} else {
			variant = variant | WKB_NDR;
		}
	}

	/* Write to WKB and free the geometry */
	auto binary = lwgeom_to_wkb_varlena(lwgeom, variant);
	lwgeom_free(lwgeom);
	return binary;
}

std::string LWGEOM_asBinary(const void *base, size_t size) {
	std::string rstr = "";
	LWGEOM *lwgeom = lwgeom_from_wkb(static_cast<const uint8_t *>(base), size, LW_PARSER_CHECK_NONE);
	rstr = lwgeom_to_hexwkb_buffer(lwgeom, WKB_NDR | WKB_EXTENDED);
	lwgeom_free(lwgeom);
	return rstr;
}

std::string LWGEOM_asGeoJson(const void *base, size_t size) {
	std::string rstr = "";
	LWGEOM *lwgeom = lwgeom_from_wkb(static_cast<const uint8_t *>(base), size, LW_PARSER_CHECK_NONE);
	auto varlen = lwgeom_to_geojson(lwgeom, nullptr, OUT_DEFAULT_DECIMAL_DIGITS, 0);
	if (!varlen) {
		return rstr;
	}
	rstr = std::string(varlen->data);
	lwfree(varlen);
	lwgeom_free(lwgeom);
	return rstr;
}

void LWGEOM_free(GSERIALIZED *gser) {
	if (gser) {
		lwfree(gser);
	}
}

} // namespace duckdb
