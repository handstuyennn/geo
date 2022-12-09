/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/MCIndexPointSnapper.java r486 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <cstddef>
#include <geos/export.hpp>

// Forward declarations
namespace geos {
namespace geom {
class Envelope;
}
namespace index {
class SpatialIndex;
}
namespace noding {
class SegmentString;
namespace snapround {
class HotPixel;
}
} // namespace noding
} // namespace geos

namespace geos {
namespace noding {    // geos::noding
namespace snapround { // geos::noding::snapround

/** \brief
 * "Snaps" all [SegmentStrings](@ref SegmentString) in a [SpatialIndex](@ref index::SpatialIndex) containing
 * [MonotoneChains](@ref index::chain::MonotoneChain) to a given HotPixel.
 *
 */
class GEOS_DLL MCIndexPointSnapper {};

} // namespace snapround
} // namespace noding
} // namespace geos
