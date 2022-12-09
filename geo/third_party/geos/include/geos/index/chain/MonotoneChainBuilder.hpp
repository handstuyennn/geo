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
 * Last port: index/chain/MonotoneChainBuilder.java r388 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <cstddef>
#include <geos/export.hpp>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
}
namespace index {
namespace chain {
class MonotoneChain;
}
} // namespace index
} // namespace geos

namespace geos {
namespace index { // geos::index
namespace chain { // geos::index::chain

/** \brief
 * Constructs [MonotoneChains](@ref index::chain::MonotoneChain)
 * for sequences of [Coordinates](@ref geom::Coordinate).
 *
 * TODO: use vector<const Coordinate*> instead ?
 */
class GEOS_DLL MonotoneChainBuilder {
public:
	/** \brief
	 * Computes a list of the {@link MonotoneChain}s for a list of coordinates,
	 * attaching a context data object to each.
	 *
	 * @param pts the list of points to compute chains for
	 * @param context a data object to attach to each chain
	 * @param[out] mcList a list of the monotone chains for the points
	 */
	static void getChains(const geom::CoordinateSequence *pts, void *context, std::vector<MonotoneChain> &mcList);
};

} // namespace chain
} // namespace index
} // namespace geos
