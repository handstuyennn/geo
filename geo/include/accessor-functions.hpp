//===----------------------------------------------------------------------===//
//                         DuckDB
//
// accessor-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static const std::vector<ScalarFunctionSet> GetAccessorScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

	// ST_DIMENSION
	ScalarFunctionSet dimension("st_dimension");
	dimension.AddFunction(ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryDimensionFunction));
	func_set.push_back(dimension);

	// ST_DUMP
	ScalarFunctionSet dump("st_dump");
	dump.AddFunction(ScalarFunction({geo_type}, LogicalType::LIST(geo_type), GeoFunctions::GeometryDumpFunction));
	func_set.push_back(dump);

	// ST_ENDPOINT
	ScalarFunctionSet endpoint("st_endpoint");
	endpoint.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryEndPointFunction));
	func_set.push_back(endpoint);

	// ST_GEOMETRYTYPE
	ScalarFunctionSet geometrytype("st_geometrytype");
	geometrytype.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryTypeFunction));
	func_set.push_back(geometrytype);

	// ST_ISCLOSED
	ScalarFunctionSet isclosed("st_isclosed");
	isclosed.AddFunction(ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsClosedFunction));
	func_set.push_back(isclosed);

	// ST_ISCOLLECTION
	ScalarFunctionSet iscollection("st_iscollection");
	iscollection.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsCollectionFunction));
	func_set.push_back(iscollection);

	// ST_ISEMPTY
	ScalarFunctionSet isempty("st_isempty");
	isempty.AddFunction(ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsEmptyFunction));
	func_set.push_back(isempty);

	// ST_ISRING
	ScalarFunctionSet isring("st_isring");
	isring.AddFunction(ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsRingFunction));
	func_set.push_back(isring);

	// ST_NPOINTS
	ScalarFunctionSet npoints("st_npoints");
	npoints.AddFunction(ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryNPointsFunction));
	func_set.push_back(npoints);

	// ST_NUMGEOMETRIES
	ScalarFunctionSet numgeometries("st_numgeometries");
	numgeometries.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryNumGeometriesFunction));
	func_set.push_back(numgeometries);

	// ST_NUMPOINTS
	ScalarFunctionSet numpoints("st_numpoints");
	numpoints.AddFunction(ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryNumPointsFunction));
	func_set.push_back(numpoints);

	// ST_POINTN
	ScalarFunctionSet pointn("st_pointn");
	pointn.AddFunction(
	    ScalarFunction({geo_type, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryPointNFunction));
	func_set.push_back(pointn);

	// ST_STARTPOINT
	ScalarFunctionSet startpoint("st_startpoint");
	startpoint.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryStartPointFunction));
	func_set.push_back(startpoint);

	// ST_X
	ScalarFunctionSet get_x("st_x");
	get_x.AddFunction(ScalarFunction({geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryGetXFunction));
	func_set.push_back(get_x);

	// ST_Y
	ScalarFunctionSet get_y("st_y");
	get_y.AddFunction(ScalarFunction({geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryGetYFunction));
	func_set.push_back(get_y);

	return func_set;
}

} // namespace duckdb
