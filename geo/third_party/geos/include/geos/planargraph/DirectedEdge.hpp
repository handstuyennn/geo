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

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>            // for composition
#include <geos/planargraph/GraphComponent.hpp> // for inheritance
#include <list>                                // for typedefs
#include <vector>                              // for typedefs

// Forward declarations
namespace geos {
namespace planargraph {
class Edge;
class Node;
} // namespace planargraph
} // namespace geos

namespace geos {
namespace planargraph { // geos.planargraph

/**
 * \brief Represents a directed edge in a PlanarGraph.
 *
 * A DirectedEdge may or may not have a reference to a parent Edge
 * (some applications of planar graphs may not require explicit Edge
 * objects to be created). Usually a client using a PlanarGraph
 * will subclass DirectedEdge to add its own application-specific
 * data and methods.
 */
class GEOS_DLL DirectedEdge : public GraphComponent {};

} // namespace planargraph
} // namespace geos
