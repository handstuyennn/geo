//===----------------------------------------------------------------------===//
//                         DuckDB
//
// constructor-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static unique_ptr<FunctionData> MakeLineArrayBind(ClientContext &context, ScalarFunction &bound_function,
                                                  vector<unique_ptr<Expression>> &arguments) {
	if (arguments[0]->HasParameter()) {
		throw ParameterNotResolvedException();
	}
	bound_function.arguments[0] = arguments[0]->return_type;
	return nullptr;
}

static unique_ptr<FunctionData> MakePolygonArrayBind(ClientContext &context, ScalarFunction &bound_function,
                                                     vector<unique_ptr<Expression>> &arguments) {
	if (arguments[0]->HasParameter()) {
		throw ParameterNotResolvedException();
	}
	bound_function.arguments[1] = arguments[1]->return_type;
	return nullptr;
}

static const std::vector<ScalarFunctionSet> GetConstructorScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

	// ST_MAKEPOINT / ST_GEOGPOINT
	ScalarFunctionSet make_point("st_makepoint");
	make_point.AddFunction(
	    ScalarFunction({LogicalType::DOUBLE, LogicalType::DOUBLE}, geo_type, GeoFunctions::MakePointFunction));
	make_point.AddFunction(ScalarFunction({LogicalType::DOUBLE, LogicalType::DOUBLE, LogicalType::DOUBLE}, geo_type,
	                                      GeoFunctions::MakePointFunction));
	func_set.push_back(make_point);

	// ST_MAKELINE
	ScalarFunctionSet make_line("st_makeline");
	make_line.AddFunction(ScalarFunction({geo_type, geo_type}, geo_type, GeoFunctions::MakeLineFunction));
	make_line.AddFunction(ScalarFunction({LogicalType::LIST(geo_type)}, geo_type, GeoFunctions::MakeLineArrayFunction,
	                                     MakeLineArrayBind));
	func_set.push_back(make_line);

	// ST_MAKEPOLYGON
	ScalarFunctionSet make_polygon("st_makepolygon");
	make_polygon.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::MakePolygonFunction));
	make_polygon.AddFunction(ScalarFunction({geo_type, LogicalType::LIST(geo_type)}, geo_type,
	                                        GeoFunctions::MakePolygonFunction, MakePolygonArrayBind));
	func_set.push_back(make_polygon);

	return func_set;
}

} // namespace duckdb
