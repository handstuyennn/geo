/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geom/Geometry.java rev. 1.112
 *
 **********************************************************************/

#include <algorithm>
#include <cassert>
#include <geos/algorithm/Centroid.hpp>
#include <geos/algorithm/ConvexHull.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryComponentFilter.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/GeometryFilter.hpp>
#include <geos/geom/HeuristicOverlay.hpp>
#include <geos/geom/IntersectionMatrix.hpp>
#include <geos/operation/buffer/BufferOp.hpp>
#include <geos/operation/overlay/OverlayOp.hpp>
#include <geos/operation/overlayng/OverlayNGRobust.hpp>
#include <geos/operation/predicate/RectangleContains.hpp>
#include <geos/operation/predicate/RectangleIntersects.hpp>
#include <geos/operation/relate/RelateOp.hpp>
#include <geos/operation/union/UnaryUnionOp.hpp>
#include <geos/operation/valid/IsSimpleOp.hpp>
#include <geos/operation/valid/IsValidOp.hpp>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

#define SHORTCIRCUIT_PREDICATES 1

using namespace geos::algorithm;
using namespace geos::operation::overlay;
using namespace geos::operation::valid;
using namespace geos::operation::buffer;
using namespace geos::operation::relate;
using namespace geos::operation;

namespace geos {
namespace geom { // geos::geom

Geometry::GeometryChangedFilter Geometry::geometryChangedFilter;

Geometry::Geometry(const GeometryFactory *newFactory) : envelope(nullptr), _factory(newFactory), _userData(nullptr) {
	if (_factory == nullptr) {
		_factory = GeometryFactory::getDefaultInstance();
	}
	SRID = _factory->getSRID();
	_factory->addRef();
}

Geometry::Geometry(const Geometry &geom) : SRID(geom.getSRID()), _factory(geom._factory), _userData(nullptr) {
	if (geom.envelope.get()) {
		envelope.reset(new Envelope(*(geom.envelope)));
	}
	// factory=geom.factory;
	// envelope(new Envelope(*(geom.envelope.get())));
	// SRID=geom.getSRID();
	//_userData=NULL;
	_factory->addRef();
}

Geometry::~Geometry() {
	_factory->dropRef();
}

const Envelope *Geometry::getEnvelopeInternal() const {
	if (!envelope.get()) {
		envelope = computeEnvelopeInternal();
	}
	return envelope.get();
}

std::unique_ptr<Geometry> Geometry::Union(const Geometry *other) const {
	// handle empty geometry cases
	if (isEmpty() || other->isEmpty()) {
		if (isEmpty() && other->isEmpty()) {
			return OverlayOp::createEmptyResult(OverlayOp::opUNION, this, other, getFactory());
		}
		// special case: if one input is empty ==> other input
		if (isEmpty())
			return other->clone();
		if (other->isEmpty())
			return clone();
	}

#ifdef SHORTCIRCUIT_PREDICATES
	// if envelopes are disjoint return a MULTI geom or
	// a geometrycollection
	if (!getEnvelopeInternal()->intersects(other->getEnvelopeInternal())) {
		// cerr<<"SHORTCIRCUITED-UNION engaged"<<endl;
		const GeometryCollection *coll;

		std::size_t ngeomsThis = getNumGeometries();
		std::size_t ngeomsOther = other->getNumGeometries();

		// Allocated for ownership transfer
		std::vector<Geometry *> *v = new std::vector<Geometry *>();
		v->reserve(ngeomsThis + ngeomsOther);

		if (nullptr != (coll = dynamic_cast<const GeometryCollection *>(this))) {
			for (std::size_t i = 0; i < ngeomsThis; ++i) {
				v->push_back(coll->getGeometryN(i)->clone().release());
			}
		} else {
			v->push_back(this->clone().release());
		}

		if (nullptr != (coll = dynamic_cast<const GeometryCollection *>(other))) {
			for (std::size_t i = 0; i < ngeomsOther; ++i) {
				v->push_back(coll->getGeometryN(i)->clone().release());
			}
		} else {
			v->push_back(other->clone().release());
		}

		std::unique_ptr<Geometry> out(_factory->buildGeometry(v));
		return out;
	}
#endif

	return HeuristicOverlay(this, other, OverlayOp::opUNION);
}

/* public */
Geometry::Ptr Geometry::Union() const {
	using geos::operation::geounion::UnaryUnionOp;
#ifdef DISABLE_OVERLAYNG
	return UnaryUnionOp::Union(*this);
#else
	return operation::overlayng::OverlayNGRobust::Union(this);
#endif
}

bool Geometry::isValid() const {
	return IsValidOp(this).isValid();
}

void Geometry::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Geometry::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Geometry::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

void Geometry::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

/* public */
const PrecisionModel *Geometry::getPrecisionModel() const {
	return _factory->getPrecisionModel();
}

/**
 * Notifies this Geometry that its Coordinates have been changed by an external
 * party (using a CoordinateFilter, for example). The Geometry will flush
 * and/or update any information it has cached (such as its {@link Envelope} ).
 */
void Geometry::geometryChanged() {
	apply_rw(&geometryChangedFilter);
}

void Geometry::GeometryChangedFilter::filter_rw(Geometry *geom) {
	geom->geometryChangedAction();
}

/**
 *  Returns the area of this <code>Geometry</code>.
 *  Areal Geometries have a non-zero area.
 *  They override this function to compute the area.
 *  Others return 0.0
 *
 * @return the area of the Geometry
 */
double Geometry::getArea() const {
	return 0.0;
}

bool Geometry::isSimple() const {
	operation::valid::IsSimpleOp op(*this);
	return op.isSimple();
}

std::unique_ptr<Geometry> Geometry::buffer(double p_distance) const {
	return std::unique_ptr<Geometry>(BufferOp::bufferOp(this, p_distance));
}

std::unique_ptr<Geometry> Geometry::difference(const Geometry *other) const
// throw(IllegalArgumentException *)
{
	// special case: if A.isEmpty ==> empty; if B.isEmpty ==> A
	if (isEmpty()) {
		return OverlayOp::createEmptyResult(OverlayOp::opDIFFERENCE, this, other, getFactory());
	}
	if (other->isEmpty()) {
		return clone();
	}

	return HeuristicOverlay(this, other, OverlayOp::opDIFFERENCE);
}

std::unique_ptr<Geometry> Geometry::intersection(const Geometry *other) const {
	/*
	 * TODO: MD - add optimization for P-A case using Point-In-Polygon
	 */

	// special case: if one input is empty ==> empty
	if (isEmpty() || other->isEmpty()) {
		return OverlayOp::createEmptyResult(OverlayOp::opINTERSECTION, this, other, getFactory());
	}

	return HeuristicOverlay(this, other, OverlayOp::opINTERSECTION);
}

/*public*/
std::unique_ptr<Point> Geometry::getCentroid() const {
	Coordinate centPt;
	if (!getCentroid(centPt)) {
		return getFactory()->createPoint(getCoordinateDimension());
	}

	// We don't use createPointFromInternalCoord here
	// because ::getCentroid() takes care about rounding
	return std::unique_ptr<Point>(getFactory()->createPoint(centPt));
}

/*public*/
bool Geometry::getCentroid(Coordinate &ret) const {
	if (isEmpty()) {
		return false;
	}
	if (!Centroid::getCentroid(*this, ret)) {
		return false;
	}
	getPrecisionModel()->makePrecise(ret); // not in JTS
	return true;
}

std::unique_ptr<Geometry> Geometry::convexHull() const {
	return ConvexHull(this).getConvexHull();
}

bool Geometry::relate(const Geometry *g, const std::string &intersectionPattern) const {
	std::unique_ptr<IntersectionMatrix> im(relate(g));
	bool res = im->matches(intersectionPattern);
	return res;
}

std::unique_ptr<IntersectionMatrix> Geometry::relate(const Geometry *other) const {
	return RelateOp::relate(this, other);
}

std::unique_ptr<IntersectionMatrix> Geometry::relate(const Geometry &other) const {
	return relate(&other);
}

bool Geometry::equals(const Geometry *g) const {
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (!getEnvelopeInternal()->equals(g->getEnvelopeInternal())) {
		return false;
	}
#endif

	if (isEmpty()) {
		return g->isEmpty();
	} else if (g->isEmpty()) {
		return isEmpty();
	}

	std::unique_ptr<IntersectionMatrix> im(relate(g));
	bool res = im->isEquals(getDimension(), g->getDimension());
	return res;
}

bool Geometry::contains(const Geometry *g) const {
	// optimization - lower dimension cannot contain areas
	if (g->getDimension() == 2 && getDimension() < 2) {
		return false;
	}

	// optimization - P cannot contain a non-zero-length L
	// Note that a point can contain a zero-length lineal geometry,
	// since the line has no boundary due to Mod-2 Boundary Rule
	if (g->getDimension() == 1 && getDimension() < 1 && g->getLength() > 0.0) {
		return false;
	}

#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (!getEnvelopeInternal()->contains(g->getEnvelopeInternal())) {
		return false;
	}
#endif

	// optimization for rectangle arguments
	if (isRectangle()) {
		const Polygon *p = detail::down_cast<const Polygon *>(this);
		return predicate::RectangleContains::contains(*p, *g);
	}
	// Incorrect: contains is not commutative
	// if (g->isRectangle()) {
	//	return predicate::RectangleContains::contains((const Polygon&)*g, *this);
	//}

	std::unique_ptr<IntersectionMatrix> im(relate(g));
	bool res = im->isContains();
	return res;
}

bool Geometry::disjoint(const Geometry *g) const {
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (!getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
		return true;
	}
#endif
	std::unique_ptr<IntersectionMatrix> im(relate(g));
	bool res = im->isDisjoint();
	return res;
}

bool Geometry::touches(const Geometry *g) const {
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (!getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
		return false;
	}
#endif
	std::unique_ptr<IntersectionMatrix> im(relate(g));
	bool res = im->isTouches(getDimension(), g->getDimension());
	return res;
}

bool Geometry::intersects(const Geometry *g) const {
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (!getEnvelopeInternal()->intersects(g->getEnvelopeInternal())) {
		return false;
	}
#endif

	/*
	 * TODO: (MD) Add optimizations:
	 *
	 * - for P-A case:
	 * If P is in env(A), test for point-in-poly
	 *
	 * - for A-A case:
	 * If env(A1).overlaps(env(A2))
	 * test for overlaps via point-in-poly first (both ways)
	 * Possibly optimize selection of point to test by finding point of A1
	 * closest to centre of env(A2).
	 * (Is there a test where we shouldn't bother - e.g. if env A
	 * is much smaller than env B, maybe there's no point in testing
	 * pt(B) in env(A)?
	 */

	// optimization for rectangle arguments
	if (isRectangle()) {
		const Polygon *p = detail::down_cast<const Polygon *>(this);
		return predicate::RectangleIntersects::intersects(*p, *g);
	}
	if (g->isRectangle()) {
		const Polygon *p = detail::down_cast<const Polygon *>(g);
		return predicate::RectangleIntersects::intersects(*p, *this);
	}

	std::unique_ptr<IntersectionMatrix> im(relate(g));
	bool res = im->isIntersects();
	return res;
}

/**
 *  Returns the length of this <code>Geometry</code>.
 *  Linear geometries return their length.
 *  Areal geometries return their perimeter.
 *  They override this function to compute the area.
 *  Others return 0.0
 *
 * @return the length of the Geometry
 */
double Geometry::getLength() const {
	return 0.0;
}

} // namespace geom
} // namespace geos
