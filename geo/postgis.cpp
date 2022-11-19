#include "postgis.hpp"

#include "postgis/geography_centroid.hpp"
#include "postgis/geography_measurement.hpp"
#include "postgis/lwgeom_export.hpp"
#include "postgis/lwgeom_functions_basic.hpp"
#include "postgis/lwgeom_geos.hpp"
#include "postgis/lwgeom_inout.hpp"
#include "postgis/lwgeom_ogc.hpp"
#include "postgis/lwgeom_dump.hpp"
#include "postgis/lwgeom_in_geohash.hpp"

namespace duckdb {
Postgis::Postgis() {
}

Postgis::~Postgis() {
}

GSERIALIZED *Postgis::LWGEOM_in(char *input) {
	return duckdb::LWGEOM_in(input);
}

GSERIALIZED *Postgis::LWGEOM_getGserialized(const void *base, size_t size) {
	return duckdb::LWGEOM_getGserialized(base, size);
}

char *Postgis::LWGEOM_base(GSERIALIZED *gser) {
	return duckdb::LWGEOM_base(gser);
}

string Postgis::LWGEOM_asBinary(const void *data, size_t size) {
	return duckdb::LWGEOM_asBinary(data, size);
}

lwvarlena_t *Postgis::LWGEOM_asBinary(GSERIALIZED *gser, string text) {
	return duckdb::LWGEOM_asBinary(gser, text);
}

string Postgis::LWGEOM_asText(const void *data, size_t size, size_t max_digits) {
	return duckdb::LWGEOM_asText(data, size, max_digits);
}

lwvarlena_t *Postgis::LWGEOM_asGeoJson(GSERIALIZED *gser, size_t m_dec_digits) {
	return duckdb::LWGEOM_asGeoJson(gser, m_dec_digits);
}

string Postgis::LWGEOM_asGeoJson(const void *data, size_t size) {
	return duckdb::LWGEOM_asGeoJson(data, size);
}

lwvarlena_t *Postgis::ST_GeoHash(GSERIALIZED *gser, size_t m_chars) {
	return duckdb::ST_GeoHash(gser, m_chars);
}

idx_t Postgis::LWGEOM_size(GSERIALIZED *gser) {
	return duckdb::LWGEOM_size(gser);
}

void Postgis::LWGEOM_free(GSERIALIZED *gser) {
	duckdb::LWGEOM_free(gser);
}

GSERIALIZED *Postgis::LWGEOM_makepoint(double x, double y) {
	return duckdb::LWGEOM_makepoint(x, y);
}

GSERIALIZED *Postgis::LWGEOM_makepoint(double x, double y, double z) {
	return duckdb::LWGEOM_makepoint(x, y, z);
}

GSERIALIZED *Postgis::LWGEOM_makeline(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::LWGEOM_makeline(geom1, geom2);
}

GSERIALIZED *Postgis::LWGEOM_makeline_garray(GSERIALIZED *gserArray[], int nelems) {
	return duckdb::LWGEOM_makeline_garray(gserArray, nelems);
}

GSERIALIZED *Postgis::LWGEOM_makepoly(GSERIALIZED *geom, GSERIALIZED *gserArray[], int nelems) {
	return duckdb::LWGEOM_makepoly(geom, gserArray, nelems);
}

GSERIALIZED *Postgis::geom_from_geojson(char *json) {
	return duckdb::geom_from_geojson(json);
}

GSERIALIZED *Postgis::LWGEOM_from_text(char *text, int srid) {
	return duckdb::LWGEOM_from_text(text, srid);
}

GSERIALIZED *Postgis::LWGEOM_from_WKB(const char *bytea_wkb, size_t byte_size, int srid) {
	return duckdb::LWGEOM_from_WKB(bytea_wkb, byte_size, srid);
}

GSERIALIZED *Postgis::LWGEOM_from_GeoHash(char *hash, int precision) {
	return duckdb::LWGEOM_from_GeoHash(hash, precision);
}

GSERIALIZED *Postgis::LWGEOM_boundary(GSERIALIZED *geom) {
	return duckdb::LWGEOM_boundary(geom);
}

int Postgis::LWGEOM_dimension(GSERIALIZED *geom) {
	return duckdb::LWGEOM_dimension(geom);
}

std::vector<GSERIALIZED *> Postgis::LWGEOM_dump(GSERIALIZED *geom) {
	return duckdb::LWGEOM_dump(geom);
}

double Postgis::ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_distance(geom1, geom2);
}

double Postgis::geography_distance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid) {
	return duckdb::geography_distance(geom1, geom2, use_spheroid);
}

double Postgis::LWGEOM_x_point(const void *data, size_t size) {
	return duckdb::LWGEOM_x_point(data, size);
}

GSERIALIZED *Postgis::centroid(GSERIALIZED *geom) {
	return duckdb::centroid(geom);
}

GSERIALIZED *Postgis::geography_centroid(GSERIALIZED *geom, bool use_spheroid) {
	return duckdb::geography_centroid(geom, use_spheroid);
}

} // namespace duckdb
