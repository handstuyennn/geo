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
 * Last port: geomgraph/DirectedEdgeStar.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>       // for p0,p1
#include <geos/geomgraph/EdgeEndStar.hpp> // for inheritance
#include <geos/geomgraph/Label.hpp>       // for private member
#include <set>
#include <string>
#include <vector>

// Forward declarations
namespace geos {
namespace geomgraph {
class DirectedEdge;
class EdgeRing;
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

/**
 * \brief
 * A DirectedEdgeStar is an ordered list of **outgoing** DirectedEdges around a node.
 *
 * It supports labelling the edges as well as linking the edges to form both
 * MaximalEdgeRings and MinimalEdgeRings.
 *
 */
class GEOS_DLL DirectedEdgeStar : public EdgeEndStar {
public:
	DirectedEdgeStar() : EdgeEndStar(), label(), resultAreaEdgesComputed(false) {
	}

	~DirectedEdgeStar() override = default;

	/// Insert a directed edge in the list
	void insert(EdgeEnd *ee) override;

	Label &getLabel() {
		return label;
	}

	/// \brief Update incomplete dirEdge labels from the labelling for the node
	void updateLabelling(const Label &nodeLabel);

	/** \brief
	 * For each dirEdge in the star, merge the label from the sym dirEdge into the label
	 */
	void mergeSymLabels();

	/** \brief
	 * Traverse the star of edges, maintaing the current location in the result
	 * area at this node (if any).
	 *
	 * If any L edges are found in the interior of the result, mark them as covered.
	 */
	void findCoveredLineEdges();

	/** \brief
	 * Traverse the star of DirectedEdges, linking the included edges together.
	 *
	 * To link two dirEdges, the `next` pointer for an incoming dirEdge
	 * is set to the next outgoing edge.
	 *
	 * DirEdges are only linked if:
	 *
	 * - they belong to an area (i.e. they have sides)
	 * - they are marked as being in the result
	 *
	 * Edges are linked in CCW order (the order they are stored). This means
	 * that rings have their face on the Right (in other words, the topological
	 * location of the face is given by the RHS label of the DirectedEdge)
	 *
	 * PRECONDITION: No pair of dirEdges are both marked as being in the result
	 */
	void linkResultDirectedEdges(); // throw(TopologyException *);

	int getOutgoingDegree();

	int getOutgoingDegree(EdgeRing *er);

	void linkMinimalDirectedEdges(EdgeRing *er);

	/** \brief
	 * Compute the DirectedEdge depths for a subsequence of the edge array.
	 */
	void computeDepths(DirectedEdge *de);

	DirectedEdge *getRightmostEdge();

private:
	/**
	 * A list of all outgoing edges in the result, in CCW order
	 */
	std::vector<DirectedEdge *> resultAreaEdgeList;

	Label label;

	bool resultAreaEdgesComputed;

	/// States for linResultDirectedEdges
	enum { SCANNING_FOR_INCOMING = 1, LINKING_TO_OUTGOING };

	/// \brief
	/// Returned vector is owned by DirectedEdgeStar object, but
	/// lazily created
	const std::vector<DirectedEdge *> &getResultAreaEdges();

	int computeDepths(EdgeEndStar::iterator startIt, EdgeEndStar::iterator endIt, int startDepth);
};

} // namespace geomgraph
} // namespace geos