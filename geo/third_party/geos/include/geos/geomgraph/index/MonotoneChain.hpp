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
 * Last port: geomgraph/index/MonotoneChain.java rev. 1.3 (JTS-1.7)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geomgraph/index/MonotoneChain.hpp>
#include <geos/geomgraph/index/MonotoneChainEdge.hpp> // for inline
#include <geos/geomgraph/index/SweepLineEventObj.hpp> // for inheritance

// Forward declarations
namespace geos {
namespace geomgraph {
namespace index {
class SegmentIntersector;
}
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos::geomgraph
namespace index {     // geos::geomgraph::index

/**
 * A chain in a MonotoneChainEdge
 */
class GEOS_DLL MonotoneChain : public SweepLineEventOBJ {
private:
	MonotoneChainEdge *mce;
	std::size_t chainIndex;

public:
	MonotoneChain(MonotoneChainEdge *newMce, std::size_t newChainIndex) : mce(newMce), chainIndex(newChainIndex) {
	}

	~MonotoneChain() override {
	}

	void computeIntersections(MonotoneChain *mc, SegmentIntersector *si) {
		mce->computeIntersectsForChain(chainIndex, *(mc->mce), mc->chainIndex, *si);
	}
};
} // namespace index
} // namespace geomgraph
} // namespace geos
