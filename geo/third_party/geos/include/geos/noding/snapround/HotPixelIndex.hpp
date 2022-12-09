/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <array>
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp> // for composition
#include <geos/geom/Envelope.hpp>   // for unique_ptr
#include <geos/geom/PrecisionModel.hpp>
#include <geos/index/kdtree/KdNodeVisitor.hpp>
#include <geos/index/kdtree/KdTree.hpp>
#include <geos/io/WKTWriter.hpp>
#include <geos/noding/snapround/HotPixel.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <map>
#include <memory>

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
namespace index {
class ItemVisitor;
}
namespace noding {
namespace snapround {
class HotPixel;
}
} // namespace noding
} // namespace geos

namespace geos {
namespace noding {    // geos::noding
namespace snapround { // geos::noding::snapround

class GEOS_DLL HotPixelIndex {

private:
	/* members */
	const geom::PrecisionModel *pm;
	double scaleFactor;
	std::unique_ptr<geos::index::kdtree::KdTree> index;
	std::deque<HotPixel> hotPixelQue;

	/* methods */
	geom::Coordinate round(const geom::Coordinate &c);
	HotPixel *find(const geom::Coordinate &pixelPt);

public:
	HotPixelIndex(const geom::PrecisionModel *p_pm);
	HotPixel *add(const geom::Coordinate &pt);
	void add(const geom::CoordinateSequence *pts);
	void add(const std::vector<geom::Coordinate> &pts);
	void addNodes(const geom::CoordinateSequence *pts);
	void addNodes(const std::vector<geom::Coordinate> &pts);

	/**
	 * Visits all the hot pixels which may intersect a segment (p0-p1).
	 * The visitor must determine whether each hot pixel actually intersects
	 * the segment.
	 */
	void query(const geom::Coordinate &p0, const geom::Coordinate &p1, index::kdtree::KdNodeVisitor &visitor);
};

} // namespace snapround
} // namespace noding
} // namespace geos
