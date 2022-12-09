/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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

#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/EdgeList.hpp>
#include <geos/noding/OrientedCoordinateArray.hpp>
#include <sstream>
#include <string>
#include <vector>

using namespace geos::noding;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public*/
void EdgeList::add(Edge *e) {
	edges.push_back(e);
	OrientedCoordinateArray oca(*e->getCoordinates());
	ocaMap[oca] = e;
}

void EdgeList::clearList() {
	for (auto *edge : edges) {
		delete edge;
	}

	edges.clear();
}

/**
 * If there is an edge equal to e already in the list, return it.
 * Otherwise return null.
 * @return  equal edge, if there is one already in the list
 *          null otherwise
 */
Edge *EdgeList::findEqualEdge(const Edge *e) const {
	OrientedCoordinateArray oca(*(e->getCoordinates()));
	auto it = ocaMap.find(oca);

	if (it != ocaMap.end()) {
		return it->second;
	}
	return nullptr;
}

} // namespace geomgraph
} // namespace geos
