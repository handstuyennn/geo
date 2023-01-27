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
	ScalarFunctionSet geomfromjson("st_geomfromgeojson");
	ScalarFunctionSet geogfromjson("st_geogfromgeojson");

	auto fromjsonfunc = ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryGeomFromGeoJsonFunction);
	geomfromjson.AddFunction(fromjsonfunc);
	geogfromjson.AddFunction(fromjsonfunc);
	func_set.push_back(geomfromjson);
	func_set.push_back(geogfromjson);

	// ST_GEOMFROMTEXT
	ScalarFunctionSet geomfromtext("st_geomfromtext");
	ScalarFunctionSet geogfromtext("st_geogfromtext");

	auto fromtextunary = ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryFromTextFunction);
	auto fromtextbinary =
	    ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryFromTextFunction);
	geomfromtext.AddFunction(fromtextunary);
	geomfromtext.AddFunction(fromtextbinary);
	geogfromtext.AddFunction(fromtextunary);
	geogfromtext.AddFunction(fromtextbinary);
	func_set.push_back(geomfromtext);
	func_set.push_back(geogfromtext);

	// ST_GEOMFROMWKB
	ScalarFunctionSet geomfromwkb("st_geomfromwkb");
	ScalarFunctionSet geogfromwkb("st_geogfromwkb");
	auto fromwkbunary = ScalarFunction({LogicalType::BLOB}, geo_type, GeoFunctions::GeometryFromWKBFunction);
	auto fromwkbbinary =
	    ScalarFunction({LogicalType::BLOB, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryFromWKBFunction);
	geomfromwkb.AddFunction(fromwkbunary);
	geomfromwkb.AddFunction(fromwkbbinary);
	geogfromwkb.AddFunction(fromwkbunary);
	geogfromwkb.AddFunction(fromwkbbinary);
	func_set.push_back(geomfromwkb);
	func_set.push_back(geogfromwkb);

	// ST_GEOMFROMGEOHASH/ST_GEOGPOINTFROMGEOHASH
	ScalarFunctionSet geomfromgeohash("st_geomfromgeohash");
	auto fromgeohashunary = ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryFromGeoHashFunction);
	auto fromgeohashbinary = ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type,
	                                        GeoFunctions::GeometryFromGeoHashFunction);
	geomfromgeohash.AddFunction(fromgeohashunary);
	geomfromgeohash.AddFunction(fromgeohashbinary);

	ScalarFunctionSet geogpointfromgeohash("st_geogpointfromgeohash");
	auto gpointfromgeohashunary =
	    ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryGPointFromGeoHashFunction);
	auto gpointfromgeohashbinary = ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type,
	                                              GeoFunctions::GeometryGPointFromGeoHashFunction);
	geogpointfromgeohash.AddFunction(gpointfromgeohashunary);
	geogpointfromgeohash.AddFunction(gpointfromgeohashbinary);
	func_set.push_back(geomfromgeohash);
	func_set.push_back(geogpointfromgeohash);

	return func_set;
}

} // namespace duckdb
