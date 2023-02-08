//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/types/geometry.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/types.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

enum class DataFormatType : uint8_t { FORMAT_VALUE_TYPE_WKB, FORMAT_VALUE_TYPE_WKT, FORMAT_VALUE_TYPE_GEOJSON };

//! The Geometry class is a static class that holds helper functions for the Geometry type.
class Geometry {
public:
	static string GetString(string_t geometry, DataFormatType ftype = DataFormatType::FORMAT_VALUE_TYPE_WKB);
	//! Converts a geometry to a string, writing the output to the designated output string.
	static void ToString(string_t geometry, char *output, DataFormatType ftype = DataFormatType::FORMAT_VALUE_TYPE_WKB);
	//! Convert a geometry object to a string
	static string ToString(string_t geometry, DataFormatType ftype = DataFormatType::FORMAT_VALUE_TYPE_WKB);

	static GSERIALIZED *GetGserialized(string_t geom);

	//! Convert a string to a geometry. This function should ONLY be called after calling GetGeometrySize, since it does
	//! NOT perform data validation.
	static void ToGeometry(GSERIALIZED *gser, data_ptr_t output);
	//! Convert a string object to a geometry
	static string ToGeometry(GSERIALIZED *gser);
	static string ToGeometry(string_t text);

	static GSERIALIZED *ToGserialized(string_t str);

	static idx_t GetGeometrySize(GSERIALIZED *gser);

	static void DestroyGeometry(GSERIALIZED *gser);

	static data_ptr_t GetBase(GSERIALIZED *gser);

	static GSERIALIZED *MakePoint(double x, double y);
	static GSERIALIZED *MakePoint(double x, double y, double z);
	static GSERIALIZED *MakeLine(GSERIALIZED *g1, GSERIALIZED *g2);
	static GSERIALIZED *MakeLineGArray(GSERIALIZED *gserArray[], int nelems);
	static GSERIALIZED *MakePolygon(GSERIALIZED *geom, GSERIALIZED *gserArray[] = {}, int nelems = 0);

	static lwvarlena_t *AsBinary(GSERIALIZED *gser, string text = "");
	static std::string AsText(GSERIALIZED *gser, int max_digits = OUT_DEFAULT_DECIMAL_DIGITS);
	static lwvarlena_t *AsGeoJson(GSERIALIZED *gser, size_t m_dec_digits = OUT_DEFAULT_DECIMAL_DIGITS);
	static lwvarlena_t *GeoHash(GSERIALIZED *gser, size_t m_chars = 0);

	static GSERIALIZED *GeomFromGeoJson(string_t json);
	static GSERIALIZED *FromText(char *text);
	static GSERIALIZED *FromText(char *text, int srid);
	static GSERIALIZED *FromWKB(const char *text, size_t byte_size);
	static GSERIALIZED *FromWKB(const char *text, size_t byte_size, int srid);
	static GSERIALIZED *FromGeoHash(string_t hash, int precision = -1);

	static GSERIALIZED *LWGEOM_boundary(GSERIALIZED *geom);
	static GSERIALIZED *Difference(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static GSERIALIZED *ClosestPoint(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static GSERIALIZED *GeometryUnion(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static GSERIALIZED *GeometryUnionGArray(GSERIALIZED *gserArray[], int nelems);
	static GSERIALIZED *GeometryIntersection(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static GSERIALIZED *GeometrySimplify(GSERIALIZED *geom, double dist);
	static GSERIALIZED *Centroid(GSERIALIZED *g);
	static GSERIALIZED *Centroid(GSERIALIZED *g, bool use_spheroid);
	static GSERIALIZED *Convexhull(GSERIALIZED *g);
	static GSERIALIZED *GeometrySnapToGrid(GSERIALIZED *geom, double size);
	static GSERIALIZED *GeometryBuffer(GSERIALIZED *geom, double radius);
	static GSERIALIZED *GeometryBufferText(GSERIALIZED *geom, double radius, string styles_text);

	static bool GeometryEquals(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryContains(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryTouches(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryWithin(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryIntersects(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryCovers(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryCoveredby(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryDisjoint(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static bool GeometryDWithin(GSERIALIZED *geom1, GSERIALIZED *geom2, double distance);

	static double GeometryArea(GSERIALIZED *geom);
	static double GeometryArea(GSERIALIZED *geom, bool use_spheroid);
	static double GeometryAngle(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static double GeometryAngle(std::vector<GSERIALIZED *> geom_vec);
	static double GeometryPerimeter(GSERIALIZED *geom);
	static double GeometryPerimeter(GSERIALIZED *geom, bool use_spheroid);
	static double GeometryAzimuth(GSERIALIZED *geom1, GSERIALIZED *geom2);
	static double GeometryLength(GSERIALIZED *geom);
	static double GeometryLength(GSERIALIZED *geom, bool use_spheroid);
	static GSERIALIZED *GeometryBoundingBox(GSERIALIZED *geom);
	static double Distance(GSERIALIZED *g1, GSERIALIZED *g2);
	static double Distance(GSERIALIZED *g1, GSERIALIZED *g2, bool use_spheroid);
	static double MaxDistance(GSERIALIZED *g1, GSERIALIZED *g2, bool use_spheroid = true);
	static GSERIALIZED *GeometryExtent(GSERIALIZED *gserArray[], int nelems);

	static std::vector<int> GeometryClusterDBScan(GSERIALIZED *gserArray[], int nelems, double tolerance,
	                                              int minpoints);

	static int LWGEOM_dimension(GSERIALIZED *geom);
	static std::vector<GSERIALIZED *> LWGEOM_dump(GSERIALIZED *geom);
	static GSERIALIZED *LWGEOM_endpoint_linestring(GSERIALIZED *geom);
	static std::string Geometrytype(GSERIALIZED *geom);
	static bool IsClosed(GSERIALIZED *geom);
	static bool IsCollection(GSERIALIZED *geom);
	static bool IsEmpty(GSERIALIZED *geom);
	static bool IsRing(GSERIALIZED *geom);
	static int NPoints(GSERIALIZED *geom);
	static int NumGeometries(GSERIALIZED *geom);
	static int NumPoints(GSERIALIZED *geom);
	static GSERIALIZED *PointN(GSERIALIZED *geom, int index);
	static GSERIALIZED *StartPoint(GSERIALIZED *geom);
	static double XPoint(GSERIALIZED *geom);
	static double YPoint(GSERIALIZED *geom);
};
} // namespace duckdb
