#include "duckdb.hpp"

using namespace duckdb;

int main() {
	DuckDB db(nullptr);

	Connection con(db);

	// auto extrv1 = con.Query("INSTALL 'fts';");
	// extrv1->Print();
	// auto extrv2 = con.Query("LOAD 'fts';");
	// extrv2->Print();

	// auto rv_ext = con.Query("select * From duckdb_extensions();");
	// rv_ext->Print();

	auto rv1 = con.Query("CREATE TABLE integers(i1 INTEGER, i2 INET, g Geometry)");
	rv1->Print();
	con.Query("INSERT INTO integers VALUES (1, '127.0.0000.2', 'POINT(100 0 100)')");
	con.Query("INSERT INTO integers VALUES (2, '127.0.0000.3', 'LINE(1 0)')");
	auto rv2 = con.Query("INSERT INTO integers VALUES (3, '127.0.0000.1', 'POINT(0 1)')");
	rv2->Print();
	con.Query("INSERT INTO integers VALUES (4, '127.0.0000.3', '{\"type\":\"Point\",\"coordinates\":[0,5]}')");
	con.Query("INSERT INTO integers VALUES (5, '127.0.0000.4', '010100000000000000000000000000000000004940')");
	con.Query("INSERT INTO integers VALUES (6, '127.0.0000.4', 'LINESTRING(0 0, 1 1, 2 1, 2 2)')");
	con.Query("INSERT INTO integers VALUES (7, '127.0.0000.4', 'SRID=4326;POINT(-72.1235 42.3521)')");
	// auto result = con.Query("SELECT i1, i2, ST_ASTEXT(g) FROM integers");
	auto result = con.Query("SELECT i1, i2, g FROM integers");
	result->Print();

	// auto rv3 = con.Query("SELECT ST_DISTANCE(ST_MakePoint(-7.1043443253471, 43.3150676015829), ST_MakePoint(-70.1043443253471, 42.3150676015829), true);");
	// rv3->Print();

	// auto rv4 = con.Query("SELECT ST_DISTANCE(ST_MakePoint(-7.1043443253471, 43.3150676015829), ST_MakePoint(-70.1043443253471, 42.3150676015829), false);");
	// rv4->Print();

	// auto rv5 = con.Query("SELECT ST_DISTANCE(ST_MakePoint(-7.1043443253471, 43.3150676015829), ST_MakePoint(-70.1043443253471, 42.3150676015829));");
	// rv5->Print();

	// auto rv6 = con.Query("SELECT ST_ASTEXT(ST_GeomFromWKB('\\x01\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x14@'::BLOB));");
	// rv6->Print();

	// auto rv7 = con.Query("SELECT '\\x01\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x14@'::BYTEA;");
	// rv7->Print();

	// con.Query("CREATE TABLE geometries (g Geometry);");
	// con.Query("INSERT INTO geometries VALUES('{\"type\":\"Point\",\"coordinates\":[30,10.2323]}'), ('{\"type\":\"Point\",\"coordinates\":[-71.064544,10.2323]}'), ('{\"type\":\"Point\",\"coordinates\":[30,43.28787]}');");
	// // con.Query("INSERT INTO geometries VALUES(''::GEOMETRY), (NULL::GEOMETRY), ('POINT(10 54)');");
	// auto rv8 = con.Query("SELECT g::VARCHAR FROM geometries;");
	// rv8->Print();



	// auto rv9 = con.Query("SELECT ''::GEOMETRY;");
	// rv9->Print();
}
