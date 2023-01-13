#pragma once

#include "duckdb/common/constants.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace duckdb {

class Postgis {
public:
	Postgis();
	~Postgis();

public:
	GSERIALIZED *LWGEOM_in(char *input);
	GSERIALIZED *LWGEOM_getGserialized(const void *base, size_t size);
	idx_t LWGEOM_size(GSERIALIZED *gser);
	char *LWGEOM_base(GSERIALIZED *gser);
	string LWGEOM_asBinary(const void *data, size_t size);
	lwvarlena_t *LWGEOM_asBinary(GSERIALIZED *gser, string text = "");
	string LWGEOM_asText(GSERIALIZED *gser, size_t max_digits = OUT_DEFAULT_DECIMAL_DIGITS);
	lwvarlena_t *LWGEOM_asGeoJson(GSERIALIZED *gser, size_t m_dec_digits = OUT_DEFAULT_DECIMAL_DIGITS);
	string LWGEOM_asGeoJson(const void *data, size_t size);
	lwvarlena_t *ST_GeoHash(GSERIALIZED *gser, size_t m_chars = 0);
	void LWGEOM_free(GSERIALIZED *gser);

	GSERIALIZED *LWGEOM_makepoint(double x, double y);
	GSERIALIZED *LWGEOM_makepoint(double x, double y, double z);
	GSERIALIZED *LWGEOM_makeline(GSERIALIZED *geom1, GSERIALIZED *geom2);
	GSERIALIZED *LWGEOM_makeline_garray(GSERIALIZED *gserArray[], int nelems);
	GSERIALIZED *LWGEOM_makepoly(GSERIALIZED *geom, GSERIALIZED *gserArray[] = {}, int nelems = 0);
	GSERIALIZED *geom_from_geojson(char *input);
	GSERIALIZED *LWGEOM_from_text(char *text, int srid = 0);
	GSERIALIZED *LWGEOM_from_WKB(const char *bytea_wkb, size_t byte_size, int srid = 0);
	GSERIALIZED *LWGEOM_from_GeoHash(char *input, int precision = -1);

	GSERIALIZED *LWGEOM_boundary(GSERIALIZED *geom);
	GSERIALIZED *ST_Difference(GSERIALIZED *geom1, GSERIALIZED *geom2);
	GSERIALIZED *LWGEOM_closestpoint(GSERIALIZED *geom1, GSERIALIZED *geom2);
	GSERIALIZED *ST_Union(GSERIALIZED *geom1, GSERIALIZED *geom2);
	GSERIALIZED *pgis_union_geometry_array(GSERIALIZED *gserArray[], int nelems);
	GSERIALIZED *ST_Intersection(GSERIALIZED *geom1, GSERIALIZED *geom2);
	GSERIALIZED *LWGEOM_simplify2d(GSERIALIZED *geom, double dist);
	GSERIALIZED *convexhull(GSERIALIZED *geom);
	GSERIALIZED *LWGEOM_snaptogrid(GSERIALIZED *geom, double size);
	GSERIALIZED *buffer(GSERIALIZED *geom, double radius, string styles_text = "");

	bool ST_Equals(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool contains(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool touches(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool within(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool ST_Intersects(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool covers(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool coveredby(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool disjoint(GSERIALIZED *geom1, GSERIALIZED *geom2);
	bool LWGEOM_dwithin(GSERIALIZED *geom1, GSERIALIZED *geom2, double distance);

	double ST_Area(GSERIALIZED *geom);
	double geography_area(GSERIALIZED *geom, bool use_spheroid);
	double LWGEOM_angle(GSERIALIZED *geom1, GSERIALIZED *geom2, GSERIALIZED *geom3);
	double LWGEOM_perimeter2d_poly(GSERIALIZED *geom);
	double geography_perimeter(GSERIALIZED *geom, bool use_spheroid);
	double LWGEOM_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2);
	double geography_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2);
	double LWGEOM_length2d_linestring(GSERIALIZED *geom);
	double geography_length(GSERIALIZED *geom, bool use_spheroid);
	GSERIALIZED *LWGEOM_envelope(GSERIALIZED *geom);
	double LWGEOM_maxdistance2d_linestring(GSERIALIZED *geom1, GSERIALIZED *geom2);
	double geography_maxdistance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid);
	GSERIALIZED *LWGEOM_envelope_garray(GSERIALIZED *gserArray[], int nelems);

	std::vector<int> ST_ClusterDBSCAN(GSERIALIZED *gserArray[], int nelems, double tolerance, int minpoints);

	int LWGEOM_dimension(GSERIALIZED *geom);
	std::vector<GSERIALIZED *> LWGEOM_dump(GSERIALIZED *geom);
	GSERIALIZED *LWGEOM_endpoint_linestring(GSERIALIZED *geom);
	std::string geometry_geometrytype(GSERIALIZED *geom);
	bool LWGEOM_isclosed(GSERIALIZED *geom);
	bool ST_IsCollection(GSERIALIZED *geom);
	bool LWGEOM_isempty(GSERIALIZED *geom);
	bool LWGEOM_isring(GSERIALIZED *geom);
	int LWGEOM_npoints(GSERIALIZED *geom);
	int LWGEOM_numgeometries_collection(GSERIALIZED *geom);
	int LWGEOM_numpoints_linestring(GSERIALIZED *geom);
	GSERIALIZED *LWGEOM_pointn_linestring(GSERIALIZED *geom, int index);
	GSERIALIZED *LWGEOM_startpoint_linestring(GSERIALIZED *geom);
	double LWGEOM_x_point(GSERIALIZED *geom);
	double LWGEOM_y_point(GSERIALIZED *geom);

	double ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2);
	double geography_distance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid);
	GSERIALIZED *centroid(GSERIALIZED *geom);
	GSERIALIZED *geography_centroid(GSERIALIZED *geom, bool use_spheroid);
};
} // namespace duckdb
