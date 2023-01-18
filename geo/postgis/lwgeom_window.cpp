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

#include "postgis/lwgeom_window.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/lwgeom_geos.hpp"
#include "liblwgeom/lwunionfind.hpp"
#include "postgis/lwgeom_geos.hpp"

namespace duckdb {

std::vector<int> ST_ClusterDBSCAN(GSERIALIZED *gserArray[], int ngeoms, double tolerance, int minpoints) {
	if (ngeoms <= 0) {
		return {};
	}
	uint32_t i;
	uint32_t *result_ids;
	LWGEOM **geoms;
	char *is_in_cluster = NULL;
	UNIONFIND *uf;
	std::vector<int> clusters(ngeoms, -1);

	bool is_error = LW_TRUE; /* until proven otherwise */

	/* Validate input parameters */
	if (tolerance < 0) {
		lwerror("Tolerance must be a positive number", tolerance);
		return {};
	}
	if (minpoints < 0) {
		lwerror("Minpoints must be a positive number", minpoints);
	}

	initGEOS(lwnotice, lwgeom_geos_error);
	geoms = (LWGEOM **)lwalloc(ngeoms * sizeof(LWGEOM *));
	uf = UF_create(ngeoms);
	for (i = 0; i < ngeoms; i++) {
		geoms[i] = lwgeom_from_gserialized(gserArray[i]);

		if (!geoms[i]) {
			/* TODO release memory ? */
			lwerror("Error reading geometry.");
			return {};
		}
	}

	if (union_dbscan(geoms, ngeoms, uf, tolerance, minpoints, minpoints > 1 ? &is_in_cluster : NULL) == LW_SUCCESS)
		is_error = LW_FALSE;

	for (i = 0; i < ngeoms; i++) {
		lwgeom_free(geoms[i]);
	}
	lwfree(geoms);

	if (is_error) {
		UF_destroy(uf);
		if (is_in_cluster)
			lwfree(is_in_cluster);
		lwerror("Error during clustering");
		return {};
	}

	result_ids = UF_get_collapsed_cluster_ids(uf, is_in_cluster);
	for (i = 0; i < ngeoms; i++) {
		if (minpoints > 1 && !is_in_cluster[i]) {
			clusters[i] = -1;
		} else {
			clusters[i] = result_ids[i];
		}
	}

	lwfree(result_ids);
	UF_destroy(uf);

	return clusters;
}

} // namespace duckdb
