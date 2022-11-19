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
	string LWGEOM_asText(const void *data, size_t size, size_t max_digits = OUT_DEFAULT_DECIMAL_DIGITS);
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

	int LWGEOM_dimension(GSERIALIZED *geom);
	std::vector<GSERIALIZED *> LWGEOM_dump(GSERIALIZED *geom);

	double ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2);
	double geography_distance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid);
	double LWGEOM_x_point(const void *data, size_t size);
	GSERIALIZED *centroid(GSERIALIZED *geom);
	GSERIALIZED *geography_centroid(GSERIALIZED *geom, bool use_spheroid);
};
} // namespace duckdb
