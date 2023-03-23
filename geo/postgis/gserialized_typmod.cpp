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
 * Copyright 2009 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 **********************************************************************/

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwgeodetic.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

namespace duckdb {

/**
 * Check the consistency of the metadata we want to enforce in the typmod:
 * srid, type and dimensionality. If things are inconsistent, shut down the query.
 */
GSERIALIZED *postgis_valid_typmod(GSERIALIZED *gser, int32_t typmod) {
	int32_t geom_srid = gserialized_get_srid(gser);
	int32_t geom_type = gserialized_get_type(gser);
	int32_t geom_z = gserialized_has_z(gser);
	int32_t geom_m = gserialized_has_m(gser);
	int32_t typmod_srid = TYPMOD_GET_SRID(typmod);
	int32_t typmod_type = TYPMOD_GET_TYPE(typmod);
	int32_t typmod_z = TYPMOD_GET_Z(typmod);
	int32_t typmod_m = TYPMOD_GET_M(typmod);

	/* No typmod (-1) => no preferences */
	if (typmod < 0)
		return gser;

	/*
	 * #3031: If a user is handing us a MULTIPOINT EMPTY but trying to fit it into
	 * a POINT geometry column, there's a strong chance the reason she has
	 * a MULTIPOINT EMPTY because we gave it to her during data dump,
	 * converting the internal POINT EMPTY into a EWKB MULTIPOINT EMPTY
	 * (because EWKB doesn't have a clean way to represent POINT EMPTY).
	 * In such a case, it makes sense to turn the MULTIPOINT EMPTY back into a
	 * point EMPTY, rather than throwing an error.
	 */
	if (typmod_type == POINTTYPE && geom_type == MULTIPOINTTYPE && gserialized_is_empty(gser)) {
		LWPOINT *empty_point = lwpoint_construct_empty(geom_srid, geom_z, geom_m);
		geom_type = POINTTYPE;
		lwfree(gser);
		if (gserialized_is_geodetic(gser))
			gser = geography_serialize(lwpoint_as_lwgeom(empty_point));
		else
			gser = geometry_serialize(lwpoint_as_lwgeom(empty_point));
	}

	/* Typmod has a preference for SRID, but geometry does not? Harmonize the geometry SRID. */
	if (typmod_srid > 0 && geom_srid == 0) {
		gserialized_set_srid(gser, typmod_srid);
		geom_srid = typmod_srid;
	}

	/* Typmod has a preference for SRID? Geometry SRID had better match. */
	if (typmod_srid > 0 && typmod_srid != geom_srid) {
		throw "Geometry SRID (" + std::to_string(geom_srid) + ") does not match column SRID (" +
		    std::to_string(typmod_srid) + ")";
	}

	/* Typmod has a preference for geometry type. */
	if (typmod_type > 0 &&
	    /* GEOMETRYCOLLECTION column can hold any kind of collection */
	    ((typmod_type == COLLECTIONTYPE && !(geom_type == COLLECTIONTYPE || geom_type == MULTIPOLYGONTYPE ||
	                                         geom_type == MULTIPOINTTYPE || geom_type == MULTILINETYPE)) ||
	     /* Other types must be strictly equal. */
	     (typmod_type != geom_type))) {
		throw "Geometry type (" + std::string(lwtype_name(geom_type)) + ") does not match column type (" +
		    std::string(lwtype_name(typmod_type)) + ")";
	}

	/* Mismatched Z dimensionality. */
	if (typmod_z && !geom_z) {
		throw "Column has Z dimension but geometry does not";
	}

	/* Mismatched Z dimensionality (other way). */
	if (geom_z && !typmod_z) {
		throw "Geometry has Z dimension but column does not";
	}

	/* Mismatched M dimensionality. */
	if (typmod_m && !geom_m) {
		throw "Column has M dimension but geometry does not";
	}

	/* Mismatched M dimensionality (other way). */
	if (geom_m && !typmod_m) {
		throw "Geometry has M dimension but column does not";
	}

	return gser;
}

} // namespace duckdb
