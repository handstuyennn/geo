#include "postgis.hpp"

#include "postgis/geography_centroid.hpp"
#include "postgis/geography_measurement.hpp"
#include "postgis/lwgeom_dump.hpp"
#include "postgis/lwgeom_export.hpp"
#include "postgis/lwgeom_functions_analytic.hpp"
#include "postgis/lwgeom_functions_basic.hpp"
#include "postgis/lwgeom_geos.hpp"
#include "postgis/lwgeom_in_geohash.hpp"
#include "postgis/lwgeom_inout.hpp"
#include "postgis/lwgeom_ogc.hpp"
#include "postgis/lwgeom_window.hpp"

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

string Postgis::LWGEOM_asText(GSERIALIZED *gser, size_t max_digits) {
	return duckdb::LWGEOM_asText(gser, max_digits);
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

GSERIALIZED *Postgis::ST_Difference(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_Difference(geom1, geom2);
}

GSERIALIZED *Postgis::LWGEOM_closestpoint(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::LWGEOM_closestpoint(geom1, geom2);
}

GSERIALIZED *Postgis::ST_Union(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_Union(geom1, geom2);
}

GSERIALIZED *Postgis::pgis_union_geometry_array(GSERIALIZED *gserArray[], int nelems) {
	return duckdb::pgis_union_geometry_array(gserArray, nelems);
}

GSERIALIZED *Postgis::ST_Intersection(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_Intersection(geom1, geom2);
}

GSERIALIZED *Postgis::LWGEOM_simplify2d(GSERIALIZED *geom, double dist) {
	return duckdb::LWGEOM_simplify2d(geom, dist);
}

GSERIALIZED *Postgis::convexhull(GSERIALIZED *geom) {
	return duckdb::convexhull(geom);
}

GSERIALIZED *Postgis::LWGEOM_snaptogrid(GSERIALIZED *geom, double size) {
	return duckdb::LWGEOM_snaptogrid(geom, 0, 0, size, size);
}

GSERIALIZED *Postgis::buffer(GSERIALIZED *geom, double radius, string styles_text) {
	return duckdb::buffer(geom, radius, styles_text);
}

bool Postgis::ST_Equals(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_Equals(geom1, geom2);
}

bool Postgis::contains(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::contains(geom1, geom2);
}

bool Postgis::touches(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::touches(geom1, geom2);
}

bool Postgis::within(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::contains(geom2, geom1);
}

bool Postgis::ST_Intersects(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_Intersects(geom1, geom2);
}

bool Postgis::covers(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::covers(geom1, geom2);
}

bool Postgis::coveredby(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::coveredby(geom1, geom2);
}

bool Postgis::disjoint(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::disjoint(geom1, geom2);
}

bool Postgis::LWGEOM_dwithin(GSERIALIZED *geom1, GSERIALIZED *geom2, double distance) {
	return duckdb::LWGEOM_dwithin(geom1, geom2, distance);
}

double Postgis::ST_Area(GSERIALIZED *geom) {
	return duckdb::ST_Area(geom);
}

double Postgis::geography_area(GSERIALIZED *geom, bool use_spheroid) {
	return duckdb::geography_area(geom, use_spheroid);
}

double Postgis::LWGEOM_angle(GSERIALIZED *geom1, GSERIALIZED *geom2, GSERIALIZED *geom3) {
	return duckdb::LWGEOM_angle(geom1, geom2, geom3);
}

double Postgis::LWGEOM_perimeter2d_poly(GSERIALIZED *geom) {
	return duckdb::LWGEOM_perimeter2d_poly(geom);
}

double Postgis::geography_perimeter(GSERIALIZED *geom, bool use_spheroid) {
	return duckdb::geography_perimeter(geom, use_spheroid);
}

double Postgis::LWGEOM_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::LWGEOM_azimuth(geom1, geom2);
}

double Postgis::geography_azimuth(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::geography_azimuth(geom1, geom2);
}

double Postgis::LWGEOM_length2d_linestring(GSERIALIZED *geom) {
	return duckdb::LWGEOM_length2d_linestring(geom);
}

double Postgis::geography_length(GSERIALIZED *geom, bool use_spheroid) {
	return duckdb::geography_length(geom, use_spheroid);
}

GSERIALIZED *Postgis::LWGEOM_envelope(GSERIALIZED *geom) {
	return duckdb::LWGEOM_envelope(geom);
}

double Postgis::LWGEOM_maxdistance2d_linestring(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::LWGEOM_maxdistance2d_linestring(geom1, geom2);
}

double Postgis::geography_maxdistance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid) {
	return duckdb::geography_maxdistance(geom1, geom2, use_spheroid);
}

GSERIALIZED *Postgis::LWGEOM_envelope_garray(GSERIALIZED *gserArray[], int nelems) {
	return duckdb::LWGEOM_envelope_garray(gserArray, nelems);
}

std::vector<int> Postgis::ST_ClusterDBSCAN(GSERIALIZED *gserArray[], int nelems, double tolerance, int minpoints) {
	return duckdb::ST_ClusterDBSCAN(gserArray, nelems, tolerance, minpoints);
}

int Postgis::LWGEOM_dimension(GSERIALIZED *geom) {
	return duckdb::LWGEOM_dimension(geom);
}

std::vector<GSERIALIZED *> Postgis::LWGEOM_dump(GSERIALIZED *geom) {
	return duckdb::LWGEOM_dump(geom);
}

GSERIALIZED *Postgis::LWGEOM_endpoint_linestring(GSERIALIZED *geom) {
	return duckdb::LWGEOM_endpoint_linestring(geom);
}

std::string Postgis::geometry_geometrytype(GSERIALIZED *geom) {
	return duckdb::geometry_geometrytype(geom);
}

bool Postgis::LWGEOM_isclosed(GSERIALIZED *geom) {
	return duckdb::LWGEOM_isclosed(geom);
}

bool Postgis::ST_IsCollection(GSERIALIZED *geom) {
	return duckdb::ST_IsCollection(geom);
}

bool Postgis::LWGEOM_isempty(GSERIALIZED *geom) {
	return duckdb::LWGEOM_isempty(geom);
}

bool Postgis::LWGEOM_isring(GSERIALIZED *geom) {
	return duckdb::LWGEOM_isring(geom);
}

int Postgis::LWGEOM_npoints(GSERIALIZED *geom) {
	return duckdb::LWGEOM_npoints(geom);
}

int Postgis::LWGEOM_numgeometries_collection(GSERIALIZED *geom) {
	return duckdb::LWGEOM_numgeometries_collection(geom);
}

int Postgis::LWGEOM_numpoints_linestring(GSERIALIZED *geom) {
	return duckdb::LWGEOM_numpoints_linestring(geom);
}

GSERIALIZED *Postgis::LWGEOM_pointn_linestring(GSERIALIZED *geom, int index) {
	return duckdb::LWGEOM_pointn_linestring(geom, index);
}

GSERIALIZED *Postgis::LWGEOM_startpoint_linestring(GSERIALIZED *geom) {
	return duckdb::LWGEOM_startpoint_linestring(geom);
}

double Postgis::LWGEOM_x_point(GSERIALIZED *geom) {
	return duckdb::LWGEOM_x_point(geom);
}

double Postgis::LWGEOM_y_point(GSERIALIZED *geom) {
	return duckdb::LWGEOM_y_point(geom);
}

double Postgis::ST_distance(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	return duckdb::ST_distance(geom1, geom2);
}

double Postgis::geography_distance(GSERIALIZED *geom1, GSERIALIZED *geom2, bool use_spheroid) {
	return duckdb::geography_distance(geom1, geom2, use_spheroid);
}

GSERIALIZED *Postgis::centroid(GSERIALIZED *geom) {
	return duckdb::centroid(geom);
}

GSERIALIZED *Postgis::geography_centroid(GSERIALIZED *geom, bool use_spheroid) {
	return duckdb::geography_centroid(geom, use_spheroid);
}

} // namespace duckdb
