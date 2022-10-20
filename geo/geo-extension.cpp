#define DUCKDB_EXTENSION_MAIN

#include "geo-extension.hpp"

namespace duckdb
{

    void GeoExtension::Load(DuckDB &db)
    {
        Connection con(db);
        con.BeginTransaction();
        auto &catalog = Catalog::GetCatalog(*con.context);
        con.Commit();
    }

    std::string GeoExtension::Name() { return "geo"; }
} // namespace duckdb

extern "C"
{

    DUCKDB_EXTENSION_API void geo_init(duckdb::DatabaseInstance &db)
    {
        duckdb::DuckDB db_wrapper(db);
        db_wrapper.LoadExtension<duckdb::GeoExtension>();
    }

    DUCKDB_EXTENSION_API const char *geo_version()
    {
        return duckdb::DuckDB::LibraryVersion();
    }
}
