/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/union/PointGeometryUnion.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm> // for copy
#include <cassert>   // for assert
#include <geos/algorithm/PointLocator.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geom/MultiPoint.hpp>
#include <geos/geom/Point.hpp>
#include <geos/geom/util/GeometryCombiner.hpp>
#include <geos/operation/union/PointGeometryUnion.hpp>
#include <memory> // for unique_ptr

namespace geos {
namespace operation { // geos::operation
namespace geounion {  // geos::operation::geounion

/* public */
std::unique_ptr<geom::Geometry> PointGeometryUnion::Union() const {
	using namespace geom;
	using algorithm::PointLocator;
	using geom::util::GeometryCombiner;

	PointLocator locater;
	// use a set to eliminate duplicates, as required for union
	std::set<Coordinate> exteriorCoords;

	for (std::size_t i = 0, n = pointGeom.getNumGeometries(); i < n; ++i) {
		const Point *point = dynamic_cast<const Point *>(pointGeom.getGeometryN(i));
		assert(point);
		const Coordinate *coord = point->getCoordinate();
		Location loc = locater.locate(*coord, &otherGeom);
		if (loc == Location::EXTERIOR) {
			exteriorCoords.insert(*coord);
		}
	}

	// if no points are in exterior, return the other geom
	if (exteriorCoords.empty()) {
		return otherGeom.clone();
	}

	// make a puntal geometry of appropriate size
	std::unique_ptr<Geometry> ptComp;

	if (exteriorCoords.size() == 1) {
		ptComp.reset(geomFact->createPoint(*(exteriorCoords.begin())));
	} else {
		std::vector<Coordinate> coords(exteriorCoords.size());
		std::copy(exteriorCoords.begin(), exteriorCoords.end(), coords.begin());
		ptComp.reset(geomFact->createMultiPoint(coords));
	}

	// add point component to the other geometry
	return std::unique_ptr<Geometry>(GeometryCombiner::combine(ptComp.get(), &otherGeom));
}

/* public  static */
std::unique_ptr<geom::Geometry> PointGeometryUnion::Union(const geom::Geometry &pointGeom,
                                                          const geom::Geometry &otherGeom) {
	PointGeometryUnion unioner(pointGeom, otherGeom);
	return unioner.Union();
}

/* public */
PointGeometryUnion::PointGeometryUnion(const geom::Geometry &pointGeom_, const geom::Geometry &otherGeom_)
    : pointGeom(pointGeom_), otherGeom(otherGeom_) {
	geomFact = otherGeom.getFactory();
}

} // namespace geounion
} // namespace operation
} // namespace geos
