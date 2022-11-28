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
 * Copyright 2009-2011 Olivier Courtin <olivier.courtin@oslandia.com>
 *
 **********************************************************************/

#include "postgis/lwgeom_export.hpp"

#include "liblwgeom/gserialized.hpp"

namespace duckdb {

lwvarlena_t *LWGEOM_asGeoJson(GSERIALIZED *geom, size_t m_dec_digits) {
	LWGEOM *lwgeom;
	int precision = m_dec_digits;
	int output_bbox = LW_FALSE;
	const char *srs = NULL;
	// int32_t srid;
	// srid = gserialized_get_srid(geom);

	lwgeom = lwgeom_from_gserialized(geom);
	auto geojson = lwgeom_to_geojson(lwgeom, srs, precision, output_bbox);
	lwgeom_free(lwgeom);
	return geojson;
}

} // namespace duckdb
