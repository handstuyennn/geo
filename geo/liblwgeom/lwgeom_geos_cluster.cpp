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
 * Copyright 2015-2016 Daniel Baston <dbaston@gmail.com>
 *
 **********************************************************************/

#include "liblwgeom/liblwgeom.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwgeom_geos.hpp"
#include "liblwgeom/lwinline.hpp"
#include "liblwgeom/lwunionfind.hpp"

#include <string.h>

namespace duckdb {

static const int STRTREE_NODE_CAPACITY = 10;

/* Utility struct used to accumulate items in GEOSSTRtree_query callback */
struct QueryContext {
	void **items_found;
	uint32_t num_items_found;
	uint32_t items_found_size;
};

/* Utility struct to keep GEOSSTRtree and associated structures to be freed after use */
struct STRTree {
	GEOSSTRtree *tree;
	GEOSGeometry **envelopes;
	uint32_t *geom_ids;
	uint32_t num_geoms;
};

static struct STRTree make_strtree(void **geoms, uint32_t num_geoms, char is_lwgeom);
static void destroy_strtree(struct STRTree *tree);

/* Make a minimal GEOSGeometry* whose Envelope covers the same 2D extent as
 * the supplied GBOX.  This is faster and uses less memory than building a
 * five-point polygon with GBOX2GEOS.
 */
static GEOSGeometry *geos_envelope_surrogate(const LWGEOM *g) {
	if (lwgeom_is_empty(g))
		return GEOSGeom_createEmptyPolygon();

	if (lwgeom_get_type(g) == POINTTYPE) {
		const POINT2D *pt = getPoint2d_cp(lwgeom_as_lwpoint(g)->point, 0);
		return make_geos_point(pt->x, pt->y);
	} else {
		const GBOX *box = lwgeom_get_bbox(g);
		if (!box)
			return NULL;

		return make_geos_segment(box->xmin, box->ymin, box->xmax, box->ymax);
	}
}

/** Make a GEOSSTRtree that stores a pointer to a variable containing
 *  the array index of the input geoms */
static struct STRTree make_strtree(void **geoms, uint32_t num_geoms, char is_lwgeom) {
	struct STRTree tree;
	tree.envelopes = 0;
	tree.num_geoms = 0;
	tree.geom_ids = 0;

	tree.tree = GEOSSTRtree_create(STRTREE_NODE_CAPACITY);
	if (tree.tree == NULL) {
		return tree;
	}
	tree.geom_ids = (uint32_t *)lwalloc(num_geoms * sizeof(uint32_t));
	tree.num_geoms = num_geoms;

	if (is_lwgeom) {
		uint32_t i;
		tree.envelopes = (GEOSGeometry **)lwalloc(num_geoms * sizeof(GEOSGeometry *));
		for (i = 0; i < num_geoms; i++) {
			tree.geom_ids[i] = i;
			tree.envelopes[i] = geos_envelope_surrogate((LWGEOM *)geoms[i]);
			GEOSSTRtree_insert(tree.tree, tree.envelopes[i], &(tree.geom_ids[i]));
		}
	} else {
		uint32_t i;
		tree.envelopes = NULL;
		for (i = 0; i < num_geoms; i++) {
			tree.geom_ids[i] = i;
			GEOSSTRtree_insert(tree.tree, (GEOSGeometry *)geoms[i], &(tree.geom_ids[i]));
		}
	}

	return tree;
}

/** Clean up STRTree after use */
static void destroy_strtree(struct STRTree *tree) {
	size_t i;
	GEOSSTRtree_destroy(tree->tree);

	if (tree->envelopes) {
		for (i = 0; i < tree->num_geoms; i++) {
			GEOSGeom_destroy(tree->envelopes[i]);
		}
		lwfree(tree->envelopes);
	}
	lwfree(tree->geom_ids);
}

static void query_accumulate(void *item, void *userdata) {
	struct QueryContext *cxt = (QueryContext *)userdata;
	if (!cxt->items_found) {
		cxt->items_found_size = 8;
		cxt->items_found = (void **)lwalloc(cxt->items_found_size * sizeof(void *));
	}

	if (cxt->num_items_found >= cxt->items_found_size) {
		cxt->items_found_size = 2 * cxt->items_found_size;
		cxt->items_found = (void **)lwrealloc(cxt->items_found, cxt->items_found_size * sizeof(void *));
	}
	cxt->items_found[cxt->num_items_found++] = item;
}

static int dbscan_update_context(GEOSSTRtree *tree, struct QueryContext *cxt, LWGEOM **geoms, uint32_t p, double eps) {
	cxt->num_items_found = 0;

	GEOSGeometry *query_envelope;
	if (geoms[p]->type == POINTTYPE) {
		const POINT2D *pt = getPoint2d_cp(lwgeom_as_lwpoint(geoms[p])->point, 0);
		query_envelope = make_geos_segment(pt->x - eps, pt->y - eps, pt->x + eps, pt->y + eps);
	} else {
		const GBOX *box = lwgeom_get_bbox(geoms[p]);
		query_envelope = make_geos_segment(box->xmin - eps, box->ymin - eps, box->xmax + eps, box->ymax + eps);
	}

	if (!query_envelope)
		return LW_FAILURE;

	GEOSSTRtree_query(tree, query_envelope, &query_accumulate, cxt);

	GEOSGeom_destroy(query_envelope);

	return LW_SUCCESS;
}

/* Union p's cluster with q's cluster, if q is not a border point of another cluster.
 * Applicable to DBSCAN with minpoints > 1.
 */
static void union_if_available(UNIONFIND *uf, uint32_t p, uint32_t q, char *is_in_core, char *in_a_cluster) {
	if (in_a_cluster[q]) {
		/* Can we merge p's cluster with q's cluster?  We can do this only
		 * if both p and q are considered _core_ points of their respective
		 * clusters.
		 */
		if (is_in_core[q]) {
			UF_union(uf, p, q);
		}
	} else {
		UF_union(uf, p, q);
		in_a_cluster[q] = LW_TRUE;
	}
}

/* An optimized DBSCAN union for the case where min_points == 1.
 * If min_points == 1, then we don't care how many neighbors we find; we can union clusters
 * on the fly, as as we go through the distance calculations.  This potentially allows us
 * to avoid some distance computations altogether.
 */
static int union_dbscan_minpoints_1(LWGEOM **geoms, uint32_t num_geoms, UNIONFIND *uf, double eps,
                                    char **in_a_cluster_ret) {
	uint32_t p, i;
	struct STRTree tree;
	struct QueryContext cxt = {.items_found = NULL, .num_items_found = 0, .items_found_size = 0};
	int success = LW_SUCCESS;

	if (in_a_cluster_ret) {
		char *in_a_cluster = (char *)lwalloc(num_geoms * sizeof(char));
		for (i = 0; i < num_geoms; i++)
			in_a_cluster[i] = LW_TRUE;
		*in_a_cluster_ret = in_a_cluster;
	}

	if (num_geoms <= 1)
		return LW_SUCCESS;

	tree = make_strtree((void **)geoms, num_geoms, LW_TRUE);
	if (tree.tree == NULL) {
		destroy_strtree(&tree);
		return LW_FAILURE;
	}

	for (p = 0; p < num_geoms; p++) {
		if (lwgeom_is_empty(geoms[p]))
			continue;

		dbscan_update_context(tree.tree, &cxt, geoms, p, eps);
		for (i = 0; i < cxt.num_items_found; i++) {
			uint32_t q = *((uint32_t *)cxt.items_found[i]);

			if (UF_find(uf, p) != UF_find(uf, q)) {
				double mindist = lwgeom_mindistance2d_tolerance(geoms[p], geoms[q], eps);
				if (mindist == FLT_MAX) {
					success = LW_FAILURE;
					break;
				}

				if (mindist <= eps)
					UF_union(uf, p, q);
			}
		}
	}

	if (cxt.items_found)
		lwfree(cxt.items_found);

	destroy_strtree(&tree);

	return success;
}

static int union_dbscan_general(LWGEOM **geoms, uint32_t num_geoms, UNIONFIND *uf, double eps, uint32_t min_points,
                                char **in_a_cluster_ret) {
	uint32_t p, i;
	struct STRTree tree;
	struct QueryContext cxt = {.items_found = NULL, .num_items_found = 0, .items_found_size = 0};
	int success = LW_SUCCESS;
	uint32_t *neighbors;
	char *in_a_cluster;
	char *is_in_core;

	in_a_cluster = (char *)lwalloc(num_geoms * sizeof(char));
	memset(in_a_cluster, 0, num_geoms * sizeof(char));

	if (in_a_cluster_ret)
		*in_a_cluster_ret = in_a_cluster;

	/* Bail if we don't even have enough inputs to make a cluster. */
	if (num_geoms <= min_points) {
		if (!in_a_cluster_ret)
			lwfree(in_a_cluster);
		return LW_SUCCESS;
	}

	tree = make_strtree((void **)geoms, num_geoms, LW_TRUE);
	if (tree.tree == NULL) {
		destroy_strtree(&tree);
		return LW_FAILURE;
	}

	is_in_core = (char *)lwalloc(num_geoms * sizeof(char));
	memset(is_in_core, 0, num_geoms * sizeof(char));
	neighbors = (uint32_t *)lwalloc(min_points * sizeof(uint32_t));

	for (p = 0; p < num_geoms; p++) {
		uint32_t num_neighbors = 0;

		if (lwgeom_is_empty(geoms[p]))
			continue;

		dbscan_update_context(tree.tree, &cxt, geoms, p, eps);

		/* We didn't find enough points to do anything, even if they are all within eps. */
		if (cxt.num_items_found < min_points)
			continue;

		for (i = 0; i < cxt.num_items_found; i++) {
			uint32_t q = *((uint32_t *)cxt.items_found[i]);

			if (num_neighbors >= min_points) {
				/* If we've already identified p as a core point, and it's already
				 * in the same cluster in q, then there's nothing to learn by
				 * computing the distance.
				 */
				if (UF_find(uf, p) == UF_find(uf, q))
					continue;

				/* Similarly, if q is already identifed as a border point of another
				 * cluster, there's no point figuring out what the distance is.
				 */
				if (in_a_cluster[q] && !is_in_core[q])
					continue;
			}

			double mindist = lwgeom_mindistance2d_tolerance(geoms[p], geoms[q], eps);
			if (mindist == FLT_MAX) {
				success = LW_FAILURE;
				break;
			}

			if (mindist <= eps) {
				/* If we haven't hit min_points yet, we don't know if we can union p and q.
				 * Just set q aside for now.
				 */
				if (num_neighbors < min_points) {
					neighbors[num_neighbors++] = q;

					/* If we just hit min_points, we can now union all of the neighbor geometries
					 * we've been saving.
					 */
					if (num_neighbors == min_points) {
						uint32_t j;
						is_in_core[p] = LW_TRUE;
						in_a_cluster[p] = LW_TRUE;
						for (j = 0; j < num_neighbors; j++) {
							union_if_available(uf, p, neighbors[j], is_in_core, in_a_cluster);
						}
					}
				} else {
					/* If we're above min_points, no need to store our neighbors, just go ahead
					 * and union them now.  This may allow us to cut out some distance
					 * computations.
					 */
					union_if_available(uf, p, q, is_in_core, in_a_cluster);
				}
			}
		}

		if (!success)
			break;
	}

	lwfree(neighbors);
	lwfree(is_in_core);

	/* Free in_a_cluster if we're not giving it to our caller */
	if (!in_a_cluster_ret)
		lwfree(in_a_cluster);

	if (cxt.items_found)
		lwfree(cxt.items_found);

	destroy_strtree(&tree);
	return success;
}

int union_dbscan(LWGEOM **geoms, uint32_t num_geoms, UNIONFIND *uf, double eps, uint32_t min_points,
                 char **in_a_cluster_ret) {
	if (min_points <= 1)
		return union_dbscan_minpoints_1(geoms, num_geoms, uf, eps, in_a_cluster_ret);
	else
		return union_dbscan_general(geoms, num_geoms, uf, eps, min_points, in_a_cluster_ret);
}

} // namespace duckdb
