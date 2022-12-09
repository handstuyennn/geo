/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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
 * Last port: geomgraph/EdgeList.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/noding/OrientedCoordinateArray.hpp> // for map comparator
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations
namespace geos {
namespace index {
class SpatialIndex;
}
namespace geomgraph {
class Edge;
}
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

/** \brief
 * A EdgeList is a list of Edges.
 *
 * It supports locating edges
 * that are pointwise equals to a target edge.
 */
class GEOS_DLL EdgeList {
private:
	std::vector<Edge *> edges;

	/**
	 * An index of the edges, for fast lookup.
	 */
	typedef std::unordered_map<noding::OrientedCoordinateArray, Edge *, noding::OrientedCoordinateArray::HashCode>
	    EdgeMap;

	EdgeMap ocaMap;

public:
	std::vector<Edge *> &getEdges() {
		return edges;
	}

	/**
	 * Insert an edge unless it is already in the list
	 */
	void add(Edge *e);

	void clearList();

	Edge *findEqualEdge(const Edge *e) const;
};

} // namespace geomgraph
} // namespace geos