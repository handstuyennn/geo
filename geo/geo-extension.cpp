#define DUCKDB_EXTENSION_MAIN

#include "geo-extension.hpp"

#include "accessor-functions.hpp"
#include "constructor-functions.hpp"
#include "duckdb.hpp"
#include "duckdb/catalog/catalog.hpp"
#include "duckdb/function/aggregate/sum_helpers.hpp"
#include "duckdb/parser/parsed_data/create_aggregate_function_info.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/parser/parsed_data/create_type_info.hpp"
#include "formatter-functions.hpp"
#include "geo_aggregate_function.hpp"
#include "measure-functions.hpp"
#include "parser-functions.hpp"
#include "predicate-functions.hpp"
#include "transformation-functions.hpp"

namespace duckdb {

void GeoExtension::Load(DuckDB &db) {
	Connection con(db);
	con.BeginTransaction();

	auto &catalog = Catalog::GetSystemCatalog(*con.context);

	auto geo_type = LogicalType(LogicalTypeId::BLOB);
	geo_type.SetAlias("GEOGRAPHY");

	CreateTypeInfo info("Geography", geo_type);
	info.temporary = true;
	info.internal = true;
	catalog.CreateType(*con.context, &info);

	// add geo casts
	auto &config = DBConfig::GetConfig(*con.context);

	auto &casts = config.GetCastFunctions();
	casts.RegisterCastFunction(LogicalType::VARCHAR, geo_type, GeoFunctions::CastVarcharToGEO, 100);
	casts.RegisterCastFunction(geo_type, LogicalType::VARCHAR, GeoFunctions::CastGeoToVarchar);

	// add geo functions
	std::vector<ScalarFunctionSet> geo_function_set {};
	// **Constructors (3)**
	auto constructor_func_set = GetConstructorScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), constructor_func_set.begin(), constructor_func_set.end());
	// **Formatters (4)**
	auto formatter_func_set = GetFormatterScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), formatter_func_set.begin(), formatter_func_set.end());
	// **Parsers (5)**
	auto parser_func_set = GetParserScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), parser_func_set.begin(), parser_func_set.end());
	// **Transformations (10)**
	auto transformation_func_set = GetTransformationScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), transformation_func_set.begin(), transformation_func_set.end());
	//  **Accessors (15)**
	auto accessor_func_set = GetAccessorScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), accessor_func_set.begin(), accessor_func_set.end());
	// **Predicates (9)**
	auto predicate_func_set = GetPredicateScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), predicate_func_set.begin(), predicate_func_set.end());
	// **Measures (9)**
	auto measure_func_set = GetMeasureScalarFunctions(geo_type);
	geo_function_set.insert(geo_function_set.end(), measure_func_set.begin(), measure_func_set.end());

	for (auto func_set : geo_function_set) {
		CreateScalarFunctionInfo func_info(func_set);
		catalog.AddFunction(*con.context, &func_info);
	}

	auto cluster_db_scan = GetClusterDBScanAggregateFunction(geo_type);
	CreateAggregateFunctionInfo cluster_db_scan_func_info(move(cluster_db_scan));
	catalog.CreateFunction(*con.context, &cluster_db_scan_func_info);

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
