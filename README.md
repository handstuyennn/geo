# DuckDB geo extension
This extension will provide a `GEO` type for DuckDB and enable basic GIS data analysis in it. It is currently a work-in-progress. A `TODO` will be shared shortly.

## Supported functions

**Constructors (3)**
​
- [x] [`ST_MAKEPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_geogpoint)  (Alias: `ST_GEOGPOINT`)
- [x] [`ST_MAKELINE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makeline)  
- [x] [`ST_MAKEPOLYGON`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_makepolygon)  
​
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
- [ ] [`ST_ISRING`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_isring)  
- [x] [`ST_NPOINTS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_npoints)  
- [x] [`ST_NUMGEOMETRIES`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numgeometries)  
- [x] [`ST_NUMPOINTS`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_numpoints)  
- [x] [`ST_POINTN`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_pointn)  
- [x] [`ST_STARTPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_startpoint)  
- [x] [`ST_X`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_x)  
- [x] [`ST_Y`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_y)

**Transformations (10)**:
- [ ] [`ST_BOUNDARY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_boundary)  
- [ ] [`ST_BUFFER`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_buffer)  
- [ ] [`ST_CENTROID`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_centroid)  
- [x] [`ST_CLOSESTPOINT`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_closestpoint)  
- [ ] [`ST_CONVEXHULL`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_convexhull)  
- [ ] [`ST_DIFFERENCE`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_difference)  
- [ ] [`ST_INTERSECTION`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_intersection)  
- [ ] [`ST_SIMPLIFY`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_simplify)  
- [ ] [`ST_SNAPTOGRID`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_snaptogrid)  
- [x] [`ST_UNION`](https://cloud.google.com/bigquery/docs/reference/standard-sql/geography_functions#st_union)  