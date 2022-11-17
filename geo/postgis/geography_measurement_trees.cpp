#include "postgis/geography_measurement_trees.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/lwgeodetic_tree.hpp"

namespace duckdb {

static int CircTreePIP(const CIRC_NODE *tree1, const GSERIALIZED *g1, const POINT4D *in_point) {
	// int tree1_type = gserialized_get_type(g1);
	// GBOX gbox1;
	// GEOGRAPHIC_POINT in_gpoint;
	// POINT3D in_point3d;

	// Need to do with postgis

	/* If the tree'ed argument is a polygon, do the P-i-P using the tree-based P-i-P */
	// if ( tree1_type == POLYGONTYPE || tree1_type == MULTIPOLYGONTYPE )
	// {
	// 	/* Need a gbox to calculate an outside point */
	// 	if ( LW_FAILURE == gserialized_get_gbox_p(g1, &gbox1) )
	// 	{
	// 		LWGEOM* lwgeom1 = lwgeom_from_gserialized(g1);
	// 		POSTGIS_DEBUG(3, "unable to read gbox from gserialized, calculating from scratch");
	// 		lwgeom_calculate_gbox_geodetic(lwgeom1, &gbox1);
	// 		lwgeom_free(lwgeom1);
	// 	}

	// 	/* Flip the candidate point into geographics */
	// 	geographic_point_init(in_point->x, in_point->y, &in_gpoint);
	// 	geog2cart(&in_gpoint, &in_point3d);

	// 	/* If the candidate isn't in the tree box, it's not in the tree area */
	// 	if ( ! gbox_contains_point3d(&gbox1, &in_point3d) )
	// 	{
	// 		POSTGIS_DEBUG(3, "in_point3d is not inside the tree gbox, CircTreePIP returning FALSE");
	// 		return LW_FALSE;
	// 	}
	// 	/* The candidate point is in the box, so it *might* be inside the tree */
	// 	else
	// 	{
	// 		POINT2D pt2d_outside; /* latlon */
	// 		POINT2D pt2d_inside;
	// 		pt2d_inside.x = in_point->x;
	// 		pt2d_inside.y = in_point->y;
	// 		/* Calculate a definitive outside point */
	// 		if (gbox_pt_outside(&gbox1, &pt2d_outside) == LW_FAILURE)
	// 			if (circ_tree_get_point_outside(tree1, &pt2d_outside) == LW_FAILURE)
	// 				lwpgerror("%s: Unable to generate outside point!", __func__);

	// 		POSTGIS_DEBUGF(3, "p2d_inside=POINT(%g %g) p2d_outside=POINT(%g %g)", pt2d_inside.x, pt2d_inside.y,
	// pt2d_outside.x, pt2d_outside.y);
	// 		/* Test the candidate point for strict containment */
	// 		POSTGIS_DEBUG(3, "calling circ_tree_contains_point for PiP test");
	// 		return circ_tree_contains_point(tree1, &pt2d_inside, &pt2d_outside, 0, NULL);
	// 	}
	// }
	// else
	{ return LW_FALSE; }
}

int geography_tree_distance(const GSERIALIZED *g1, const GSERIALIZED *g2, const SPHEROID *s, double tolerance,
                            double *distance) {
	CIRC_NODE *circ_tree1 = NULL;
	CIRC_NODE *circ_tree2 = NULL;
	LWGEOM *lwgeom1 = NULL;
	LWGEOM *lwgeom2 = NULL;
	POINT4D pt1, pt2;

	lwgeom1 = lwgeom_from_gserialized(g1);
	lwgeom2 = lwgeom_from_gserialized(g2);
	circ_tree1 = lwgeom_calculate_circ_tree(lwgeom1);
	circ_tree2 = lwgeom_calculate_circ_tree(lwgeom2);
	lwgeom_startpoint(lwgeom1, &pt1);
	lwgeom_startpoint(lwgeom2, &pt2);

	if (CircTreePIP(circ_tree1, g1, &pt2) || CircTreePIP(circ_tree2, g2, &pt1)) {
		*distance = 0.0;
	} else {
		/* Calculate tree/tree distance */
		*distance = circ_tree_distance_tree(circ_tree1, circ_tree2, s, tolerance);
	}

	circ_tree_free(circ_tree1);
	circ_tree_free(circ_tree2);
	lwgeom_free(lwgeom1);
	lwgeom_free(lwgeom2);
	return LW_SUCCESS;
}

} // namespace duckdb