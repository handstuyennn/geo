//===----------------------------------------------------------------------===//
//                         DuckDB
//
// parser-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static const std::vector<ScalarFunctionSet> GetParserScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

	// ST_GEOGFROM
	ScalarFunctionSet geog_from("st_geogfrom");
	geog_from.AddFunction(ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryGeogFromFunction));
	func_set.push_back(geog_from);

	// ST_GEOMFROMGEOJSON
	ScalarFunctionSet geomfromjson_from("st_geomfromgeojson");
	geomfromjson_from.AddFunction(
	    ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryGeomFromGeoJsonFunction));
	func_set.push_back(geomfromjson_from);

	// ST_GEOMFROMTEXT
	ScalarFunctionSet from_text("st_geomfromtext");
	from_text.AddFunction(ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryFromTextFunction));
	from_text.AddFunction(
	    ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryFromTextFunction));
	func_set.push_back(from_text);

	// ST_GEOMFROMWKB
	ScalarFunctionSet from_wkb("st_geomfromwkb");
	from_wkb.AddFunction(ScalarFunction({LogicalType::BLOB}, geo_type, GeoFunctions::GeometryFromWKBFunction));
	from_wkb.AddFunction(
	    ScalarFunction({LogicalType::BLOB, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryFromWKBFunction));
	func_set.push_back(from_wkb);

	// ST_GEOMFROMGEOHASH
	ScalarFunctionSet from_geohash("st_geomfromgeohash");
	from_geohash.AddFunction(
	    ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryFromGeoHashFunction));
	from_geohash.AddFunction(ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type,
	                                        GeoFunctions::GeometryFromGeoHashFunction));
	func_set.push_back(from_geohash);

	return func_set;
}

} // namespace duckdb
