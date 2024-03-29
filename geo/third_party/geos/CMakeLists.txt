add_library(
  duckdb_geo_geos OBJECT
  geos_c.cpp
  geos_ts_c.cpp
  algorithm/BoundaryNodeRule.cpp
  algorithm/Orientation.cpp
  algorithm/PointLocation.cpp
  algorithm/PointLocator.cpp
  algorithm/CGAlgorithumsDD.cpp
  algorithm/LineIntersector.cpp
  algorithm/RayCrossingCounter.cpp
  algorithm/PolygonNodeTopology.cpp
  algorithm/locate/IndexedPointInAreaLocator.cpp
  algorithm/locate/SimplePointInAreaLocator.cpp
  algorithm/Distance.cpp
  algorithm/Area.cpp
  algorithm/Angle.cpp
  algorithm/Intersection.cpp
  algorithm/Centroid.cpp
  algorithm/ConvexHull.cpp
  algorithm/Length.cpp
  edgegraph/HalfEdge.cpp
  math/DD.cpp
  operation/valid/IsSimpleOp.cpp
  operation/valid/IsValidOp.cpp
  operation/valid/RepeatedPointRemover.cpp
  operation/valid/IndexedNestedHoleTester.cpp
  operation/valid/PolygonTopologyAnalyzer.cpp
  operation/valid/PolygonRing.cpp
  operation/valid/TopologyValidationError.cpp
  operation/valid/IndexedNestedPolygonTester.cpp
  operation/valid/PolygonRingTouch.cpp
  operation/valid/PolygonRingSelfNode.cpp
  operation/valid/PolygonIntersectionAnalyzer.cpp
  operation/valid/RepeatedPointTester.cpp
  operation/overlay/snap/GeometrySnapper.cpp
  operation/overlay/snap/LineStringSnapper.cpp
  operation/overlay/OverlayOp.cpp
  operation/overlay/PointBuilder.cpp
  operation/overlay/LineBuilder.cpp
  operation/overlay/PolygonBuilder.cpp
  operation/overlay/ElevationMatrix.cpp
  operation/GeometryGraphOperation.cpp
  operation/BoundaryOp.cpp
  operation/polygonize/EdgeRing.cpp
  operation/overlay/MaximalEdgeRing.cpp
  operation/overlay/MinimalEdgeRing.cpp
  operation/overlay/ElevationMatrixCell.cpp
  operation/overlay/OverlayNodeFactory.cpp
  operation/overlayng/OverlayNGRobust.cpp
  operation/overlayng/InputGeometry.cpp
  operation/overlayng/PrecisionUtil.cpp
  operation/overlayng/OverlayNG.cpp
  operation/overlayng/LineBuilder.cpp
  operation/overlayng/OverlayUtil.cpp
  operation/overlayng/OverlayEdge.cpp
  operation/overlayng/OverlayGraph.cpp
  operation/overlayng/OverlayPoints.cpp
  operation/overlayng/ElevationModel.cpp
  operation/overlayng/PolygonBuilder.cpp
  operation/overlayng/MaximalEdgeRing.cpp
  operation/overlayng/OverlayEdgeRing.cpp
  operation/overlayng/OverlayLabeller.cpp
  operation/overlayng/OverlayLabel.cpp
  operation/overlayng/EdgeNodingBuilder.cpp
  operation/overlayng/EdgeMerger.cpp
  operation/overlayng/LineLimiter.cpp
  operation/overlayng/EdgeSourceInfo.cpp
  operation/overlayng/OverlayMixedPoints.cpp
  operation/overlayng/IntersectionPointBuilder.cpp
  operation/overlayng/RobustClipEnvelopeComputer.cpp
  operation/overlayng/Edge.cpp
  operation/overlayng/PrecisionReducer.cpp
  operation/overlayng/RingClipper.cpp
  operation/overlayng/IndexedPointOnLineLocator.cpp
  operation/union/UnaryUnionOp.cpp
  operation/union/PointGeometryUnion.cpp
  operation/union/UnionStrategy.cpp
  operation/union/CascadedPolygonUnion.cpp
  operation/buffer/BufferOp.cpp
  operation/buffer/BufferParameters.cpp
  operation/buffer/BufferBuilder.cpp
  operation/buffer/BufferCurveSetBuilder.cpp
  operation/buffer/BufferSubgraph.cpp
  operation/buffer/SubgraphDepthLocater.cpp
  operation/buffer/OffsetCurveBuilder.cpp
  operation/buffer/RightmostEdgeFinder.cpp
  operation/buffer/OffsetSegmentGenerator.cpp
  operation/buffer/BufferInputLineSimplifier.cpp
  operation/relate/RelateComputer.cpp
  operation/relate/RelateOp.cpp
  operation/relate/RelateNodeFactory.cpp
  operation/relate/EdgeEndBundleStar.cpp
  operation/relate/EdgeEndBuilder.cpp
  operation/relate/RelateNode.cpp
  operation/relate/EdgeEndBundle.cpp
  operation/predicate/RectangleContains.cpp
  operation/predicate/RectangleIntersects.cpp
  operation/predicate/SegmentIntersectionTester.cpp
  noding/MCIndexNoder.cpp
  noding/NodedSegmentString.cpp
  noding/SegmentNodeList.cpp
  noding/SegmentNode.cpp
  noding/Octant.cpp
  noding/FastNodingValidator.cpp
  noding/ValidatingNoder.cpp
  noding/IntersectionAdder.cpp
  noding/OrientedCoordinateArray.cpp
  noding/NodingIntersectionFinder.cpp
  noding/IntersectionFinderAdder.cpp
  noding/NodingValidator.cpp
  noding/ScaledNoder.cpp
  noding/snap/SnappingPointIndex.cpp
  noding/snap/SnappingNoder.cpp
  noding/snap/SnappingIntersectionAdder.cpp
  noding/snapround/HotPixelIndex.cpp
  noding/snapround/HotPixel.cpp
  noding/snapround/SnapRoundingNoder.cpp
  noding/snapround/SnapRoundingIntersectionAdder.cpp
  precision/CommonBitsRemover.cpp
  precision/CommonBits.cpp
  precision/GeometryPrecisionReducer.cpp
  precision/PrecisionReducerTransformer.cpp
  precision/PointwisePrecisionReducerTransformer.cpp
  util/Interrupt.cpp
  util/Assert.cpp
  util/math.cpp
  index/chain/MonotoneChainBuilder.cpp
  index/chain/MonotoneChain.cpp
  index/chain/MonotoneChainOverlapAction.cpp
  index/kdtree/KdNode.cpp
  index/kdtree/KdTree.cpp
  index/strtree/SimpleSTRtree.cpp
  index/strtree/SimpleSTRnode.cpp
  geomgraph/index/MonotoneChainEdge.cpp
  geomgraph/index/MonotoneChainIndexer.cpp
  geomgraph/index/SegmentIntersector.cpp
  geomgraph/index/SweepLineEvent.cpp
  geomgraph/NodeFactory.cpp
  geomgraph/PlanarGraph.cpp
  geomgraph/DirectedEdge.cpp
  geomgraph/GeometryGraph.cpp
  geomgraph/PlannarGraph.cpp
  geomgraph/DirectedEdgeStar.cpp
  geomgraph/TopologyLocation.cpp
  geomgraph/EdgeNodingValidator.cpp
  geomgraph/EdgeIntersectionList.cpp
  geomgraph/Node.cpp
  geomgraph/Edge.cpp
  geomgraph/GraphComponent.cpp
  geomgraph/Depth.cpp
  geomgraph/EdgeEnd.cpp
  geomgraph/NodeMap.cpp
  geomgraph/EdgeList.cpp
  geomgraph/EdgeRing.cpp
  geomgraph/EdgeEndStar.cpp
  geomgraph/index/SimpleMCSweepLineIntersector.cpp
  geom/Coordinate.cpp
  geom/Geometry.cpp
  geom/GeometryFactory.cpp
  geom/LinearRing.cpp
  geom/LineString.cpp
  geom/Point.cpp
  geom/Polygon.cpp
  geom/MultiPoint.cpp
  geom/MultiPolygon.cpp
  geom/MultiLineString.cpp
  geom/Triangle.cpp
  geom/CoordinateArraySequence.cpp
  geom/HeuristicOverlay.cpp
  geom/PrecisionModel.cpp
  geom/DefaultCoordinateSequencefactory.cpp
  geom/GeometryComponentFilter.cpp
  geom/Envelope.cpp
  geom/CoordinateSequence.cpp
  geom/Location.cpp
  geom/LineSegment.cpp
  geom/Position.cpp
  geom/util/GeometryEditor.cpp
  geom/util/CoordinateOperation.cpp
  geom/util/LinearComponentExtracter.cpp
  geom/util/GeometryCombiner.cpp
  geom/util/NoOpGeometryOperation.cpp
  geom/util/GeometryTransformer.cpp
  geom/util/ShortCircuitedGeometryVisitor.cpp
  geom/IntersectionMatrix.cpp
  geom/Dimension.cpp
  geom/GeometryCollection.cpp)
set(GEO_LIBRARY_FILES
    ${GEO_LIBRARY_FILES} $<TARGET_OBJECTS:duckdb_geo_geos>
    PARENT_SCOPE)
