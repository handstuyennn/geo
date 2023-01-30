/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
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

#pragma once
#include "duckdb.hpp"
#include "liblwgeom/liblwgeom.hpp"

namespace duckdb {

/**
 * Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *geometry_serialize(LWGEOM *lwgeom);

/**
 * Utility method to call the serialization and then set the
 * PgSQL varsize header appropriately with the serialized size.
 */
GSERIALIZED *geography_serialize(LWGEOM *lwgeom);

/**
 * Compare SRIDs of two GSERIALIZEDs and print informative error message if they differ.
 */
void gserialized_error_if_srid_mismatch(const GSERIALIZED *g1, const GSERIALIZED *g2, const char *funcname);

/**
 * Compare SRIDs of GSERIALIZEDs to reference and print informative error message if they differ.
 */
void gserialized_error_if_srid_mismatch_reference(const GSERIALIZED *g1, const int32_t srid, const char *funcname);

} // namespace duckdb
