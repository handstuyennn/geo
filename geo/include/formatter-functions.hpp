//===----------------------------------------------------------------------===//
//                         DuckDB
//
// formatter-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static const std::vector<ScalarFunctionSet> GetFormatterScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

	// ST_ASBINARY
	ScalarFunctionSet as_binary("st_asbinary");
	as_binary.AddFunction(ScalarFunction({geo_type}, LogicalType::BLOB, GeoFunctions::GeometryAsBinaryFunction));
	as_binary.AddFunction(
	    ScalarFunction({geo_type, LogicalType::VARCHAR}, LogicalType::BLOB, GeoFunctions::GeometryAsBinaryFunction));
	func_set.push_back(as_binary);

	// ST_ASTEXT
	ScalarFunctionSet as_text("st_astext");
	as_text.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryAsTextFunction));
	as_text.AddFunction(
	    ScalarFunction({geo_type, LogicalType::INTEGER}, LogicalType::VARCHAR, GeoFunctions::GeometryAsTextFunction));
	func_set.push_back(as_text);

	// ST_ASGEOJSON
	ScalarFunctionSet as_geojson("st_asgeojson");
	as_geojson.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryAsGeojsonFunction));
	as_geojson.AddFunction(ScalarFunction({geo_type, LogicalType::INTEGER}, LogicalType::VARCHAR,
	                                      GeoFunctions::GeometryAsGeojsonFunction));
	func_set.push_back(as_geojson);

	// ST_GEOHASH
	ScalarFunctionSet geohash("st_geohash");
	geohash.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryGeoHashFunction));
	geohash.AddFunction(
	    ScalarFunction({geo_type, LogicalType::INTEGER}, LogicalType::VARCHAR, GeoFunctions::GeometryGeoHashFunction));
	func_set.push_back(geohash);

	return func_set;
}

} // namespace duckdb
