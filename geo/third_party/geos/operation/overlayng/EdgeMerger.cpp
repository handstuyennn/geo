/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Dimension.hpp>
#include <geos/operation/overlayng/EdgeMerger.hpp>
#include <geos/util/Assert.hpp>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/*public static */
std::vector<Edge *> EdgeMerger::merge(std::vector<Edge *> &edges) {
	std::vector<Edge *> mergedEdges;
	std::map<EdgeKey, Edge *> edgeMap;

	for (Edge *edge : edges) {
		EdgeKey edgeKey(edge);
		auto it = edgeMap.find(edgeKey);
		if (it == edgeMap.end()) {
			// this is the first (and maybe only) edge for this line
			edgeMap[edgeKey] = edge;
			mergedEdges.push_back(edge);
			// Debug.println("edge added: " + edge);
			// Debug.println(edge.toLineString());
		} else {
			// found an existing edge
			Edge *baseEdge = it->second;
			// Assert: edges are identical (up to direction)
			// this is a fast (but incomplete) sanity check
			//
			// NOTE: we throw an exception to avoid crashing processes
			// See https://trac.osgeo.org/geos/ticket/1051#comment:29
			//
			util::Assert::isTrue(baseEdge->size() == edge->size(),
			                     "Merge of edges of different sizes - probable noding error.");

			baseEdge->merge(edge);
			// Debug.println("edge merged: " + existing);
			// Debug.println(edge.toLineString());
		}
	}
	return mergedEdges;
}

} // namespace overlayng
} // namespace operation
} // namespace geos
