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
 * Last port: operation/valid/RepeatedPointTester.java rev. 1.8 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/MultiLineString.hpp>
#include <geos/geom/MultiPoint.hpp>
#include <geos/geom/MultiPolygon.hpp>
#include <geos/geom/Point.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/operation/valid/RepeatedPointTester.hpp>
#include <geos/util/UnsupportedOperationException.hpp>
#include <typeinfo>

using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

CoordinateXY &RepeatedPointTester::getCoordinate() {
	return repeatedCoord;
}

bool RepeatedPointTester::hasRepeatedPoint(const Geometry *g) {
	if (g->isEmpty()) {
		return false;
	}

	if (dynamic_cast<const Point *>(g)) {
		return false;
	}
	if (dynamic_cast<const MultiPoint *>(g)) {
		return false;
	}

	// LineString also handles LinearRings
	if (const LineString *x = dynamic_cast<const LineString *>(g)) {
		return hasRepeatedPoint(x->getCoordinatesRO());
	}

	if (const Polygon *x = dynamic_cast<const Polygon *>(g)) {
		return hasRepeatedPoint(x);
	}

	if (const MultiPolygon *x = dynamic_cast<const MultiPolygon *>(g)) {
		return hasRepeatedPoint(x);
	}

	if (const MultiLineString *x = dynamic_cast<const MultiLineString *>(g)) {
		return hasRepeatedPoint(x);
	}

	if (const GeometryCollection *x = dynamic_cast<const GeometryCollection *>(g)) {
		return hasRepeatedPoint(x);
	}

	throw util::UnsupportedOperationException(typeid(*g).name());
}

bool RepeatedPointTester::hasRepeatedPoint(const CoordinateSequence *coord) {
	auto npts = coord->getSize();
	for (std::size_t i = 1; i < npts; ++i) {
		if (coord->getAt(i - 1) == coord->getAt(i)) {
			repeatedCoord = coord->getAt(i);
			return true;
		}
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const Polygon *p) {
	if (hasRepeatedPoint(p->getExteriorRing()->getCoordinatesRO())) {
		return true;
	}

	for (std::size_t i = 0, n = p->getNumInteriorRing(); i < n; ++i) {
		if (hasRepeatedPoint(p->getInteriorRingN(i)->getCoordinatesRO())) {
			return true;
		}
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const GeometryCollection *gc) {
	for (std::size_t i = 0, n = gc->getNumGeometries(); i < n; ++i) {
		const Geometry *g = gc->getGeometryN(i);
		if (hasRepeatedPoint(g)) {
			return true;
		}
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const MultiPolygon *gc) {
	for (std::size_t i = 0, n = gc->getNumGeometries(); i < n; ++i) {
		const Polygon *g = gc->getGeometryN(i);
		if (hasRepeatedPoint(g)) {
			return true;
		}
	}
	return false;
}

bool RepeatedPointTester::hasRepeatedPoint(const MultiLineString *gc) {
	for (std::size_t i = 0, n = gc->getNumGeometries(); i < n; ++i) {
		const LineString *g = gc->getGeometryN(i);
		if (hasRepeatedPoint(g)) {
			return true;
		}
	}
	return false;
}

} // namespace valid
} // namespace operation
} // namespace geos
