#include "geometry.hpp"

#include "duckdb/common/types/vector.hpp"
#include "postgis.hpp"

namespace duckdb {

string Geometry::GetString(string_t geometry, DataFormatType ftype) {
	auto data = (const_data_ptr_t)geometry.GetDataUnsafe();
	auto len = geometry.GetSize();
	if (len == 0) {
		return "";
	}
	Postgis postgis;
	string text = "";
	switch (ftype) {
	case DataFormatType::FORMAT_VALUE_TYPE_WKB:
		text = postgis.LWGEOM_asBinary(data, len);
		break;

	case DataFormatType::FORMAT_VALUE_TYPE_WKT: {
		auto gser = Geometry::GetGserialized(geometry);
		text = postgis.LWGEOM_asText(gser);
		Geometry::DestroyGeometry(gser);
	} break;

	case DataFormatType::FORMAT_VALUE_TYPE_GEOJSON:
		text = postgis.LWGEOM_asGeoJson(data, len);
		break;

	default:
		break;
	}
	return text;
}

void Geometry::ToString(string_t geometry, char *output, DataFormatType ftype) {
	auto text = Geometry::GetString(geometry, ftype);
	auto len = text.size();
	memcpy(output, text.c_str(), len);
}

string Geometry::ToString(string_t geometry, DataFormatType ftype) {
	auto text = Geometry::GetString(geometry, ftype);
	auto str_len = text.size();
	auto buffer = std::unique_ptr<char[]>(new char[str_len]);
	memcpy(buffer.get(), text.c_str(), str_len);
	return string(buffer.get(), str_len);
}

void Geometry::ToGeometry(GSERIALIZED *gser, data_ptr_t output) {
	Postgis postgis;
	data_ptr_t base = (data_ptr_t)postgis.LWGEOM_base(gser);
	auto geometry_len = Geometry::GetGeometrySize(gser);
	memcpy(output, base, geometry_len);
}

string Geometry::ToGeometry(GSERIALIZED *gser) {
	auto geometry_len = Geometry::GetGeometrySize(gser);
	auto buffer = std::unique_ptr<char[]>(new char[geometry_len]);
	Geometry::ToGeometry(gser, (data_ptr_t)buffer.get());
	return string(buffer.get(), geometry_len);
}

string Geometry::ToGeometry(string_t text) {
	auto gser = Geometry::ToGserialized(text);
	auto str = Geometry::ToGeometry(gser);
	Geometry::DestroyGeometry(gser);
	return str;
}

GSERIALIZED *Geometry::GetGserialized(string_t geom) {
	Postgis postgis;
	auto data = (const_data_ptr_t)geom.GetDataUnsafe();
	auto size = geom.GetSize();
	return postgis.LWGEOM_getGserialized(data, size);
}

GSERIALIZED *Geometry::ToGserialized(string_t str) {
	Postgis postgis;
	auto ger = postgis.LWGEOM_in(&str.GetString()[0]);
	return ger;
}

idx_t Geometry::GetGeometrySize(GSERIALIZED *gser) {
	Postgis postgis;
	auto gsize = postgis.LWGEOM_size(gser);
	return gsize;
}

void Geometry::DestroyGeometry(GSERIALIZED *gser) {
	Postgis postgis;
	postgis.LWGEOM_free(gser);
}

data_ptr_t Geometry::GetBase(GSERIALIZED *gser) {
	Postgis postgis;
	data_ptr_t base = (data_ptr_t)postgis.LWGEOM_base(gser);
	return base;
}

GSERIALIZED *Geometry::MakePoint(double x, double y) {
	Postgis postgis;
	return postgis.LWGEOM_makepoint(x, y);
}

GSERIALIZED *Geometry::MakePoint(double x, double y, double z) {
	Postgis postgis;
	return postgis.LWGEOM_makepoint(x, y, z);
}

GSERIALIZED *Geometry::MakeLine(GSERIALIZED *g1, GSERIALIZED *g2) {
	Postgis postgis;
	return postgis.LWGEOM_makeline(g1, g2);
}

GSERIALIZED *Geometry::MakeLineGArray(GSERIALIZED *gserArray[], int nelems) {
	Postgis postgis;
	return postgis.LWGEOM_makeline_garray(gserArray, nelems);
}

GSERIALIZED *Geometry::MakePolygon(GSERIALIZED *geom, GSERIALIZED *gserArray[], int nelems) {
	Postgis postgis;
	return postgis.LWGEOM_makepoly(geom, gserArray, nelems);
}

lwvarlena_t *Geometry::AsBinary(GSERIALIZED *geom, string text) {
	Postgis postgis;
	return postgis.LWGEOM_asBinary(geom, text);
}

std::string Geometry::AsText(GSERIALIZED *geom, int max_digits) {
	Postgis postgis;
	return postgis.LWGEOM_asText(geom, max_digits);
}

lwvarlena_t *Geometry::AsGeoJson(GSERIALIZED *geom, size_t m_dec_digits) {
	Postgis postgis;
	return postgis.LWGEOM_asGeoJson(geom, m_dec_digits);
}

lwvarlena_t *Geometry::GeoHash(GSERIALIZED *geom, size_t m_chars) {
	Postgis postgis;
	return postgis.ST_GeoHash(geom, m_chars);
}

GSERIALIZED *Geometry::GeomFromGeoJson(string_t json) {
	Postgis postgis;
	auto ger = postgis.geom_from_geojson(&json.GetString()[0]);
	return ger;
}

GSERIALIZED *Geometry::FromText(char *text) {
	Postgis postgis;
	return postgis.LWGEOM_from_text(text);
}

GSERIALIZED *Geometry::FromText(char *text, int srid) {
	Postgis postgis;
	return postgis.LWGEOM_from_text(text, srid);
}

GSERIALIZED *Geometry::FromWKB(const char *text, size_t byte_size) {
	Postgis postgis;
	return postgis.LWGEOM_from_WKB(text, byte_size);
}

GSERIALIZED *Geometry::FromWKB(const char *text, size_t byte_size, int srid) {
	Postgis postgis;
	return postgis.LWGEOM_from_WKB(text, byte_size, srid);
}

GSERIALIZED *Geometry::FromGeoHash(string_t hash, int precision) {
	Postgis postgis;
	return postgis.LWGEOM_from_GeoHash(&hash.GetString()[0], precision);
}

GSERIALIZED *Geometry::LWGEOM_boundary(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_boundary(geom);
}

GSERIALIZED *Geometry::Difference(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.ST_Difference(geom1, geom2);
}

GSERIALIZED *Geometry::ClosestPoint(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.LWGEOM_closestpoint(geom1, geom2);
}

GSERIALIZED *Geometry::GeometryUnion(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.ST_Union(geom1, geom2);
}

GSERIALIZED *Geometry::GeometryUnionGArray(GSERIALIZED *gserArray[], int nelems) {
	Postgis postgis;
	return postgis.pgis_union_geometry_array(gserArray, nelems);
}

GSERIALIZED *Geometry::GeometryIntersection(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.ST_Intersection(geom1, geom2);
}

GSERIALIZED *Geometry::GeometrySimplify(GSERIALIZED *geom, double dist) {
	Postgis postgis;
	return postgis.LWGEOM_simplify2d(geom, dist);
}

GSERIALIZED *Geometry::GeometrySnapToGrid(GSERIALIZED *geom, double size) {
	Postgis postgis;
	return postgis.LWGEOM_snaptogrid(geom, size);
}

GSERIALIZED *Geometry::GeometryBuffer(GSERIALIZED *geom, double radius) {
	Postgis postgis;
	return postgis.buffer(geom, radius);
}

GSERIALIZED *Geometry::GeometryBufferText(GSERIALIZED *geom, double radius, string styles_text) {
	Postgis postgis;
	return postgis.buffer(geom, radius, styles_text);
}

bool Geometry::GeometryEquals(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.ST_Equals(geom1, geom2);
}

bool Geometry::GeometryContains(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.contains(geom1, geom2);
}

bool Geometry::GeometryTouches(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.touches(geom1, geom2);
}

bool Geometry::GeometryWithin(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.within(geom1, geom2);
}

bool Geometry::GeometryIntersects(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.ST_Intersects(geom1, geom2);
}

bool Geometry::GeometryCovers(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.covers(geom1, geom2);
}

bool Geometry::GeometryCoveredby(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.coveredby(geom1, geom2);
}

bool Geometry::GeometryDisjoint(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.disjoint(geom1, geom2);
}

bool Geometry::GeometryDWithin(GSERIALIZED *geom1, GSERIALIZED *geom2, double distance) {
	Postgis postgis;
	return postgis.LWGEOM_dwithin(geom1, geom2, distance);
}

double Geometry::GeometryArea(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.ST_Area(geom);
}

double Geometry::GeometryArea(GSERIALIZED *geom, bool use_spheroid) {
	Postgis postgis;
	return postgis.geography_area(geom, use_spheroid);
}

double Geometry::GeometryAngle(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	return postgis.LWGEOM_angle(geom1, geom2);
}

double Geometry::GeometryAngle(std::vector<GSERIALIZED *> geom_vec) {
	Postgis postgis;
	return postgis.LWGEOM_angle(geom_vec);
}

double Geometry::GeometryPerimeter(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_perimeter2d_poly(geom);
}

double Geometry::GeometryPerimeter(GSERIALIZED *geom, bool use_spheroid) {
	Postgis postgis;
	return postgis.geography_perimeter(geom, use_spheroid);
}

double Geometry::GeometryAzimuth(GSERIALIZED *geom1, GSERIALIZED *geom2) {
	Postgis postgis;
	// For geometry
	// return postgis.LWGEOM_azimuth(geom1, geom2);
	// For geography
	return postgis.geography_azimuth(geom1, geom2);
}

double Geometry::GeometryLength(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_length2d_linestring(geom);
}

double Geometry::GeometryLength(GSERIALIZED *geom, bool use_spheroid) {
	Postgis postgis;
	return postgis.geography_length(geom, use_spheroid);
}

GSERIALIZED *Geometry::GeometryBoundingBox(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_envelope(geom);
}

double Geometry::MaxDistance(GSERIALIZED *g1, GSERIALIZED *g2, bool use_spheroid) {
	Postgis postgis;
	// For geometry
	// return postgis.LWGEOM_maxdistance2d_linestring(g1, g2);
	// For Geography
	return postgis.geography_maxdistance(g1, g2, use_spheroid);
}

GSERIALIZED *Geometry::GeometryExtent(GSERIALIZED *gserArray[], int nelems) {
	Postgis postgis;
	return postgis.LWGEOM_envelope_garray(gserArray, nelems);
}

std::vector<int> Geometry::GeometryClusterDBScan(GSERIALIZED *gserArray[], int nelems, double tolerance,
                                                 int minpoints) {
	Postgis postgis;
	return postgis.ST_ClusterDBSCAN(gserArray, nelems, tolerance, minpoints);
}

int Geometry::LWGEOM_dimension(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_dimension(geom);
}

std::vector<GSERIALIZED *> Geometry::LWGEOM_dump(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_dump(geom);
}

GSERIALIZED *Geometry::LWGEOM_endpoint_linestring(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_endpoint_linestring(geom);
}

std::string Geometry::Geometrytype(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.geometry_geometrytype(geom);
}

bool Geometry::IsClosed(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_isclosed(geom);
}

bool Geometry::IsCollection(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.ST_IsCollection(geom);
}

bool Geometry::IsEmpty(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_isempty(geom);
}

bool Geometry::IsRing(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_isring(geom);
}

int Geometry::NPoints(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_npoints(geom);
}

int Geometry::NumGeometries(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_numgeometries_collection(geom);
}

int Geometry::NumPoints(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_numpoints_linestring(geom);
}

GSERIALIZED *Geometry::PointN(GSERIALIZED *geom, int index) {
	Postgis postgis;
	return postgis.LWGEOM_pointn_linestring(geom, index);
}

GSERIALIZED *Geometry::StartPoint(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_startpoint_linestring(geom);
}

double Geometry::Distance(GSERIALIZED *g1, GSERIALIZED *g2) {
	Postgis postgis;
	return postgis.ST_distance(g1, g2);
}

double Geometry::Distance(GSERIALIZED *g1, GSERIALIZED *g2, bool use_spheroid) {
	Postgis postgis;
	return postgis.geography_distance(g1, g2, use_spheroid);
}

double Geometry::XPoint(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_x_point(geom);
}

double Geometry::YPoint(GSERIALIZED *geom) {
	Postgis postgis;
	return postgis.LWGEOM_y_point(geom);
}

GSERIALIZED *Geometry::Centroid(GSERIALIZED *g) {
	Postgis postgis;
	return postgis.centroid(g);
}

GSERIALIZED *Geometry::Centroid(GSERIALIZED *g, bool use_spheroid) {
	Postgis postgis;
	return postgis.geography_centroid(g, use_spheroid);
}

GSERIALIZED *Geometry::Convexhull(GSERIALIZED *g) {
	Postgis postgis;
	return postgis.convexhull(g);
}

} // namespace duckdb
