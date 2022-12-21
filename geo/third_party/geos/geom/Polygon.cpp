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
 * Last port: geom/Polygon.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <cassert>
#include <cmath> // for fabs
#include <geos/algorithm/Area.hpp>
#include <geos/geom/CoordinateSequenceFilter.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/GeometryFilter.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/util.hpp>
#include <memory>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

// using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

/*protected*/
Polygon::Polygon(const Polygon &p)
    : Geometry(p), shell(detail::make_unique<LinearRing>(*p.shell)), holes(p.holes.size()) {
	for (std::size_t i = 0; i < holes.size(); ++i) {
		holes[i] = detail::make_unique<LinearRing>(*p.holes[i]);
	}
}

/*protected*/
Polygon::Polygon(LinearRing *newShell, std::vector<LinearRing *> *newHoles, const GeometryFactory *newFactory)
    : Geometry(newFactory) {
	if (newShell == nullptr) {
		shell = getFactory()->createLinearRing();
	} else {
		if (newHoles != nullptr && newShell->isEmpty() && hasNonEmptyElements(newHoles)) {
			throw util::IllegalArgumentException("shell is empty but holes are not");
		}
		shell.reset(newShell);
	}

	if (newHoles != nullptr) {
		if (hasNullElements(newHoles)) {
			throw util::IllegalArgumentException("holes must not contain null elements");
		}
		for (const auto &hole : *newHoles) {
			holes.emplace_back(hole);
		}
		delete newHoles;
	}
}

Polygon::Polygon(std::unique_ptr<LinearRing> &&newShell, const GeometryFactory &newFactory)
    : Geometry(&newFactory), shell(std::move(newShell)) {
	if (shell == nullptr) {
		shell = getFactory()->createLinearRing();
	}
}

Polygon::Polygon(std::unique_ptr<LinearRing> &&newShell, std::vector<std::unique_ptr<LinearRing>> &&newHoles,
                 const GeometryFactory &newFactory)
    : Geometry(&newFactory), shell(std::move(newShell)), holes(std::move(newHoles)) {
	if (shell == nullptr) {
		shell = getFactory()->createLinearRing();
	}

	// TODO move into validateConstruction() method
	if (shell->isEmpty() && hasNonEmptyElements(&holes)) {
		throw util::IllegalArgumentException("shell is empty but holes are not");
	}
	if (hasNullElements(&holes)) {
		throw util::IllegalArgumentException("holes must not contain null elements");
	}
}

std::string Polygon::getGeometryType() const {
	return "Polygon";
}

GeometryTypeId Polygon::getGeometryTypeId() const {
	return GEOS_POLYGON;
}

uint8_t Polygon::getCoordinateDimension() const {
	uint8_t dimension = 2;

	if (shell != nullptr) {
		dimension = std::max(dimension, shell->getCoordinateDimension());
	}

	for (const auto &hole : holes) {
		dimension = std::max(dimension, hole->getCoordinateDimension());
	}

	return dimension;
}

size_t Polygon::getNumPoints() const {
	std::size_t numPoints = shell->getNumPoints();
	for (const auto &lr : holes) {
		numPoints += lr->getNumPoints();
	}
	return numPoints;
}

const LinearRing *Polygon::getExteriorRing() const {
	return shell.get();
}

const LinearRing *Polygon::getInteriorRingN(std::size_t n) const {
	return holes[n].get();
}

size_t Polygon::getNumInteriorRing() const {
	return holes.size();
}

bool Polygon::isEmpty() const {
	return shell->isEmpty();
}

Envelope::Ptr Polygon::computeEnvelopeInternal() const {
	return detail::make_unique<Envelope>(*(shell->getEnvelopeInternal()));
}

const CoordinateXY *Polygon::getCoordinate() const {
	return shell->getCoordinate();
}

Dimension::DimensionType Polygon::getDimension() const {
	return Dimension::A; // area
}

void Polygon::apply_ro(CoordinateFilter *filter) const {
	shell->apply_ro(filter);
	for (const auto &lr : holes) {
		lr->apply_ro(filter);
	}
}

void Polygon::apply_rw(const CoordinateFilter *filter) {
	shell->apply_rw(filter);
	for (auto &lr : holes) {
		lr->apply_rw(filter);
	}
}

void Polygon::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Polygon::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Polygon::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
	shell->apply_ro(filter);
	for (std::size_t i = 0, n = holes.size(); i < n && !filter->isDone(); ++i) {
		holes[i]->apply_ro(filter);
	}
}

void Polygon::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
	shell->apply_rw(filter);
	for (std::size_t i = 0, n = holes.size(); i < n && !filter->isDone(); ++i) {
		holes[i]->apply_rw(filter);
	}
}

void Polygon::apply_rw(CoordinateSequenceFilter &filter) {
	shell->apply_rw(filter);

	if (!filter.isDone()) {
		for (std::size_t i = 0, n = holes.size(); i < n; ++i) {
			holes[i]->apply_rw(filter);
			if (filter.isDone()) {
				break;
			}
		}
	}
	if (filter.isGeometryChanged()) {
		geometryChanged();
	}
}

void Polygon::apply_ro(CoordinateSequenceFilter &filter) const {
	shell->apply_ro(filter);

	if (!filter.isDone()) {
		for (std::size_t i = 0, n = holes.size(); i < n; ++i) {
			holes[i]->apply_ro(filter);
			if (filter.isDone()) {
				break;
			}
		}
	}
	// if (filter.isGeometryChanged()) geometryChanged();
}

/*
 *  Returns the area of this <code>Polygon</code>
 *
 * @return the area of the polygon
 */
double Polygon::getArea() const {
	double area = 0.0;
	area += algorithm::Area::ofRing(shell->getCoordinatesRO());
	for (const auto &lr : holes) {
		const CoordinateSequence *h = lr->getCoordinatesRO();
		area -= algorithm::Area::ofRing(h);
	}
	return area;
}

// Returns a newly allocated Geometry object
/*public*/
std::unique_ptr<Geometry> Polygon::getBoundary() const {
	/*
	 * We will make sure that what we
	 * return is composed of LineString,
	 * not LinearRings
	 */

	const GeometryFactory *gf = getFactory();

	if (isEmpty()) {
		return std::unique_ptr<Geometry>(gf->createMultiLineString());
	}

	if (holes.empty()) {
		return std::unique_ptr<Geometry>(gf->createLineString(*shell));
	}

	std::vector<std::unique_ptr<Geometry>> rings(holes.size() + 1);

	rings[0] = gf->createLineString(*shell);
	for (std::size_t i = 0, n = holes.size(); i < n; ++i) {
		const LinearRing *hole = holes[i].get();
		std::unique_ptr<LineString> ls = gf->createLineString(*hole);
		rings[i + 1] = std::move(ls);
	}

	return getFactory()->createMultiLineString(std::move(rings));
}

int Polygon::getBoundaryDimension() const {
	return 1;
}

std::unique_ptr<Geometry> Polygon::convexHull() const {
	return getExteriorRing()->convexHull();
}

/**
 * Returns the perimeter of this <code>Polygon</code>
 *
 * @return the perimeter of the polygon
 */
double Polygon::getLength() const {
	double len = 0.0;
	len += shell->getLength();
	for (const auto &hole : holes) {
		len += hole->getLength();
	}
	return len;
}

bool Polygon::isRectangle() const {
	if (getNumInteriorRing() != 0) {
		return false;
	}
	assert(shell != nullptr);
	if (shell->getNumPoints() != 5) {
		return false;
	}

	const CoordinateSequence &seq = *(shell->getCoordinatesRO());

	// check vertices have correct values
	const Envelope &env = *getEnvelopeInternal();
	for (uint32_t i = 0; i < 5; i++) {
		double x = seq.getX(i);
		if (!(x == env.getMinX() || x == env.getMaxX())) {
			return false;
		}
		double y = seq.getY(i);
		if (!(y == env.getMinY() || y == env.getMaxY())) {
			return false;
		}
	}

	// check vertices are in right order
	double prevX = seq.getX(0);
	double prevY = seq.getY(0);
	for (uint32_t i = 1; i <= 4; i++) {
		double x = seq.getX(i);
		double y = seq.getY(i);
		bool xChanged = (x != prevX);
		bool yChanged = (y != prevY);
		if (xChanged == yChanged) {
			return false;
		}
		prevX = x;
		prevY = y;
	}
	return true;
}

} // namespace geom
} // namespace geos
