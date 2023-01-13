cmake_minimum_required(VERSION 2.8.12)
set(TARGET_NAME geo)
project(${TARGET_NAME})

include_directories(include)
include_directories(third_party/json/include)
include_directories(third_party/geos/include)

add_subdirectory(third_party)

set(GEO_EXTENSION_FILES
    ${GEO_LIBRARY_FILES}
    geo-extension.cpp
    geo-functions.cpp
    postgis.cpp
    geometry.cpp
    postgis/lwgeom_inout.cpp
    postgis/lwgeom_functions_basic.cpp
    postgis/lwgeom_functions_analytic.cpp
    postgis/geography_measurement.cpp
    postgis/geography_measurement_trees.cpp
    postgis/lwgeom_ogc.cpp
    postgis/lwgeom_geos.cpp
    postgis/geography_centroid.cpp
    postgis/lwgeom_export.cpp
    postgis/lwgeom_in_geojson.cpp
    postgis/lwgeom_in_geohash.cpp
    postgis/lwgeom_box.cpp
    postgis/lwgeom_dump.cpp
    postgis/lwgeom_window.cpp
    liblwgeom/lwin_wkt.cpp
    liblwgeom/lwin_wkb.cpp
    liblwgeom/lwutil.cpp
    liblwgeom/ptarray.cpp
    liblwgeom/lwpoint.cpp
    liblwgeom/lwgeom.cpp
    liblwgeom/gbox.cpp
    liblwgeom/lwout_wkb.cpp
    liblwgeom/lwgeodetic.cpp
    liblwgeom/lwalgorithm.cpp
    liblwgeom/gserialized.cpp
    liblwgeom/gserialized1.cpp
    liblwgeom/gserialized2.cpp
    liblwgeom/lwgeom_api.cpp
    liblwgeom/lwout_wkt.cpp
    liblwgeom/stringbuffer.cpp
    liblwgeom/lwprint.cpp
    liblwgeom/lwin_geojson.cpp
    liblwgeom/lwout_geojson.cpp
    liblwgeom/measures.cpp
    liblwgeom/lwgeodetic_tree.cpp
    liblwgeom/lwspheroid.cpp
    liblwgeom/lwline.cpp
    liblwgeom/lwcircstring.cpp
    liblwgeom/lwcollection.cpp
    liblwgeom/lwcompound.cpp
    liblwgeom/lwmline.cpp
    liblwgeom/lwmpoint.cpp
    liblwgeom/lwtriangle.cpp
    liblwgeom/lwpoly.cpp
    liblwgeom/lwcurvepoly.cpp
    liblwgeom/lwmpoly.cpp
    liblwgeom/lwiterator.cpp
    liblwgeom/lwpsurface.cpp
    liblwgeom/lwtin.cpp
    liblwgeom/lwgeom_geos.cpp
    liblwgeom/lwstroke.cpp
    liblwgeom/lwunionfind.cpp
    liblwgeom/lwgeom_geos_cluster.cpp
    parser/lwin_wkt_lex.cpp
    parser/lwin_wkt_parse.cpp
    libpgcommon/lwgeom_pg.cpp
    libpgcommon/lwgeom_transform.cpp
    lib/ryu.cpp)

add_library(geo_extension STATIC ${GEO_EXTENSION_FILES})

set(PARAMETERS "-no-warnings")

build_loadable_extension(${TARGET_NAME} ${PARAMETERS} ${GEO_EXTENSION_FILES})

install(
  TARGETS geo_extension
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
