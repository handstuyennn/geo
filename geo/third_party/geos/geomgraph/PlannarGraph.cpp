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
 * Last port: geomgraph/PlanarGraph.java r428 (JTS-1.12+)
 *
 **********************************************************************/

#include <cassert>
#include <geos/algorithm/Orientation.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geom/Quadrant.hpp>
#include <geos/geomgraph/DirectedEdge.hpp>
#include <geos/geomgraph/DirectedEdgeStar.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/EdgeEndStar.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/geomgraph/NodeFactory.hpp>
#include <geos/geomgraph/NodeMap.hpp>
#include <geos/geomgraph/PlanarGraph.hpp>
#include <geos/util.hpp>
#include <sstream>
#include <string>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*protected*/
void PlanarGraph::insertEdge(Edge *e) {
	assert(e);
	assert(edges);
	edges->push_back(e);
}

} // namespace geomgraph
} // namespace geos