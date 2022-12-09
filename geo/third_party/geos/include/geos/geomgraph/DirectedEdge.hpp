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
 * Last port: geomgraph/DirectedEdge.java r428 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geomgraph/EdgeEnd.hpp> // for inheritance
#include <string>

// Forward declarations
namespace geos {
namespace geomgraph {
class Edge;
class EdgeRing;
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

/// A directed EdgeEnd
class GEOS_DLL DirectedEdge : public EdgeEnd {
public:
	// DirectedEdge();
	// virtual ~DirectedEdge();

	DirectedEdge(Edge *newEdge, bool newIsForward);

	/** \brief
	 * Tells wheter this edge is an Area
	 *
	 * This is an interior Area edge if
	 * - its label is an Area label for both Geometries
	 * - and for each Geometry both sides are in the interior.
	 *
	 * @return true if this is an interior Area edge
	 */
	bool isInteriorAreaEdge();

	// this is no different from Base class, no need to override
	// Edge* getEdge();

	void setInResult(bool v) {
		isInResultVar = v;
	};

	bool isInResult() const {
		return isInResultVar;
	};

	bool isVisited() const {
		return isVisitedVar;
	};

	void setVisited(bool v) {
		isVisitedVar = v;
	};

	EdgeRing *getEdgeRing() const {
		return edgeRing;
	};

	/** \brief
	 * Each Edge gives rise to a pair of symmetric DirectedEdges,
	 * in opposite directions.
	 *
	 * @return the DirectedEdge for the same Edge but in the
	 *         opposite direction
	 */
	DirectedEdge *getSym() const {
		return sym;
	};

	void setSym(DirectedEdge *de) {
		sym = de;
	};

	DirectedEdge *getNext() const {
		return next;
	};

	void setNext(DirectedEdge *newNext) {
		next = newNext;
	};

	DirectedEdge *getNextMin() const {
		return nextMin;
	};

	void setNextMin(DirectedEdge *nm) {
		nextMin = nm;
	};

	void setMinEdgeRing(EdgeRing *mer) {
		minEdgeRing = mer;
	};

	EdgeRing *getMinEdgeRing() const {
		return minEdgeRing;
	};

	void setEdgeRing(EdgeRing *er) {
		edgeRing = er;
	};

	bool isForward() const {
		return isForwardVar;
	};

	int getDepth(int position) const {
		return depth[position];
	};

	/** \brief
	 * Tells wheter this edge is a Line
	 *
	 * This edge is a line edge if
	 * - at least one of the labels is a line label
	 * - any labels which are not line labels have all Locations = EXTERIOR
	 *
	 */
	bool isLineEdge();

	/// Marks both DirectedEdges attached to a given Edge.
	///
	/// This is used for edges corresponding to lines, which will only
	/// appear oriented in a single direction in the result.
	///
	void setVisitedEdge(bool newIsVisited);

	/** \brief
	 * Set both edge depths.
	 *
	 * One depth for a given side is provided.
	 * The other is computed depending on the Location transition and the
	 * depthDelta of the edge.
	 */
	void setEdgeDepths(int position, int newDepth);

	void setDepth(int position, int newDepth);

protected:
	bool isForwardVar;

private:
	bool isInResultVar;

	bool isVisitedVar;

	/// the symmetric edge
	DirectedEdge *sym;

	/// the next edge in the edge ring for the polygon containing this edge
	DirectedEdge *next;

	/// the next edge in the MinimalEdgeRing that contains this edge
	DirectedEdge *nextMin;

	/// the EdgeRing that this edge is part of
	EdgeRing *edgeRing;

	/// the MinimalEdgeRing that this edge is part of
	EdgeRing *minEdgeRing;

	/** \brief
	 * The depth of each side (position) of this edge.
	 * The 0 element of the array is never used.
	 */
	int depth[3];

	/// Compute the label in the appropriate orientation for this DirEdge
	void computeDirectedLabel();
};

} // namespace geomgraph
} // namespace geos
