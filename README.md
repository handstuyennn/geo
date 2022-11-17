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
