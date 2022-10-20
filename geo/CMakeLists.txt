cmake_minimum_required(VERSION 2.8.12)
set(TARGET_NAME geo)
project(${TARGET_NAME})

include_directories(include)

set(ALL_SOURCES geo-extension.cpp)

add_library(geo_extension STATIC ${ALL_SOURCES})

set(PARAMETERS "-no-warnings")

build_loadable_extension(${TARGET_NAME} ${PARAMETERS} ${ALL_SOURCES})

install(
  TARGETS geo_extension
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
