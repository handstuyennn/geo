/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/GeometryGraph.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp> // for unique_ptr<CoordinateSequence>
#include <geos/geom/LineString.hpp>         // for LineStringLT
#include <geos/geomgraph/PlanarGraph.hpp>
#include <geos/geomgraph/index/SegmentIntersector.hpp>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class LineString;
class LinearRing;
class Polygon;
class Geometry;
class GeometryCollection;
class Point;
class Envelope;
} // namespace geom
namespace algorithm {
class LineIntersector;
class BoundaryNodeRule;
} // namespace algorithm
namespace geomgraph {
class Edge;
class Node;
namespace index {
class EdgeSetIntersector;
}
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

/** \brief
 * A GeometryGraph is a graph that models a given Geometry.
 */
class GEOS_DLL GeometryGraph : public PlanarGraph {
	using PlanarGraph::add;
	using PlanarGraph::findEdge;

private:
	const geom::Geometry *parentGeom;

	/**
	 * The lineEdgeMap is a map of the linestring components of the
	 * parentGeometry to the edges which are derived from them.
	 * This is used to efficiently perform findEdge queries
	 *
	 * Following the above description there's no need to
	 * compare LineStrings other then by pointer value.
	 */
	std::unordered_map<const geom::LineString *, Edge *> lineEdgeMap;

	/**
	 * the index of this geometry as an argument to a spatial function
	 * (used for labelling)
	 */
	uint8_t argIndex;

	bool hasTooFewPointsVar;

	geom::Coordinate invalidPoint;

	/**
	 * If this flag is true, the Boundary Determination Rule will
	 * used when deciding whether nodes are in the boundary or not
	 */
	bool useBoundaryDeterminationRule;

	const algorithm::BoundaryNodeRule &boundaryNodeRule;

	/// Cache for fast responses to getBoundaryPoints
	std::unique_ptr<geom::CoordinateSequence> boundaryPoints;

	std::unique_ptr<std::vector<Node *>> boundaryNodes;

	/// Allocates a new EdgeSetIntersector. Remember to delete it!
	index::EdgeSetIntersector *createEdgeSetIntersector();

	void addSelfIntersectionNodes(uint8_t p_argIndex);

	/** \brief
	 * Add a node for a self-intersection.
	 *
	 * If the node is a potential boundary node (e.g. came from an edge
	 * which is a boundary) then insert it as a potential boundary node.
	 * Otherwise, just add it as a regular node.
	 */
	void addSelfIntersectionNode(uint8_t p_argIndex, const geom::Coordinate &coord, geom::Location loc);

	void insertPoint(uint8_t p_argIndex, const geom::Coordinate &coord, geom::Location onLocation);

	/** \brief
	 * Adds candidate boundary points using the current
	 * algorithm::BoundaryNodeRule.
	 *
	 * This is used to add the boundary
	 * points of dim-1 geometries (Curves/MultiCurves).
	 */
	void insertBoundaryPoint(uint8_t p_argIndex, const geom::Coordinate &coord);

	void add(const geom::Geometry *g);

	void addCollection(const geom::GeometryCollection *gc);

	void addPoint(const geom::Point *p);

	void addPolygonRing(const geom::LinearRing *lr, geom::Location cwLeft, geom::Location cwRight);

	void addPolygon(const geom::Polygon *p);

	void addLineString(const geom::LineString *line);

	void addPoint(geom::Coordinate &pt);

public:
	GeometryGraph(uint8_t newArgIndex, const geom::Geometry *newParentGeom);

	GeometryGraph(uint8_t newArgIndex, const geom::Geometry *newParentGeom,
	              const algorithm::BoundaryNodeRule &boundaryNodeRule);

	~GeometryGraph() override {};

	static bool isInBoundary(int boundaryCount);

	static geom::Location determineBoundary(int boundaryCount);

	static geom::Location determineBoundary(const algorithm::BoundaryNodeRule &boundaryNodeRule, int boundaryCount);

	/// Returned object is owned by this GeometryGraph
	void getBoundaryNodes(std::vector<Node *> &bdyNodes) {
		nodes->getBoundaryNodes(static_cast<uint8_t>(argIndex), bdyNodes);
	};

	std::vector<Node *> *getBoundaryNodes();

	/**
	 * \brief
	 * Compute self-nodes, taking advantage of the Geometry type to minimize
	 * the number of intersection tests. (E.g. rings are not tested for
	 * self-intersection, since they are assumed to be valid).
	 *
	 * @param li the LineIntersector to use
	 * @param computeRingSelfNodes if `false`, intersection checks are optimized
	 *                             to not test rings for self-intersection
	 * @param env an Envelope
	 *
	 * @return the SegmentIntersector used, containing information about
	 *         the intersections found
	 */
	std::unique_ptr<index::SegmentIntersector>
	computeSelfNodes(algorithm::LineIntersector *li, bool computeRingSelfNodes, const geom::Envelope *env = nullptr) {
		return computeSelfNodes(*li, computeRingSelfNodes, env);
	}

	// Quick inline calling the function above, the above should probably
	// be deprecated.
	std::unique_ptr<index::SegmentIntersector>
	computeSelfNodes(algorithm::LineIntersector &li, bool computeRingSelfNodes, const geom::Envelope *env = nullptr);

	std::unique_ptr<index::SegmentIntersector> computeEdgeIntersections(GeometryGraph *g,
	                                                                    algorithm::LineIntersector *li,
	                                                                    bool includeProper,
	                                                                    const geom::Envelope *env = nullptr);

	void computeSplitEdges(std::vector<Edge *> *edgelist);

	std::vector<Edge *> *getEdges();

	const geom::Geometry *getGeometry() {
		return parentGeom;
	};

	const algorithm::BoundaryNodeRule &getBoundaryNodeRule() const {
		return boundaryNodeRule;
	}
};

} // namespace geomgraph
} // namespace geos
