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
 **********************************************************************/

#include <geos/precision/CommonBits.hpp>
#include <geos/precision/CommonBitsRemover.hpp>
// for CommonCoordinateFilter inheritance
#include <cassert>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateFilter.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/util.hpp>

using namespace geos::geom;

namespace geos {
namespace precision { // geos.precision

class Translater : public geom::CoordinateFilter {

private:
	geom::Coordinate trans;

public:
	Translater(geom::Coordinate &newTrans) : trans(newTrans) {
	}

	void filter_ro(const geom::Coordinate *coord) override // Not used
	{
		::geos::ignore_unused_variable_warning(coord);
		assert(0);
	}

	void filter_rw(geom::Coordinate *coord) const override {
		coord->x += trans.x;
		coord->y += trans.y;
	}
};

class CommonCoordinateFilter : public geom::CoordinateFilter {
private:
	CommonBits commonBitsX;
	CommonBits commonBitsY;

public:
	void filter_rw(geom::Coordinate *coord) const override {
		// CommonCoordinateFilter is a read-only filter
		::geos::ignore_unused_variable_warning(coord);
		assert(0);
	}

	void filter_ro(const geom::Coordinate *coord) override {
		commonBitsX.add(coord->x);
		commonBitsY.add(coord->y);
	}

	void getCommonCoordinate(geom::Coordinate &c) {
		c = Coordinate(commonBitsX.getCommon(), commonBitsY.getCommon());
	}
};

/**
 * Removes the common coordinate bits from a Geometry.
 * The coordinates of the Geometry are changed.
 *
 * @param geom the Geometry from which to remove the common coordinate bits
 * @return the shifted Geometry
 */
void CommonBitsRemover::removeCommonBits(Geometry *geom) {
	if (commonCoord.x == 0.0 && commonCoord.y == 0.0) {
		return;
	}

	Coordinate invCoord(commonCoord);
	invCoord.x = -invCoord.x;
	invCoord.y = -invCoord.y;

	Translater trans(invCoord);
	geom->apply_rw(&trans);
	geom->geometryChanged();
}

/**
 * Adds the common coordinate bits back into a Geometry.
 * The coordinates of the Geometry are changed.
 *
 * @param geom the Geometry to which to add the common coordinate bits
 * @return the shifted Geometry
 */
Geometry *CommonBitsRemover::addCommonBits(Geometry *geom) {
	Translater trans(commonCoord);

	geom->apply_rw(&trans);
	geom->geometryChanged();

	return geom;
}

/**
 * Add a geometry to the set of geometries whose common bits are
 * being computed.  After this method has executed the
 * common coordinate reflects the common bits of all added
 * geometries.
 *
 * @param geom a Geometry to test for common bits
 */
void CommonBitsRemover::add(const Geometry *geom) {
	geom->apply_ro(ccFilter);
	ccFilter->getCommonCoordinate(commonCoord);
}

CommonBitsRemover::CommonBitsRemover() {
	ccFilter = new CommonCoordinateFilter();
}

CommonBitsRemover::~CommonBitsRemover() {
	delete ccFilter;
}

} // namespace precision
} // namespace geos
