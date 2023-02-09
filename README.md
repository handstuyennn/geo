# DuckDB geo extension
This extension will provide a `GEO` type for DuckDB and enable basic GIS data analysis in it. It is currently a work-in-progress. 
It only support [geography](https://gis.stackexchange.com/questions/26082/what-is-the-difference-between-geometric-and-geographic-columns) types and operations.

## Installation


Actually the geo is build with the version v0.6.2-dev1218.

Be sure to have a compiler and git client installed.

Clone into a folder.
```
$ git clone ...
$ cd geo
```

Build it with the command `make`

At the end of compilation you should have a new build directory present.

```
$ ls
CMakeLists.txt README.md      duckdb         geo
Makefile       build          examples       test
```

Copy the geo duckdb extension to the extensions directory of DuckDB.

`cp build/release/extension/geo/geo.duckdb_extension ~/.duckdb/extensions/e2dfc274b0/osx_arm64`

note : adapt to your OS and duckdb build

## Usage with DuckDB

### CLI

Launch duckdb with unsigned parameter.

```
duckdb -unsigned
v0.6.2-dev1218 e2dfc274b0
Enter ".help" for usage hints.
D LOAD geo;
D SELECT ST_MAKEPOINT(52.347113,4.869454);
┌────────────────────────────────────────────┐
│     st_makepoint(52.347113, 4.869454)      │
│                 geography                  │
├────────────────────────────────────────────┤
│ 01010000001B82E3326E2C4A406B813D26527A1340 │
```

### Python


Install the master version of DuckDB.

```
$ python
>>> import duckdb

>>> con = duckdb.connect(config={'allow_unsigned_extensions' : 'true'})
>>> con.load('geo')
>>> result = con.execute("SELECT ST_MAKEPOINT(52.347113,4.869454);")
>>> print(result.fetchall())
[(b'\x01\x01\x00\x00\x00\x1b\x82\xe32n,J@k\x81=&Rz\x13@',)]
```


## Supported functions

**Constructors (3)**
- [x] [`ST_MAKEPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)  (Alias: `ST_GEOGPOINT`)
- [x] [`ST_MAKELINE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makeline)  
- [x] [`ST_MAKEPOLYGON`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygon)  

**Formatters (4)**
- [x] [`ST_ASBINARY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asbinary)  
- [x] [`ST_ASGEOJSON`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_asgeojson)  
- [x] [`ST_ASTEXT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_astext)  
- [x] [`ST_GEOHASH`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geohash)

**Parsers (5)**
- [x] [`ST_GEOGFROM`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfrom)  
- [x] [`ST_GEOGFROMGEOJSON`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromgeojson)  
- [x] [`ST_GEOGFROMTEXT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromtext)  
- [x] [`ST_GEOGFROMWKB`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogfromwkb)  
- [x] [`ST_GEOGPOINTFROMGEOHASH`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpointfromgeohash)

**Accessors (15)**:
- [x] [`ST_DIMENSION`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dimension)  
- [x] [`ST_DUMP`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dump)  
- [x] [`ST_ENDPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_endpoint)  
- [x] [`ST_GEOMETRYTYPE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geometrytype)  
- [x] [`ST_ISCLOSED`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isclosed)  
- [x] [`ST_ISCOLLECTION`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_iscollection)  
- [x] [`ST_ISEMPTY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isempty)  
- [x] [`ST_ISRING`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isring)  
- [x] [`ST_NPOINTS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_npoints)  
- [x] [`ST_NUMGEOMETRIES`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numgeometries)  
- [x] [`ST_NUMPOINTS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numpoints)  
- [x] [`ST_POINTN`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_pointn)  
- [x] [`ST_STARTPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_startpoint)  
- [x] [`ST_X`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_x)  
- [x] [`ST_Y`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_y)

**Transformations (10)**:
- [x] [`ST_BOUNDARY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundary)  
- [x] [`ST_BUFFER`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_buffer)  
- [x] [`ST_CENTROID`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid)  
- [x] [`ST_CLOSESTPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_closestpoint)  
- [x] [`ST_CONVEXHULL`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_convexhull)  
- [x] [`ST_DIFFERENCE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_difference)  
- [x] [`ST_INTERSECTION`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersection)  
- [x] [`ST_SIMPLIFY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify)  
- [x] [`ST_SNAPTOGRID`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid)  
- [x] [`ST_UNION`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union)  

**Predicates (9)**
- [x] [`ST_CONTAINS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_contains)  
- [x] [`ST_COVEREDBY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_coveredby)  
- [x] [`ST_COVERS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_covers)  
- [x] [`ST_DISJOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_disjoint)  
- [x] [`ST_DWITHIN`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_dwithin)  
- [x] [`ST_EQUALS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_equals)  
- [x] [`ST_INTERSECTS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersects)  
- [x] [`ST_TOUCHES`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_touches)  
- [x] [`ST_WITHIN`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_within)

**Measures (9)**:
- [x] [`ST_ANGLE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_angle)  
- [x] [`ST_AREA`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_area)  
- [x] [`ST_AZIMUTH`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_azimuth)  
- [x] [`ST_BOUNDINGBOX`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundingbox)  (alias: `ST_ENVELOPE`)
- [x] [`ST_DISTANCE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_distance)  
- [x] [`ST_EXTENT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_extent)
- [x] [`ST_LENGTH`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_length)  
- [x] [`ST_MAXDISTANCE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_maxdistance)  
- [x] [`ST_PERIMETER`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_perimeter)

**Other (1)**
- [x] [`ST_CLUSTERDBSCAN`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_clusterdbscan)
