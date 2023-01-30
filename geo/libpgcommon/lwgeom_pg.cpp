/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 *
 * http://postgis.net
 *
 * Copyright (C) 2011      Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2009-2011 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2008      Mark Cave-Ayland <mark.cave-ayland@siriusit.co.uk>
 * Copyright (C) 2004-2007 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 **********************************************************************/

#include "libpgcommon/lwgeom_pg.hpp"

#include "liblwgeom/gserialized.hpp"

namespace duckdb {

/**
 * Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *geometry_serialize(LWGEOM *lwgeom) {
	size_t ret_size;
	GSERIALIZED *g;

	g = gserialized_from_lwgeom(lwgeom, &ret_size);
	SET_VARSIZE(g, ret_size);
	return g;
}

/**
 * Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *geography_serialize(LWGEOM *lwgeom) {
	size_t ret_size;
	GSERIALIZED *g;
	/** force to geodetic in case it's not **/
	lwgeom_set_geodetic(lwgeom, true);

	g = gserialized_from_lwgeom(lwgeom, &ret_size);
	SET_VARSIZE(g, ret_size);
	return g;
}

} // namespace duckdb
