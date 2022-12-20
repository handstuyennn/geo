/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Point.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequenceFilter.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/GeometryFilter.hpp>
#include <geos/geom/Point.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <memory>
#include <string>

namespace geos {
namespace geom { // geos::geom

const static FixedSizeCoordinateSequence<0> emptyCoords2d(2);
const static FixedSizeCoordinateSequence<0> emptyCoords3d(3);

/*protected*/
Point::Point(CoordinateSequence *newCoords, const GeometryFactory *factory)
    : Geometry(factory), empty2d(false), empty3d(false) {
	std::unique_ptr<CoordinateSequence> coords(newCoords);

	if (coords == nullptr) {
		empty2d = true;
		return;
	}

	if (coords->getSize() == 1) {
		coordinates.setAt(coords->getAt(0), 0);
	} else if (coords->getSize() > 1) {
		throw util::IllegalArgumentException("Point coordinate list must contain a single element");
	} else if (coords->getDimension() == 3) {
		empty3d = true;
	} else {
		empty2d = true;
	}
}

Point::Point(const Coordinate &c, const GeometryFactory *factory) : Geometry(factory), empty2d(false), empty3d(false) {
	coordinates.setAt(c, 0);
}

Point::Point(const CoordinateXY &c, const GeometryFactory *factory)
    : Geometry(factory), empty2d(false), empty3d(false) {
	coordinates.setAt(Coordinate(c), 0);
}

/*protected*/
Point::Point(const Point &p) : Geometry(p), coordinates(p.coordinates), empty2d(p.empty2d), empty3d(p.empty3d) {
}

uint8_t Point::getCoordinateDimension() const {
	return (uint8_t)getCoordinatesRO()->getDimension();
}

bool Point::isEmpty() const {
	if (empty2d || empty3d)
		return true;
	const Coordinate &c = coordinates.getAt(0);
	if (std::isnan(c.x) && std::isnan(c.y))
		return true;
	else
		return false;
}

std::string Point::getGeometryType() const {
	return "Point";
}

GeometryTypeId Point::getGeometryTypeId() const {
	return GEOS_POINT;
}

/*public*/
const CoordinateSequence *Point::getCoordinatesRO() const {
	if (empty2d) {
		return &emptyCoords2d;
	} else if (empty3d) {
		return &emptyCoords3d;
	}
	return &coordinates;
}

std::size_t Point::getNumPoints() const {
	return isEmpty() ? 0 : 1;
}

Envelope::Ptr Point::computeEnvelopeInternal() const {
	if (isEmpty()) {
		return Envelope::Ptr(new Envelope());
	}

	return Envelope::Ptr(new Envelope(getCoordinate()->x, getCoordinate()->x, getCoordinate()->y, getCoordinate()->y));
}

Dimension::DimensionType Point::getDimension() const {
	return Dimension::P; // point
}

void Point::apply_rw(const CoordinateFilter *filter) {
	if (isEmpty()) {
		return;
	}
	coordinates.apply_rw(filter);
}

void Point::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Point::apply_ro(CoordinateFilter *filter) const {
	if (isEmpty()) {
		return;
	}
	filter->filter_ro(getCoordinate());
}

void Point::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Point::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

void Point::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

void Point::apply_rw(CoordinateSequenceFilter &filter) {
	if (isEmpty()) {
		return;
	}
	filter.filter_rw(coordinates, 0);
	if (filter.isGeometryChanged()) {
		geometryChanged();
	}
}

void Point::apply_ro(CoordinateSequenceFilter &filter) const {
	if (isEmpty()) {
		return;
	}
	filter.filter_ro(coordinates, 0);
	// if (filter.isGeometryChanged()) geometryChanged();
}

bool Point::isSimple() const {
	return true;
}

std::unique_ptr<Geometry> Point::getBoundary() const {
	return getFactory()->createGeometryCollection();
}

int Point::getBoundaryDimension() const {
	return Dimension::False;
}

} // namespace geom
} // namespace geos
