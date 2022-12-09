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

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {
/*
 * LWGEOM_in(cstring)
 * format is '[SRID=#;]wkt|wkb'
 *  LWGEOM_in( 'SRID=99;POINT(0 0)')
 *  LWGEOM_in( 'POINT(0 0)')            --> assumes SRID=SRID_UNKNOWN
 *  LWGEOM_in( 'SRID=99;0101000000000000000000F03F000000000000004')
 *  LWGEOM_in( '0101000000000000000000F03F000000000000004')
 *  LWGEOM_in( '{"type":"Point","coordinates":[1,1]}')
 *  returns a GSERIALIZED object
 */
GSERIALIZED *LWGEOM_in(char *input);
GSERIALIZED *LWGEOM_getGserialized(const void *base, size_t size);

GSERIALIZED *geom_from_geojson(char *json);
size_t LWGEOM_size(GSERIALIZED *gser);
char *LWGEOM_base(GSERIALIZED *gser);
lwvarlena_t *LWGEOM_asBinary(GSERIALIZED *gser, string text = "");
std::string LWGEOM_asBinary(const void *base, size_t size);
std::string LWGEOM_asText(GSERIALIZED *gser, size_t max_digits = OUT_DEFAULT_DECIMAL_DIGITS);
std::string LWGEOM_asGeoJson(const void *base, size_t size);
void LWGEOM_free(GSERIALIZED *gser);

} // namespace duckdb
