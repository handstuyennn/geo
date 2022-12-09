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
 * Last port: noding/snapround/MCIndexSnapRounder.java r486 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/LineIntersector.hpp> // for composition
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>                      // for use in vector
#include <geos/geom/PrecisionModel.hpp>                  // for inlines
#include <geos/noding/NodedSegmentString.hpp>            // for inlines
#include <geos/noding/Noder.hpp>                         // for inheritance
#include <geos/noding/snapround/MCIndexPointSnapper.hpp> // for inlines
#include <vector>

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
namespace noding {
class SegmentString;
class MCIndexNoder;
} // namespace noding
} // namespace geos

namespace geos {
namespace noding {    // geos::noding
namespace snapround { // geos::noding::snapround

/** \brief
 * Uses Snap Rounding to compute a rounded,
 * fully noded arrangement from a set of SegmentString
 *
 * Implements the Snap Rounding technique described in Hobby, Guibas & Marimont,
 * and Goodrich et al.
 *
 * Snap Rounding assumes that all vertices lie on a uniform grid
 * (hence the precision model of the input must be fixed precision,
 * and all the input vertices must be rounded to that precision).
 *
 * This implementation uses a monotone chains and a spatial index to
 * speed up the intersection tests.
 *
 * This implementation appears to be fully robust using an integer
 * precision model.
 *
 * It will function with non-integer precision models, but the
 * results are not 100% guaranteed to be correctly noded.
 */
class GEOS_DLL MCIndexSnapRounder : public Noder { // implements Noder
};

} // namespace snapround
} // namespace noding
} // namespace geos
