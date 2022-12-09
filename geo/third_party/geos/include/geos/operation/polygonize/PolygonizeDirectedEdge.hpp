/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/PolygonizeDirectedEdge.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/planargraph/DirectedEdge.hpp> // for inheritance

// Forward declarations
namespace geos {
namespace geom {
// class LineString;
}
namespace planargraph {
class Node;
}
namespace operation {
namespace polygonize {
class EdgeRing;
}
} // namespace operation
} // namespace geos

namespace geos {
namespace operation {  // geos::operation
namespace polygonize { // geos::operation::polygonize

/** \brief
 * A DirectedEdge of a PolygonizeGraph, which represents
 * an edge of a polygon formed by the graph.
 *
 * May be logically deleted from the graph by setting the
 * <code>marked</code> flag.
 */
class GEOS_DLL PolygonizeDirectedEdge : public planargraph::DirectedEdge {};
} // namespace polygonize
} // namespace operation
} // namespace geos