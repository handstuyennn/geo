//===----------------------------------------------------------------------===//
//                         DuckDB
//
// geo-functions.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/function/cast/cast_function_set.hpp"
#include "duckdb/function/function_set.hpp"
#include "duckdb/function/scalar_function.hpp"

namespace duckdb {

struct GeoFunctions {
	static bool CastVarcharToGEO(Vector &source, Vector &result, idx_t count, CastParameters &parameters);
	static bool CastGeoToVarchar(Vector &source, Vector &result, idx_t count, CastParameters &parameters);
	static void MakePointFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void MakeLineFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void MakeLineArrayFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void MakePolygonFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryAsBinaryFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryAsTextFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryAsGeojsonFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryGeoHashFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryGeogFromFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryGeomFromGeoJsonFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryFromTextFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryFromWKBFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryFromGeoHashFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryGPointFromGeoHashFunction(DataChunk &args, ExpressionState &state, Vector &result);

	// **Accessors (15)**
	static void GeometryDimensionFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryDumpFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryEndPointFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryTypeFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryIsClosedFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryIsCollectionFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryIsEmptyFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryIsRingFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryNPointsFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryNumGeometriesFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryNumPointsFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryPointNFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryStartPointFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryGetXFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryGetYFunction(DataChunk &args, ExpressionState &state, Vector &result);

	// **Transformations (10)**:
	static void GeometryBoundaryFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryDifferenceFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryClosestPointFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryUnionFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryUnionArrayFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryIntersectionFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometrySimplifyFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryCentroidFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryConvexhullFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometrySnapToGridFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryBufferFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryBufferTextFunction(DataChunk &args, ExpressionState &state, Vector &result);

	// **Predicates (9)**
	static void GeometryEqualsFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryContainsFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryTouchesFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryWithinFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryIntersectsFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryCoversFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryCoveredByFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryDisjointFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryDWithinFunction(DataChunk &args, ExpressionState &state, Vector &result);

	// **Measures (9)**
	static void GeometryDistanceFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryAreaFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryAngleFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryPerimeterFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryAzimuthFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryLengthFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryBoundingBoxFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryMaxDistanceFunction(DataChunk &args, ExpressionState &state, Vector &result);
	static void GeometryExtentFunction(DataChunk &args, ExpressionState &state, Vector &result);
};

} // namespace duckdb
