/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/util/GeometryTransformer.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <cassert>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/MultiLineString.hpp>
#include <geos/geom/MultiPoint.hpp>
#include <geos/geom/MultiPolygon.hpp>
#include <geos/geom/Point.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geom/util/GeometryTransformer.hpp>
#include <geos/util.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <typeinfo>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/*public*/
GeometryTransformer::GeometryTransformer()
    : factory(nullptr), inputGeom(nullptr), pruneEmptyGeometry(true), preserveGeometryCollectionType(true),
      // preserveCollections(false),
      preserveType(false), skipTransformedInvalidInteriorRings(false) {
}

/*public*/
std::unique_ptr<Geometry> GeometryTransformer::transform(const Geometry *nInputGeom) {
	using geos::util::IllegalArgumentException;

	inputGeom = nInputGeom;
	factory = inputGeom->getFactory();

	if (const Point *p = dynamic_cast<const Point *>(inputGeom)) {
		return transformPoint(p, nullptr);
	}
	if (const MultiPoint *mp = dynamic_cast<const MultiPoint *>(inputGeom)) {
		return transformMultiPoint(mp, nullptr);
	}
	if (const LinearRing *lr = dynamic_cast<const LinearRing *>(inputGeom)) {
		return transformLinearRing(lr, nullptr);
	}
	if (const LineString *ls = dynamic_cast<const LineString *>(inputGeom)) {
		return transformLineString(ls, nullptr);
	}
	if (const MultiLineString *mls = dynamic_cast<const MultiLineString *>(inputGeom)) {
		return transformMultiLineString(mls, nullptr);
	}
	if (const Polygon *p = dynamic_cast<const Polygon *>(inputGeom)) {
		return transformPolygon(p, nullptr);
	}
	if (const MultiPolygon *mp = dynamic_cast<const MultiPolygon *>(inputGeom)) {
		return transformMultiPolygon(mp, nullptr);
	}
	if (const GeometryCollection *gc = dynamic_cast<const GeometryCollection *>(inputGeom)) {
		return transformGeometryCollection(gc, nullptr);
	}

	throw IllegalArgumentException("Unknown Geometry subtype.");
}

std::unique_ptr<CoordinateSequence>
GeometryTransformer::createCoordinateSequence(std::unique_ptr<std::vector<Coordinate>> coords) {
	return std::unique_ptr<CoordinateSequence>(factory->getCoordinateSequenceFactory()->create(coords.release()));
}

std::unique_ptr<CoordinateSequence> GeometryTransformer::transformCoordinates(const CoordinateSequence *coords,
                                                                              const Geometry *parent) {

	::geos::ignore_unused_variable_warning(parent);

	return std::unique_ptr<CoordinateSequence>(coords->clone());
}

Geometry::Ptr GeometryTransformer::transformPoint(const Point *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	CoordinateSequence::Ptr cs(transformCoordinates(geom->getCoordinatesRO(), geom));

	return Geometry::Ptr(factory->createPoint(cs.release()));
}

Geometry::Ptr GeometryTransformer::transformMultiPoint(const MultiPoint *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);
	std::vector<std::unique_ptr<Geometry>> transGeomList;

	for (std::size_t i = 0, n = geom->getNumGeometries(); i < n; i++) {
		const Point *p = geom->getGeometryN(i);
		assert(p);

		Geometry::Ptr transformGeom = transformPoint(p, geom);
		if (transformGeom.get() == nullptr) {
			continue;
		}
		if (transformGeom->isEmpty()) {
			continue;
		}

		transGeomList.push_back(std::move(transformGeom));
	}

	if (transGeomList.empty()) {
		return factory->createMultiPoint();
	}

	return factory->buildGeometry(std::move(transGeomList));
}

Geometry::Ptr GeometryTransformer::transformLinearRing(const LinearRing *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	CoordinateSequence::Ptr seq(transformCoordinates(geom->getCoordinatesRO(), geom));

	std::size_t seqSize = seq ? seq->size() : 0;

	// ensure a valid LinearRing
	if (seqSize > 0 && seqSize < 4 && !preserveType) {
		return factory->createLineString(std::move(seq));
	} else {
		return factory->createLinearRing(std::move(seq));
	}
}

Geometry::Ptr GeometryTransformer::transformLineString(const LineString *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	// should check for 1-point sequences and downgrade them to points
	return factory->createLineString(transformCoordinates(geom->getCoordinatesRO(), geom));
}

Geometry::Ptr GeometryTransformer::transformMultiLineString(const MultiLineString *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	std::vector<std::unique_ptr<Geometry>> transGeomList;

	for (std::size_t i = 0, n = geom->getNumGeometries(); i < n; i++) {
		const LineString *l = geom->getGeometryN(i);
		assert(l);

		Geometry::Ptr transformGeom = transformLineString(l, geom);
		if (transformGeom.get() == nullptr) {
			continue;
		}
		if (transformGeom->isEmpty()) {
			continue;
		}

		transGeomList.push_back(std::move(transformGeom));
	}

	if (transGeomList.empty()) {
		return factory->createMultiLineString();
	}

	return factory->buildGeometry(std::move(transGeomList));
}

Geometry::Ptr GeometryTransformer::transformPolygon(const Polygon *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	bool isAllValidLinearRings = true;

	const LinearRing *lr = geom->getExteriorRing();
	assert(lr);

	Geometry::Ptr shell = transformLinearRing(lr, geom);
	if (shell.get() == nullptr || !dynamic_cast<LinearRing *>(shell.get()) || shell->isEmpty()) {
		isAllValidLinearRings = false;
	}

	std::vector<std::unique_ptr<LinearRing>> holes;
	for (std::size_t i = 0, n = geom->getNumInteriorRing(); i < n; i++) {
		const LinearRing *p_lr = geom->getInteriorRingN(i);
		assert(p_lr);

		Geometry::Ptr hole(transformLinearRing(p_lr, geom));

		if (hole.get() == nullptr || hole->isEmpty()) {
			continue;
		}

		if (dynamic_cast<LinearRing *>(hole.get())) {
			holes.emplace_back(dynamic_cast<LinearRing *>(hole.release()));
		} else {
			if (skipTransformedInvalidInteriorRings) {
				continue;
			}
			isAllValidLinearRings = false;
		}
	}

	if (isAllValidLinearRings) {
		std::unique_ptr<LinearRing> shell_lr(dynamic_cast<LinearRing *>(shell.release()));
		return factory->createPolygon(std::move(shell_lr), std::move(holes));
	} else {
		std::vector<std::unique_ptr<Geometry>> components;
		if (shell.get() != nullptr) {
			components.push_back(std::move(shell));
		}

		for (auto &g : holes) {
			components.push_back(std::move(g));
		}

		return factory->buildGeometry(std::move(components));
	}
}

Geometry::Ptr GeometryTransformer::transformMultiPolygon(const MultiPolygon *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	std::vector<std::unique_ptr<Geometry>> transGeomList;

	for (std::size_t i = 0, n = geom->getNumGeometries(); i < n; i++) {
		const Polygon *p = geom->getGeometryN(i);
		assert(p);

		Geometry::Ptr transformGeom = transformPolygon(p, geom);
		if (transformGeom.get() == nullptr) {
			continue;
		}
		if (transformGeom->isEmpty()) {
			continue;
		}

		transGeomList.push_back(std::move(transformGeom));
	}

	if (transGeomList.empty()) {
		return factory->createMultiPolygon();
	}

	return factory->buildGeometry(std::move(transGeomList));
}

Geometry::Ptr GeometryTransformer::transformGeometryCollection(const GeometryCollection *geom, const Geometry *parent) {
	::geos::ignore_unused_variable_warning(parent);

	std::vector<std::unique_ptr<Geometry>> transGeomList;

	for (std::size_t i = 0, n = geom->getNumGeometries(); i < n; i++) {
		Geometry::Ptr transformGeom = transform(geom->getGeometryN(i)); // no parent ?
		if (transformGeom.get() == nullptr) {
			continue;
		}
		if (pruneEmptyGeometry && transformGeom->isEmpty()) {
			continue;
		}

		transGeomList.push_back(std::move(transformGeom));
	}

	if (preserveGeometryCollectionType) {
		return factory->createGeometryCollection(std::move(transGeomList));
	} else {
		return factory->buildGeometry(std::move(transGeomList));
	}
}

} // namespace util
} // namespace geom
} // namespace geos
