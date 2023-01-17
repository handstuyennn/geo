//===----------------------------------------------------------------------===//
//                         DuckDB
//
// predicate-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static const std::vector<ScalarFunctionSet> GetPredicateScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

	// ST_CONTAINS
	ScalarFunctionSet contains("st_contains");
	contains.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryContainsFunction));
	func_set.push_back(contains);

	// ST_COVEREDBY
	ScalarFunctionSet coveredby("st_coveredby");
	coveredby.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryCoveredByFunction));
	func_set.push_back(coveredby);

	// ST_COVERS
	ScalarFunctionSet covers("st_covers");
	covers.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryCoversFunction));
	func_set.push_back(covers);

	// ST_DISJOINT
	ScalarFunctionSet disjoint("st_disjoint");
	disjoint.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryDisjointFunction));
	func_set.push_back(disjoint);

	// ST_DWITHIN
	ScalarFunctionSet dwithin("st_dwithin");
	dwithin.AddFunction(ScalarFunction({geo_type, geo_type, LogicalType::DOUBLE}, LogicalType::BOOLEAN,
	                                   GeoFunctions::GeometryDWithinFunction));
	func_set.push_back(dwithin);

	// ST_EQUALS
	ScalarFunctionSet equals("st_equals");
	equals.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryEqualsFunction));
	func_set.push_back(equals);

	// ST_INTERSECTS
	ScalarFunctionSet intersects("st_intersects");
	intersects.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIntersectsFunction));
	func_set.push_back(intersects);

	// ST_TOUCHES
	ScalarFunctionSet touches("st_touches");
	touches.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryTouchesFunction));
	func_set.push_back(touches);

	// ST_WITHIN
	ScalarFunctionSet within("st_within");
	within.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryWithinFunction));
	func_set.push_back(within);

	return func_set;
}

} // namespace duckdb
