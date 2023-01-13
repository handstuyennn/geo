//===----------------------------------------------------------------------===//
//                         DuckDB
//
// transformation-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static unique_ptr<FunctionData> GeometryUnionArrayBind(ClientContext &context, ScalarFunction &bound_function,
                                                       vector<unique_ptr<Expression>> &arguments) {
	if (arguments[0]->HasParameter()) {
		throw ParameterNotResolvedException();
	}
	bound_function.arguments[0] = arguments[0]->return_type;
	return nullptr;
}

static const std::vector<ScalarFunctionSet> GetTransformationScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

	// ST_BOUNDARY
	ScalarFunctionSet boundary("st_boundary");
	boundary.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryBoundaryFunction));
	func_set.push_back(boundary);

	// ST_BUFFER
	ScalarFunctionSet buffer("st_buffer");
	buffer.AddFunction(ScalarFunction({geo_type, LogicalType::DOUBLE}, geo_type, GeoFunctions::GeometryBufferFunction));
	buffer.AddFunction(ScalarFunction({geo_type, LogicalType::DOUBLE, LogicalType::VARCHAR}, geo_type,
	                                  GeoFunctions::GeometryBufferTextFunction));
	func_set.push_back(buffer);

	// ST_CENTROID
	ScalarFunctionSet centroid("st_centroid");
	centroid.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryCentroidFunction));
	centroid.AddFunction(
	    ScalarFunction({geo_type, LogicalType::BOOLEAN}, geo_type, GeoFunctions::GeometryCentroidFunction));
	func_set.push_back(centroid);

	// ST_CLOSESTPOINT
	ScalarFunctionSet closestpoint("st_closestpoint");
	closestpoint.AddFunction(
	    ScalarFunction({geo_type, geo_type}, geo_type, GeoFunctions::GeometryClosestPointFunction));
	func_set.push_back(closestpoint);

	// ST_CONVEXHULL
	ScalarFunctionSet convexhull("st_convexhull");
	convexhull.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryConvexhullFunction));
	func_set.push_back(convexhull);

	// ST_DIFFERENCE
	ScalarFunctionSet difference("st_difference");
	difference.AddFunction(ScalarFunction({geo_type, geo_type}, geo_type, GeoFunctions::GeometryDifferenceFunction));
	func_set.push_back(difference);

	// ST_INTERSECTION
	ScalarFunctionSet intersection("st_intersection");
	intersection.AddFunction(
	    ScalarFunction({geo_type, geo_type}, geo_type, GeoFunctions::GeometryIntersectionFunction));
	func_set.push_back(intersection);

	// ST_SIMPLIFY
	ScalarFunctionSet simplify("st_simplify");
	simplify.AddFunction(
	    ScalarFunction({geo_type, LogicalType::DOUBLE}, geo_type, GeoFunctions::GeometrySimplifyFunction));
	func_set.push_back(simplify);

	// ST_SNAPTOGRID
	ScalarFunctionSet snaptogrid("st_snaptogrid");
	snaptogrid.AddFunction(
	    ScalarFunction({geo_type, LogicalType::DOUBLE}, geo_type, GeoFunctions::GeometrySnapToGridFunction));
	func_set.push_back(snaptogrid);

	// ST_UNION
	ScalarFunctionSet geom_union("st_union");
	geom_union.AddFunction(ScalarFunction({geo_type, geo_type}, geo_type, GeoFunctions::GeometryUnionFunction));
	geom_union.AddFunction(ScalarFunction({LogicalType::LIST(geo_type)}, geo_type,
	                                      GeoFunctions::GeometryUnionArrayFunction, GeometryUnionArrayBind));
	func_set.push_back(geom_union);

	return func_set;
}

} // namespace duckdb
