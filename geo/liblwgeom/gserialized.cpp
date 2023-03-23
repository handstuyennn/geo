#include "liblwgeom/gserialized.hpp"

#include "liblwgeom/gserialized1.hpp"
#include "liblwgeom/gserialized2.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

namespace duckdb {

/* First four bits don't change between v0 and v1 */
#define GFLAG_Z        0x01
#define GFLAG_M        0x02
#define GFLAG_BBOX     0x04
#define GFLAG_GEODETIC 0x08
/* v1 and v2 MUST share the same version bits */
#define GFLAG_VER_0                0x40
#define GFLAGS_GET_VERSION(gflags) (((gflags)&GFLAG_VER_0) >> 6)

/**
 * Check if a #GSERIALIZED has a bounding box without deserializing first.
 */
int gserialized_has_bbox(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_has_bbox(g);
	else
		return gserialized1_has_bbox(g);
}

/**
 * Extract the SRID from the serialized form (it is packed into
 * three bytes so this is a handy function).
 */
int32_t gserialized_get_srid(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_get_srid(g);
	else
		return gserialized1_get_srid(g);
}

/**
 * Write the SRID into the serialized form (it is packed into
 * three bytes so this is a handy function).
 */
void gserialized_set_srid(GSERIALIZED *g, int32_t srid) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_set_srid(g, srid);
	else
		return gserialized1_set_srid(g, srid);
}

/**
 * Check if a #GSERIALIZED is empty without deserializing first.
 * Only checks if the number of elements of the parent geometry
 * is zero, will not catch collections of empty, eg:
 * GEOMETRYCOLLECTION(POINT EMPTY)
 */
int gserialized_is_empty(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_is_empty(g);
	else
		return gserialized1_is_empty(g);
}

/**
 * Allocate a new #GSERIALIZED from an #LWGEOM. For all non-point types, a bounding
 * box will be calculated and embedded in the serialization. The geodetic flag is used
 * to control the box calculation (cartesian or geocentric). If set, the size pointer
 * will contain the size of the final output, which is useful for setting the PgSQL
 * VARSIZE information.
 */
GSERIALIZED *gserialized_from_lwgeom(LWGEOM *geom, size_t *size) {
	return gserialized2_from_lwgeom(geom, size);
}

/**
 * Allocate a new #LWGEOM from a #GSERIALIZED. The resulting #LWGEOM will have coordinates
 * that are double aligned and suitable for direct reading using getPoint2d_p_ro
 */
LWGEOM *lwgeom_from_gserialized(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return lwgeom_from_gserialized2(g);
	else
		return lwgeom_from_gserialized1(g);
}

/**
 * Extract the geometry type from the serialized form (it hides in
 * the anonymous data area, so this is a handy function).
 */
uint32_t gserialized_get_type(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_get_type(g);
	else
		return gserialized1_get_type(g);
}

int gserialized_peek_first_point(const GSERIALIZED *g, POINT4D *out_point) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_peek_first_point(g, out_point);
	else
		return gserialized1_peek_first_point(g, out_point);
}

void gserialized_error_if_srid_mismatch(const GSERIALIZED *g1, const GSERIALIZED *g2, const char *funcname) {
	int32_t srid1 = gserialized_get_srid(g1);
	int32_t srid2 = gserialized_get_srid(g2);
	if (srid1 != srid2)
		lwerror("%s: Operation on mixed SRID geometries (%s, %d) != (%s, %d)", funcname,
		        lwtype_name(gserialized1_get_type(g1)), srid1, lwtype_name(gserialized_get_type(g2)), srid2);
}

void gserialized_error_if_srid_mismatch_reference(const GSERIALIZED *g1, const int32_t srid2, const char *funcname) {
	int32_t srid1 = gserialized_get_srid(g1);
	if (srid1 != srid2) {
		std::ostringstream s;
		s << funcname << ": Operation on mixed SRID geometries " << lwtype_name(gserialized1_get_type(g1)) << " "
		  << srid1 << " != " << srid2;
		std::string errormsg = s.str();
		lwerror(errormsg.c_str());
	}
}

/**
 * Check if a #GSERIALIZED has a Z ordinate.
 */
int gserialized_has_z(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_has_z(g);
	else
		return gserialized1_has_z(g);
}

/**
 * Check if a #GSERIALIZED has an M ordinate.
 */
int gserialized_has_m(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_has_m(g);
	else
		return gserialized1_has_m(g);
}

/**
 * Check if a #GSERIALIZED is a geography.
 */
int gserialized_is_geodetic(const GSERIALIZED *g) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_is_geodetic(g);
	else
		return gserialized1_is_geodetic(g);
}

/**
 * Read the box from the #GSERIALIZED or calculate it if necessary.
 * Return #LWFAILURE if box cannot be calculated (NULL or EMPTY
 * input).
 */
int gserialized_get_gbox_p(const GSERIALIZED *g, GBOX *gbox) {
	if (GFLAGS_GET_VERSION(g->gflags))
		return gserialized2_get_gbox_p(g, gbox);
	else
		return gserialized1_get_gbox_p(g, gbox);
}

} // namespace duckdb
