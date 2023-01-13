/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: index/strtree/STRtree.java rev. 1.11
 *
 **********************************************************************/

#ifndef GEOS_INDEX_STRTREE_STRTREE_H
#define GEOS_INDEX_STRTREE_STRTREE_H

#include <geos/export.hpp>
#include <geos/geom/Envelope.hpp>      // for inlines
#include <geos/index/SpatialIndex.hpp> // for inheritance
// #include <geos/index/strtree/AbstractSTRtree.h> // for inheritance
// #include <geos/index/strtree/BoundablePair.h>
// #include <geos/index/strtree/ItemDistance.h>
#include <vector>

namespace geos {
namespace index {   // geos::index
namespace strtree { // geos::index::strtree

/**
 * \brief
 * A query-only R-tree created using the Sort-Tile-Recursive (STR) algorithm.
 * For two-dimensional spatial data.
 *
 * The STR packed R-tree is simple to implement and maximizes space
 * utilization; that is, as many leaves as possible are filled to capacity.
 * Overlap between nodes is far less than in a basic R-tree. However, once the
 * tree has been built (explicitly or on the first call to #query), items may
 * not be added or removed.
 *
 * Described in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial
 * Databases With Application To GIS. Morgan Kaufmann, San Francisco, 2002.
 *
 */
class GEOS_DLL STRtree : public SpatialIndex {
	// using AbstractSTRtree::insert;
	// using AbstractSTRtree::query;
};

} // namespace strtree
} // namespace index
} // namespace geos

#endif // GEOS_INDEX_STRTREE_STRTREE_H
