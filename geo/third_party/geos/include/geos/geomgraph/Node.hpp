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
 * Last port: geomgraph/Node.java r411 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <cassert>
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>          // for member
#include <geos/geomgraph/GraphComponent.hpp> // for inheritance
#include <string>

// Forward declarations
namespace geos {
namespace geom {
class IntersectionMatrix;
}
namespace geomgraph {
class Node;
class EdgeEndStar;
class EdgeEnd;
class Label;
class NodeFactory;
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

/** \brief The node component of a geometry graph. */
class GEOS_DLL Node : public GraphComponent {
public:
	Node(const geom::Coordinate &newCoord, EdgeEndStar *newEdges);

	~Node() override;

	virtual EdgeEndStar *getEdges();

	/** \brief
	 * Add the edge to the list of edges at this node
	 */
	virtual void add(EdgeEnd *e);

	virtual void mergeLabel(const Node &n);

	/** \brief
	 * To merge labels for two nodes,
	 * the merged location for each LabelElement is computed.
	 *
	 * The location for the corresponding node LabelElement is set
	 * to the result, as long as the location is non-null.
	 */
	virtual void mergeLabel(const Label &label2);

	bool isIsolated() const override;

	virtual const geom::Coordinate &getCoordinate() const;

	virtual void setLabel(uint8_t argIndex, geom::Location onLocation);

	/** \brief
	 * Updates the label of a node to BOUNDARY,
	 * obeying the mod-2 boundaryDetermination rule.
	 */
	virtual void setLabelBoundary(uint8_t argIndex);

	virtual void addZ(double);

	/** \brief
	 * Tests whether any incident edge is flagged as
	 * being in the result.
	 *
	 * This test can be used to determine if the node is in the result,
	 * since if any incident edge is in the result, the node must be in
	 * the result as well.
	 *
	 * @return <code>true</code> if any indicident edge in the in
	 *         the result
	 */
	virtual bool isIncidentEdgeInResult() const;

	/**
	 * The location for a given eltIndex for a node will be one
	 * of { null, INTERIOR, BOUNDARY }.
	 * A node may be on both the boundary and the interior of a geometry;
	 * in this case, the rule is that the node is considered to be
	 * in the boundary.
	 * The merged location is the maximum of the two input values.
	 */
	virtual geom::Location computeMergedLocation(const Label &label2, uint8_t eltIndex);

protected:
	void testInvariant() const;

	geom::Coordinate coord;

	EdgeEndStar *edges;

	/** \brief
	 * Basic nodes do not compute IMs
	 */
	void computeIM(geom::IntersectionMatrix & /*im*/) override {
	}

private:
	std::vector<double> zvals;

	double ztot;
};

inline void Node::testInvariant() const {
}

} // namespace geomgraph
} // namespace geos
