/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <cstdint>
#include <geos/export.hpp>
#include <geos/planargraph/GraphComponent.hpp> // for inheritance
#include <iosfwd>                              // ostream
#include <set>                                 // for typedefs
#include <vector>                              // for typedefs

// Forward declarations
namespace geos {
namespace planargraph {
class DirectedEdgeStar;
class DirectedEdge;
class Edge;
class Node;
} // namespace planargraph
} // namespace geos

namespace geos {
namespace planargraph { // geos.planargraph

/**
 * \brief Represents an undirected edge of a PlanarGraph.
 *
 * An undirected edge in fact simply acts as a central point of reference
 * for two opposite DirectedEdge.
 *
 * Usually a client using a PlanarGraph will subclass Edge
 * to add its own application-specific data and methods.
 */
class GEOS_DLL Edge : public GraphComponent {};

/// For backward compatibility
// typedef Edge planarEdge;

} // namespace planargraph
} // namespace geos