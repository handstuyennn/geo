/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/LineBuilder.java rev. 1.15 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/operation/overlay/OverlayOp.hpp> // for OverlayOp::OpCode enum
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class CoordinateSequence;
class LineString;
} // namespace geom
namespace geomgraph {
class DirectedEdge;
class Edge;
} // namespace geomgraph
namespace algorithm {
class PointLocator;
}
namespace operation {
namespace overlay {
class OverlayOp;
}
} // namespace operation
} // namespace geos

namespace geos {
namespace operation { // geos::operation
namespace overlay {   // geos::operation::overlay

/** \brief
 * Forms JTS LineStrings out of a the graph of geomgraph::DirectedEdge
 * created by an OverlayOp.
 *
 */
class GEOS_DLL LineBuilder {
public:
	LineBuilder(OverlayOp *newOp, const geom::GeometryFactory *newGeometryFactory,
	            algorithm::PointLocator *newPtLocator);

	~LineBuilder() = default;

	/**
	 * @return a list of the LineStrings in the result of the specified overlay operation
	 */
	std::vector<geom::LineString *> *build(OverlayOp::OpCode opCode);

	/**
	 * Collect line edges which are in the result.
	 *
	 * Line edges are in the result if they are not part of
	 * an area boundary, if they are in the result of the overlay operation,
	 * and if they are not covered by a result area.
	 *
	 * @param de the directed edge to test.
	 * @param opCode the overlap operation
	 * @param edges the list of included line edges.
	 */
	void collectLineEdge(geomgraph::DirectedEdge *de, OverlayOp::OpCode opCode, std::vector<geomgraph::Edge *> *edges);

private:
	OverlayOp *op;
	const geom::GeometryFactory *geometryFactory;
	algorithm::PointLocator *ptLocator;
	std::vector<geomgraph::Edge *> lineEdgesList;
	std::vector<geom::LineString *> *resultLineList;
	void findCoveredLineEdges();
	void collectLines(OverlayOp::OpCode opCode);
	void buildLines(OverlayOp::OpCode opCode);

	/**
	 * Collect edges from Area inputs which should be in the result but
	 * which have not been included in a result area.
	 * This happens ONLY:
	 *
	 *  -  during an intersection when the boundaries of two
	 *     areas touch in a line segment
	 *  -   OR as a result of a dimensional collapse.
	 *
	 */
	void collectBoundaryTouchEdge(geomgraph::DirectedEdge *de, OverlayOp::OpCode opCode,
	                              std::vector<geomgraph::Edge *> *edges);

	/*
	 * If the given CoordinateSequence has mixed 3d/2d vertexes
	 * set Z for all vertexes missing it.
	 * The Z value is interpolated between 3d vertexes and copied
	 * from a 3d vertex to the end.
	 */
	void propagateZ(geom::CoordinateSequence *cs);
};

} // namespace overlay
} // namespace operation
} // namespace geos
