/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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
 * Last port: geom/GeometryCollection.java rev. 1.41
 *
 **********************************************************************/

#include <algorithm>
#include <geos/geom/CoordinateSequenceFilter.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/GeometryFilter.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <memory>
#include <vector>

namespace geos {
namespace geom { // geos::geom

/*protected*/
GeometryCollection::GeometryCollection(const GeometryCollection &gc) : Geometry(gc), geometries(gc.geometries.size()) {
	for (std::size_t i = 0; i < geometries.size(); ++i) {
		geometries[i] = gc.geometries[i]->clone();
	}
}

/*protected*/
GeometryCollection::GeometryCollection(std::vector<Geometry *> *newGeoms, const GeometryFactory *factory)
    : Geometry(factory) {
	if (newGeoms == nullptr) {
		return;
	}
	if (hasNullElements(newGeoms)) {
		throw util::IllegalArgumentException("geometries must not contain null elements\n");
	}
	for (const auto &geom : *newGeoms) {
		geometries.emplace_back(geom);
	}
	delete newGeoms;

	// Set SRID for inner geoms
	setSRID(getSRID());
}

GeometryCollection::GeometryCollection(std::vector<std::unique_ptr<Geometry>> &&newGeoms,
                                       const GeometryFactory &factory)
    : Geometry(&factory), geometries(std::move(newGeoms)) {

	if (hasNullElements(&geometries)) {
		throw util::IllegalArgumentException("geometries must not contain null elements\n");
	}

	setSRID(getSRID());
}

std::string GeometryCollection::getGeometryType() const {
	return "GeometryCollection";
}

GeometryTypeId GeometryCollection::getGeometryTypeId() const {
	return GEOS_GEOMETRYCOLLECTION;
}

bool GeometryCollection::isEmpty() const {
	for (const auto &g : geometries) {
		if (!g->isEmpty()) {
			return false;
		}
	}
	return true;
}

uint8_t GeometryCollection::getCoordinateDimension() const {
	uint8_t dimension = 2;

	for (const auto &g : geometries) {
		dimension = std::max(dimension, g->getCoordinateDimension());
	}
	return dimension;
}

size_t GeometryCollection::getNumGeometries() const {
	return geometries.size();
}

const Geometry *GeometryCollection::getGeometryN(std::size_t n) const {
	return geometries[n].get();
}

size_t GeometryCollection::getNumPoints() const {
	std::size_t numPoints = 0;
	for (const auto &g : geometries) {
		numPoints += g->getNumPoints();
	}
	return numPoints;
}

Envelope::Ptr GeometryCollection::computeEnvelopeInternal() const {
	Envelope::Ptr p_envelope(new Envelope());
	for (const auto &g : geometries) {
		const Envelope *env = g->getEnvelopeInternal();
		p_envelope->expandToInclude(env);
	}
	return p_envelope;
}

const CoordinateXY *GeometryCollection::getCoordinate() const {
	for (const auto &g : geometries) {
		if (!g->isEmpty()) {
			return g->getCoordinate();
		}
	}
	return nullptr;
}

Dimension::DimensionType GeometryCollection::getDimension() const {
	Dimension::DimensionType dimension = Dimension::False;
	for (const auto &g : geometries) {
		dimension = std::max(dimension, g->getDimension());
	}
	return dimension;
}

int GeometryCollection::getBoundaryDimension() const {
	int dimension = Dimension::False;
	for (const auto &g : geometries) {
		dimension = std::max(dimension, g->getBoundaryDimension());
	}
	return dimension;
}

bool GeometryCollection::isDimensionStrict(Dimension::DimensionType d) const {
	return std::all_of(geometries.begin(), geometries.end(),
	                   [&d](const std::unique_ptr<Geometry> &g) { return g->getDimension() == d; });
}

void GeometryCollection::apply_rw(const CoordinateFilter *filter) {
	for (auto &g : geometries) {
		g->apply_rw(filter);
	}
}

void GeometryCollection::apply_ro(CoordinateFilter *filter) const {
	for (const auto &g : geometries) {
		g->apply_ro(filter);
	}
}

void GeometryCollection::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
	for (const auto &g : geometries) {
		g->apply_ro(filter);
	}
}

void GeometryCollection::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
	for (auto &g : geometries) {
		g->apply_rw(filter);
	}
}

void GeometryCollection::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
	for (auto &g : geometries) {
		if (filter->isDone()) {
			return;
		}
		g->apply_rw(filter);
	}
}

void GeometryCollection::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
	for (const auto &g : geometries) {
		if (filter->isDone()) {
			return;
		}
		g->apply_ro(filter);
	}
}

void GeometryCollection::apply_rw(CoordinateSequenceFilter &filter) {
	for (auto &g : geometries) {
		g->apply_rw(filter);
		if (filter.isDone()) {
			break;
		}
	}
	if (filter.isGeometryChanged()) {
		geometryChanged();
	}
}

void GeometryCollection::apply_ro(CoordinateSequenceFilter &filter) const {
	for (const auto &g : geometries) {
		g->apply_ro(filter);
		if (filter.isDone()) {
			break;
		}
	}

	assert(!filter.isGeometryChanged()); // read-only filter...
	                                     // if (filter.isGeometryChanged()) geometryChanged();
}

std::vector<std::unique_ptr<Geometry>> GeometryCollection::releaseGeometries() {
	auto ret = std::move(geometries);
	geometryChanged();
	return ret;
}

/**
 * @return the area of this collection
 */
double GeometryCollection::getArea() const {
	double area = 0.0;
	for (const auto &g : geometries) {
		area += g->getArea();
	}
	return area;
}

void GeometryCollection::setSRID(int newSRID) {
	Geometry::setSRID(newSRID);
	for (auto &g : geometries) {
		g->setSRID(newSRID);
	}
}

std::unique_ptr<Geometry> GeometryCollection::getBoundary() const {
	throw util::IllegalArgumentException("Operation not supported by GeometryCollection\n");
}

/**
 * @return the total length of this collection
 */
double GeometryCollection::getLength() const {
	double sum = 0.0;
	for (const auto &g : geometries) {
		sum += g->getLength();
	}
	return sum;
}

} // namespace geom
} // namespace geos
