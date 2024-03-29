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
 * Last port: geomgraph/Edge.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <cassert>
#include <geos/export.hpp>
#include <geos/geom/CoordinateSequence.hpp> // for inlines
#include <geos/geom/Envelope.hpp>
#include <geos/geomgraph/Depth.hpp>                // for member
#include <geos/geomgraph/EdgeIntersectionList.hpp> // for composition
#include <geos/geomgraph/GraphComponent.hpp>       // for inheritance
#include <string>

// Forward declarations
namespace geos {
namespace geom {
class IntersectionMatrix;
class Coordinate;
} // namespace geom
namespace algorithm {
class LineIntersector;
}
namespace geomgraph {
class Node;
class EdgeEndStar;
class Label;
class NodeFactory;
namespace index {
class MonotoneChainEdge;
}
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

/** The edge component of a geometry graph */
class GEOS_DLL Edge : public GraphComponent {
	using GraphComponent::updateIM;

private:
	/// Lazily-created, owned by Edge.
	std::unique_ptr<index::MonotoneChainEdge> mce;

	geom::Envelope env;

	Depth depth;

	int depthDelta; // the change in area depth from the R to L side of this edge

	bool isIsolatedVar;

public:
	void testInvariant() const {
		assert(pts);
		assert(pts->size() > 1);
	}

	static void updateIM(const Label &lbl, geom::IntersectionMatrix &im);

	/// Takes ownership of CoordinateSequence
	Edge(geom::CoordinateSequence *newPts, const Label &newLabel);

	/// Takes ownership of CoordinateSequence
	Edge(geom::CoordinateSequence *newPts);

	EdgeIntersectionList eiList;

	/// Externally-set, owned by Edge. FIXME: refuse ownership
	std::unique_ptr<geom::CoordinateSequence> pts;

	~Edge() override;

	virtual const geom::Envelope *getEnvelope();

	/// return true if the coordinate sequences of the Edges are identical
	virtual bool isPointwiseEqual(const Edge *e) const;

	virtual Depth &getDepth() {
		testInvariant();
		return depth;
	}

	/** \brief
	 * The depthDelta is the change in depth as an edge is crossed from R to L
	 *
	 * @return the change in depth as the edge is crossed from R to L
	 */
	virtual int getDepthDelta() const {
		testInvariant();
		return depthDelta;
	}

	virtual void setDepthDelta(int newDepthDelta) {
		depthDelta = newDepthDelta;
		testInvariant();
	}

	/** \brief
	 * An Edge is collapsed if it is an Area edge and it consists of
	 * two segments which are equal and opposite (eg a zero-width V).
	 */
	virtual bool isCollapsed() const;

	virtual Edge *getCollapsedEdge();

	virtual size_t getNumPoints() const {
		return pts->getSize();
	}

	virtual const geom::CoordinateSequence *getCoordinates() const {
		testInvariant();
		return pts.get();
	}

	virtual const geom::Coordinate &getCoordinate(std::size_t i) const {
		testInvariant();
		return pts->getAt(i);
	}

	virtual const geom::Coordinate &getCoordinate() const {
		testInvariant();
		return pts->getAt(0);
	}

	virtual void setIsolated(bool newIsIsolated) {
		isIsolatedVar = newIsIsolated;
		testInvariant();
	}

	bool isIsolated() const override {
		testInvariant();
		return isIsolatedVar;
	}

	virtual bool isClosed() const {
		testInvariant();
		return pts->getAt(0) == pts->getAt(getNumPoints() - 1);
	}

	/// \brief
	/// Return this Edge's index::MonotoneChainEdge,
	/// ownership is retained by this object.
	///
	virtual index::MonotoneChainEdge *getMonotoneChainEdge();

	/** \brief
	 * Adds EdgeIntersections for one or both
	 * intersections found for a segment of an edge to the edge intersection list.
	 */
	virtual void addIntersections(algorithm::LineIntersector *li, std::size_t segmentIndex, std::size_t geomIndex);

	/// Add an EdgeIntersection for intersection intIndex.
	//
	/// An intersection that falls exactly on a vertex of the edge is normalized
	/// to use the higher of the two possible segmentIndexes
	///
	virtual void addIntersection(algorithm::LineIntersector *li, std::size_t segmentIndex, std::size_t geomIndex,
	                             std::size_t intIndex);

	/// Update the IM with the contribution for this component.
	//
	/// A component only contributes if it has a labelling for both
	/// parent geometries
	///
	void computeIM(geom::IntersectionMatrix &im) override {
		updateIM(label, im);
		testInvariant();
	}

	virtual EdgeIntersectionList &getEdgeIntersectionList() {
		testInvariant();
		return eiList;
	}
};

} // namespace geomgraph
} // namespace geos
