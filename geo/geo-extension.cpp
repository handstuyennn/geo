#define DUCKDB_EXTENSION_MAIN

#include "geo-extension.hpp"

#include "duckdb.hpp"
#include "duckdb/catalog/catalog.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/parser/parsed_data/create_type_info.hpp"
#include "geo-functions.hpp"

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

void GeoExtension::Load(DuckDB &db) {
	Connection con(db);
	con.BeginTransaction();

	auto &catalog = Catalog::GetCatalog(*con.context);

	auto geo_type = LogicalType(LogicalTypeId::BLOB);
	geo_type.SetAlias("GEOMETRY");

	CreateTypeInfo info("Geometry", geo_type);
	info.temporary = true;
	info.internal = true;
	catalog.CreateType(*con.context, &info);

	// add geo casts
	auto &config = DBConfig::GetConfig(*con.context);

	auto &casts = config.GetCastFunctions();
	casts.RegisterCastFunction(LogicalType::VARCHAR, geo_type, GeoFunctions::CastVarcharToGEO, 100);
	casts.RegisterCastFunction(geo_type, LogicalType::VARCHAR, GeoFunctions::CastGeoToVarchar);

	// add geo functions
	// ST_MAKEPOINT / ST_GEOGPOINT
	ScalarFunctionSet make_point("st_makepoint");
	make_point.AddFunction(
	    ScalarFunction({LogicalType::DOUBLE, LogicalType::DOUBLE}, geo_type, GeoFunctions::MakePointFunction));
	make_point.AddFunction(ScalarFunction({LogicalType::DOUBLE, LogicalType::DOUBLE, LogicalType::DOUBLE}, geo_type,
	                                      GeoFunctions::MakePointFunction));

	CreateScalarFunctionInfo make_point_func_info(make_point);
	catalog.AddFunction(*con.context, &make_point_func_info);

	// ST_MAKELINE
	ScalarFunctionSet make_line("st_makeline");
	make_line.AddFunction(ScalarFunction({geo_type, geo_type}, geo_type, GeoFunctions::MakeLineFunction));
	make_line.AddFunction(ScalarFunction({LogicalType::LIST(geo_type)}, geo_type, GeoFunctions::MakeLineArrayFunction,
	                                     MakeLineArrayBind));

	CreateScalarFunctionInfo make_line_func_info(make_line);
	catalog.AddFunction(*con.context, &make_line_func_info);

	// ST_MAKEPOLYGON
	ScalarFunctionSet make_polygon("st_makepolygon");
	make_polygon.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::MakePolygonFunction));
	make_polygon.AddFunction(ScalarFunction({geo_type, LogicalType::LIST(geo_type)}, geo_type,
	                                        GeoFunctions::MakePolygonFunction, MakePolygonArrayBind));

	CreateScalarFunctionInfo make_polygon_func_info(make_polygon);
	catalog.AddFunction(*con.context, &make_polygon_func_info);

	// ST_ASBINARY
	ScalarFunctionSet as_binary("st_asbinary");
	as_binary.AddFunction(ScalarFunction({geo_type}, LogicalType::BLOB, GeoFunctions::GeometryAsBinaryFunction));
	as_binary.AddFunction(
	    ScalarFunction({geo_type, LogicalType::VARCHAR}, LogicalType::BLOB, GeoFunctions::GeometryAsBinaryFunction));

	CreateScalarFunctionInfo as_binary_func_info(as_binary);
	catalog.AddFunction(*con.context, &as_binary_func_info);

	// ST_ASTEXT
	ScalarFunctionSet as_text("st_astext");
	as_text.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryAsTextFunction));
	as_text.AddFunction(
	    ScalarFunction({geo_type, LogicalType::INTEGER}, LogicalType::VARCHAR, GeoFunctions::GeometryAsTextFunction));

	CreateScalarFunctionInfo as_text_func_info(as_text);
	catalog.AddFunction(*con.context, &as_text_func_info);

	// ST_ASGEOJSON
	ScalarFunctionSet as_geojson("st_asgeojson");
	as_geojson.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryAsGeojsonFunction));
	as_geojson.AddFunction(ScalarFunction({geo_type, LogicalType::INTEGER}, LogicalType::VARCHAR,
	                                      GeoFunctions::GeometryAsGeojsonFunction));

	CreateScalarFunctionInfo as_geojson_func_info(as_geojson);
	catalog.AddFunction(*con.context, &as_geojson_func_info);

	// ST_GEOHASH
	ScalarFunctionSet geohash("st_geohash");
	geohash.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryGeoHashFunction));
	geohash.AddFunction(
	    ScalarFunction({geo_type, LogicalType::INTEGER}, LogicalType::VARCHAR, GeoFunctions::GeometryGeoHashFunction));

	CreateScalarFunctionInfo geohash_func_info(geohash);
	catalog.AddFunction(*con.context, &geohash_func_info);

	// ST_GEOGFROM
	ScalarFunctionSet geog_from("st_geogfrom");
	geog_from.AddFunction(ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryGeogFromFunction));

	CreateScalarFunctionInfo geog_from_func_info(geog_from);
	catalog.AddFunction(*con.context, &geog_from_func_info);

	// ST_GEOMFROMGEOJSON
	ScalarFunctionSet geomfromjson_from("st_geomfromgeojson");
	geomfromjson_from.AddFunction(
	    ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryGeomFromGeoJsonFunction));

	CreateScalarFunctionInfo geomfromjson_from_func_info(geomfromjson_from);
	catalog.AddFunction(*con.context, &geomfromjson_from_func_info);

	// ST_DISTANCE
	ScalarFunctionSet distance("st_distance");
	distance.AddFunction(
	    ScalarFunction({geo_type, geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryDistanceFunction));
	distance.AddFunction(ScalarFunction({geo_type, geo_type, LogicalType::BOOLEAN}, LogicalType::DOUBLE,
	                                    GeoFunctions::GeometryDistanceFunction));

	CreateScalarFunctionInfo distance_func_info(distance);
	catalog.AddFunction(*con.context, &distance_func_info);

	// ST_CENTROID
	ScalarFunctionSet centroid("st_centroid");
	centroid.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryCentroidFunction));
	centroid.AddFunction(
	    ScalarFunction({geo_type, LogicalType::BOOLEAN}, geo_type, GeoFunctions::GeometryCentroidFunction));

	CreateScalarFunctionInfo centroid_func_info(centroid);
	catalog.AddFunction(*con.context, &centroid_func_info);

	// ST_GEOMFROMTEXT
	ScalarFunctionSet from_text("st_geomfromtext");
	from_text.AddFunction(ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryFromTextFunction));
	from_text.AddFunction(
	    ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryFromTextFunction));

	CreateScalarFunctionInfo from_text_func_info(from_text);
	catalog.AddFunction(*con.context, &from_text_func_info);

	// ST_GEOMFROMWKB
	ScalarFunctionSet from_wkb("st_geomfromwkb");
	from_wkb.AddFunction(ScalarFunction({LogicalType::BLOB}, geo_type, GeoFunctions::GeometryFromWKBFunction));
	from_wkb.AddFunction(
	    ScalarFunction({LogicalType::BLOB, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryFromWKBFunction));

	CreateScalarFunctionInfo from_wkb_func_info(from_wkb);
	catalog.AddFunction(*con.context, &from_wkb_func_info);

	// ST_GEOMFROMGEOHASH
	ScalarFunctionSet from_geohash("st_geomfromgeohash");
	from_geohash.AddFunction(
	    ScalarFunction({LogicalType::VARCHAR}, geo_type, GeoFunctions::GeometryFromGeoHashFunction));
	from_geohash.AddFunction(ScalarFunction({LogicalType::VARCHAR, LogicalType::INTEGER}, geo_type,
	                                        GeoFunctions::GeometryFromGeoHashFunction));

	CreateScalarFunctionInfo from_geohash_func_info(from_geohash);
	catalog.AddFunction(*con.context, &from_geohash_func_info);

	// ST_BOUNDARY
	ScalarFunctionSet boundary("st_boundary");
	boundary.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryBoundaryFunction));

	CreateScalarFunctionInfo boundary_func_info(boundary);
	catalog.AddFunction(*con.context, &boundary_func_info);

	// ST_DIMENSION
	ScalarFunctionSet dimension("st_dimension");
	dimension.AddFunction(ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryDimensionFunction));

	CreateScalarFunctionInfo dimension_func_info(dimension);
	catalog.AddFunction(*con.context, &dimension_func_info);

	// ST_DUMP
	ScalarFunctionSet dump("st_dump");
	dump.AddFunction(ScalarFunction({geo_type}, LogicalType::LIST(geo_type), GeoFunctions::GeometryDumpFunction));

	CreateScalarFunctionInfo dump_func_info(dump);
	catalog.AddFunction(*con.context, &dump_func_info);

	// ST_ENDPOINT
	ScalarFunctionSet endpoint("st_endpoint");
	endpoint.AddFunction(ScalarFunction({geo_type}, geo_type, GeoFunctions::GeometryEndPointFunction));

	CreateScalarFunctionInfo endpoint_func_info(endpoint);
	catalog.AddFunction(*con.context, &endpoint_func_info);

	// ST_GEOMETRYTYPE
	ScalarFunctionSet geometrytype("st_geometrytype");
	geometrytype.AddFunction(ScalarFunction({geo_type}, LogicalType::VARCHAR, GeoFunctions::GeometryTypeFunction));

	CreateScalarFunctionInfo geometrytype_func_info(geometrytype);
	catalog.AddFunction(*con.context, &geometrytype_func_info);

	// ST_ISCLOSED
	ScalarFunctionSet isclosed("st_isclosed");
	isclosed.AddFunction(ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsClosedFunction));

	CreateScalarFunctionInfo isclosed_func_info(isclosed);
	catalog.AddFunction(*con.context, &isclosed_func_info);

	// ST_ISCOLLECTION
	ScalarFunctionSet iscollection("st_iscollection");
	iscollection.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsCollectionFunction));

	CreateScalarFunctionInfo iscollection_func_info(iscollection);
	catalog.AddFunction(*con.context, &iscollection_func_info);

	// ST_ISEMPTY
	ScalarFunctionSet isempty("st_isempty");
	isempty.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsEmptyFunction));

	CreateScalarFunctionInfo isempty_func_info(isempty);
	catalog.AddFunction(*con.context, &isempty_func_info);

	// ST_ISRING
	ScalarFunctionSet isring("st_isring");
	isring.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::BOOLEAN, GeoFunctions::GeometryIsRingFunction));

	CreateScalarFunctionInfo isring_func_info(isring);
	catalog.AddFunction(*con.context, &isring_func_info);

	// ST_NPOINTS
	ScalarFunctionSet npoints("st_npoints");
	npoints.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryNPointsFunction));

	CreateScalarFunctionInfo npoints_func_info(npoints);
	catalog.AddFunction(*con.context, &npoints_func_info);

	// ST_NUMGEOMETRIES
	ScalarFunctionSet numgeometries("st_numgeometries");
	numgeometries.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryNumGeometriesFunction));

	CreateScalarFunctionInfo numgeometries_func_info(numgeometries);
	catalog.AddFunction(*con.context, &numgeometries_func_info);

	// ST_NUMPOINTS
	ScalarFunctionSet numpoints("st_numpoints");
	numpoints.AddFunction(
	    ScalarFunction({geo_type}, LogicalType::INTEGER, GeoFunctions::GeometryNumPointsFunction));

	CreateScalarFunctionInfo numpoints_func_info(numpoints);
	catalog.AddFunction(*con.context, &numpoints_func_info);

	// ST_POINTN
	ScalarFunctionSet pointn("st_pointn");
	pointn.AddFunction(
	    ScalarFunction({geo_type, LogicalType::INTEGER}, geo_type, GeoFunctions::GeometryPointNFunction));

	CreateScalarFunctionInfo pointn_func_info(pointn);
	catalog.AddFunction(*con.context, &pointn_func_info);

	// ST_X
	ScalarFunctionSet get_x("st_x");
	get_x.AddFunction(ScalarFunction({geo_type}, LogicalType::DOUBLE, GeoFunctions::GeometryGetXFunction));

	CreateScalarFunctionInfo get_x_func_info(get_x);
	catalog.AddFunction(*con.context, &get_x_func_info);

	con.Commit();
}

std::string GeoExtension::Name() {
	return "geo";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void geo_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::GeoExtension>();
}

DUCKDB_EXTENSION_API const char *geo_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
