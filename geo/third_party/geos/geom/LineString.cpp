/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geom/LineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <cassert>
#include <geos/algorithm/Length.hpp>
#include <geos/geom/CoordinateArraySequence.hpp>
#include <geos/geom/CoordinateFilter.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/CoordinateSequenceFilter.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/GeometryFilter.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/operation/BoundaryOp.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <memory>
#include <typeinfo>

using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

LineString::~LineString() {
}

/*protected*/
LineString::LineString(const LineString &ls) : Geometry(ls), points(ls.points->clone()) {
	// points=ls.points->clone();
}

/*private*/
void LineString::validateConstruction() {
	if (points.get() == nullptr) {
		points = getFactory()->getCoordinateSequenceFactory()->create();
		return;
	}

	if (points->size() == 1) {
		throw util::IllegalArgumentException("point array must contain 0 or >1 elements\n");
	}
}

/*protected*/
LineString::LineString(CoordinateSequence *newCoords, const GeometryFactory *factory)
    : Geometry(factory), points(newCoords) {
	validateConstruction();
}

/*public*/
LineString::LineString(CoordinateSequence::Ptr &&newCoords, const GeometryFactory &factory)
    : Geometry(&factory), points(std::move(newCoords)) {
	validateConstruction();
}

/*public*/
LineString::LineString(std::vector<Coordinate> &&newCoords, const GeometryFactory &factory)
    : Geometry(&factory), points(new CoordinateArraySequence(std::move(newCoords))) {
	validateConstruction();
}

const CoordinateSequence *LineString::getCoordinatesRO() const {
	assert(nullptr != points.get());
	return points.get();
}

const Coordinate &LineString::getCoordinateN(std::size_t n) const {
	assert(points.get());
	return points->getAt(n);
}

std::string LineString::getGeometryType() const {
	return "LineString";
}

GeometryTypeId LineString::getGeometryTypeId() const {
	return GEOS_LINESTRING;
}

uint8_t LineString::getCoordinateDimension() const {
	return (uint8_t)points->getDimension();
}

bool LineString::isEmpty() const {
	assert(points.get());
	return points->isEmpty();
}

std::size_t LineString::getNumPoints() const {
	assert(points.get());
	return points->getSize();
}

bool LineString::isClosed() const {
	if (isEmpty()) {
		return false;
	}
	return getCoordinateN(0).equals2D(getCoordinateN(getNumPoints() - 1));
}

/*protected*/
Envelope::Ptr LineString::computeEnvelopeInternal() const {
	if (isEmpty()) {
		// We don't return NULL here
		// as it would indicate "unknown"
		// envelope. In this case we
		// *know* the envelope is EMPTY.
		return Envelope::Ptr(new Envelope());
	}

	return detail::make_unique<Envelope>(points->getEnvelope());
}

Dimension::DimensionType LineString::getDimension() const {
	return Dimension::L; // line
}

const CoordinateXY *LineString::getCoordinate() const {
	if (isEmpty()) {
		return nullptr;
	}
	return &(points->getAt(0));
}

void LineString::apply_ro(GeometryComponentFilter *filter) const {
	assert(filter);
	filter->filter_ro(this);
}

void LineString::apply_rw(const CoordinateFilter *filter) {
	assert(points.get());
	points->apply_rw(filter);
}

void LineString::apply_ro(CoordinateFilter *filter) const {
	assert(points.get());
	points->apply_ro(filter);
}

void LineString::apply_rw(GeometryFilter *filter) {
	assert(filter);
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryFilter *filter) const {
	assert(filter);
	filter->filter_ro(this);
}

void LineString::apply_rw(GeometryComponentFilter *filter) {
	assert(filter);
	filter->filter_rw(this);
}

void LineString::apply_rw(CoordinateSequenceFilter &filter) {
	std::size_t npts = points->size();
	if (!npts) {
		return;
	}
	for (std::size_t i = 0; i < npts; ++i) {
		filter.filter_rw(*points, i);
		if (filter.isDone()) {
			break;
		}
	}
	if (filter.isGeometryChanged()) {
		geometryChanged();
	}
}

void LineString::apply_ro(CoordinateSequenceFilter &filter) const {
	std::size_t npts = points->size();
	if (!npts) {
		return;
	}
	for (std::size_t i = 0; i < npts; ++i) {
		filter.filter_ro(*points, i);
		if (filter.isDone()) {
			break;
		}
	}
	// if (filter.isGeometryChanged()) geometryChanged();
}

bool LineString::isRing() const {
	return isClosed() && isSimple();
}

std::unique_ptr<Geometry> LineString::getBoundary() const {
	operation::BoundaryOp bop(*this);
	return bop.getBoundary();
}

int LineString::getBoundaryDimension() const {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

std::unique_ptr<Point> LineString::getPointN(std::size_t n) const {
	assert(getFactory());
	assert(points.get());
	return std::unique_ptr<Point>(getFactory()->createPoint(points->getAt(n)));
}

std::unique_ptr<Point> LineString::getStartPoint() const {
	if (isEmpty()) {
		return nullptr;
		// return new Point(NULL,NULL);
	}
	return getPointN(0);
}

std::unique_ptr<Point> LineString::getEndPoint() const {
	if (isEmpty()) {
		return nullptr;
		// return new Point(NULL,NULL);
	}
	return getPointN(getNumPoints() - 1);
}

double LineString::getLength() const {
	return Length::ofLine(points.get());
}

} // namespace geom
} // namespace geos
