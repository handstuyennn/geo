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
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/noding/SegmentString.hpp> // for inheritance

namespace geos {
namespace geom {
class Coordinate;
}
} // namespace geos

namespace geos {
namespace noding { // geos::noding

/** \brief
 * An interface for classes which support adding nodes to
 * a segment string.
 *
 * @author Martin Davis
 */
class GEOS_DLL NodableSegmentString : public SegmentString {
public:
	NodableSegmentString(const void *newContext) : SegmentString(newContext) {
	}
};

} // namespace noding
} // namespace geos
