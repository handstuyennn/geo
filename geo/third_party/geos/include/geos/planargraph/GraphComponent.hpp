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
 **********************************************************************
 *
 * Last port: planargraph/GraphComponent.java rev. 1.7 (JTS-1.7)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>

namespace geos {
namespace planargraph { // geos.planargraph

/**
 * \brief The base class for all graph component classes.
 *
 * Maintains flags of use in generic graph algorithms.
 * Provides two flags:
 *
 *  - <b>marked</b> - typically this is used to indicate a state that
 *    persists for the course of the graph's lifetime.  For instance,
 *    it can be used to indicate that a component has been logically
 *    deleted from the graph.
 *  - <b>visited</b> - this is used to indicate that a component has been
 *    processed or visited by an single graph algorithm.  For instance,
 *    a breadth-first traversal of the graph might use this to indicate
 *    that a node has already been traversed.
 *    The visited flag may be set and cleared many times during the
 *    lifetime of a graph.
 *
 */
class GEOS_DLL GraphComponent {};

} // namespace planargraph
} // namespace geos
