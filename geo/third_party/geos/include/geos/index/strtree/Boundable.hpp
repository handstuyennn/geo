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
 **********************************************************************/

#ifndef GEOS_INDEX_STRTREE_BOUNDABLE_H
#define GEOS_INDEX_STRTREE_BOUNDABLE_H

#include <geos/export.hpp>

namespace geos {
namespace index {   // geos::index
namespace strtree { // geos::index::strtree

/// A spatial object in an AbstractSTRtree.
class GEOS_DLL Boundable {
public:
	virtual bool isLeaf() const = 0;
	virtual ~Boundable() {
	}
};

} // namespace strtree
} // namespace index
} // namespace geos

#endif // GEOS_INDEX_STRTREE_BOUNDABLE_H
