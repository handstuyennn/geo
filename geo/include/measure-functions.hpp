//===----------------------------------------------------------------------===//
//                         DuckDB
//
// measure-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#include "geo-functions.hpp"

#pragma once

namespace duckdb {

static const std::vector<ScalarFunctionSet> GetMeasureScalarFunctions(LogicalType geo_type) {
	std::vector<ScalarFunctionSet> func_set {};

    // ST_ANGLE
	ScalarFunctionSet angle("st_angle");
	angle.AddFunction(
	    ScalarFunction({geo_type, geo_type, geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryAngleFunction));
    func_set.push_back(angle);

    // ST_AREA
	ScalarFunctionSet area("st_area");
	area.AddFunction(ScalarFunction({geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryAreaFunction));
	area.AddFunction(
	    ScalarFunction({geo_type, LogicalType::BOOLEAN}, LogicalType::DOUBLE, GeoFunctions::GeometryAreaFunction));
    func_set.push_back(area);

    // ST_AZIMUTH
	ScalarFunctionSet azimuth("st_azimuth");
	azimuth.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryAzimuthFunction));
    func_set.push_back(azimuth);

    // ST_BOUNDINGBOX (ALIAS: ST_ENVELOPE)
	ScalarFunctionSet boundingbox("st_boundingbox");
	ScalarFunctionSet envelope("st_envelope");
	auto boundingboxUnaryFunc = ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryBoundingBoxFunction);

	boundingbox.AddFunction(boundingboxUnaryFunc);
	envelope.AddFunction(boundingboxUnaryFunc);
    func_set.push_back(boundingbox);
    func_set.push_back(envelope);

    // ST_EXTENT
	ScalarFunctionSet extent("st_extent");
	extent.AddFunction(ScalarFunction({LogicalType::LIST(geo_type)}, geo_type, GeoFunctions::GeometryExtentFunction));
    func_set.push_back(extent);

    // ST_DISTANCE
	ScalarFunctionSet distance("st_distance");
	distance.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryDistanceFunction));
	distance.AddFunction(ScalarFunction({geo_type, geo_type, LogicalType::BOOLEAN}, LogicalType::DOUBLE,
	                                    GeoFunctions::GeometryDistanceFunction));
    func_set.push_back(distance);

    // ST_LENGTH
	ScalarFunctionSet length("st_length");
	length.AddFunction(ScalarFunction({geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryLengthFunction));
	length.AddFunction(
	    ScalarFunction({geo_type, LogicalType::BOOLEAN}, LogicalType::DOUBLE, GeoFunctions::GeometryLengthFunction));
    func_set.push_back(length);

    // ST_MAXDISTANCE
	ScalarFunctionSet maxdistance("st_maxdistance");
	maxdistance.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryMaxDistanceFunction));
    func_set.push_back(maxdistance);

    // ST_PERIMETER
	ScalarFunctionSet perimeter("st_perimeter");
	perimeter.AddFunction(ScalarFunction({geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryPerimeterFunction));
	perimeter.AddFunction(
	    ScalarFunction({geo_type, LogicalType::BOOLEAN}, LogicalType::DOUBLE, GeoFunctions::GeometryPerimeterFunction));
    func_set.push_back(perimeter);

    return func_set;
}

} // namespace duckdb
