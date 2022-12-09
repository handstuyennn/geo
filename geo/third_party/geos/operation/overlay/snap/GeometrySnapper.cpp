/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2010  Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/GeometrySnapper.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/MultiPolygon.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geom/PrecisionModel.hpp>
#include <geos/geom/util/GeometryTransformer.hpp> // inherit. of SnapTransformer
#include <geos/operation/overlay/snap/GeometrySnapper.hpp>
#include <geos/operation/overlay/snap/LineStringSnapper.hpp>
#include <geos/util.hpp>
#include <geos/util/UniqueCoordinateArrayFilter.hpp>
#include <memory>
#include <vector>

//
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay {   // geos.operation.overlay
namespace snap {      // geos.operation.overlay.snap

const double GeometrySnapper::snapPrecisionFactor = 1e-9;

class SnapTransformer : public geos::geom::util::GeometryTransformer {

private:
	double snapTol;

	const Coordinate::ConstVect &snapPts;

	CoordinateSequence::Ptr snapLine(const CoordinateSequence *srcPts) {
		using std::unique_ptr;

		assert(srcPts);
		std::vector<Coordinate> coords;
		srcPts->toVector(coords);
		LineStringSnapper snapper(coords, snapTol);
		std::unique_ptr<Coordinate::Vect> newPts = snapper.snapTo(snapPts);

		const CoordinateSequenceFactory *cfact = factory->getCoordinateSequenceFactory();
		return std::unique_ptr<CoordinateSequence>(cfact->create(newPts.release()));
	}

public:
	SnapTransformer(double nSnapTol, const Coordinate::ConstVect &nSnapPts) : snapTol(nSnapTol), snapPts(nSnapPts) {
	}

	CoordinateSequence::Ptr transformCoordinates(const CoordinateSequence *coords, const Geometry *parent) override {
		::geos::ignore_unused_variable_warning(parent);
		return snapLine(coords);
	}
};

/*public static*/
double GeometrySnapper::computeSizeBasedSnapTolerance(const geom::Geometry &g) {
	const Envelope *env = g.getEnvelopeInternal();
	double minDimension = std::min(env->getHeight(), env->getWidth());
	double snapTol = minDimension * snapPrecisionFactor;
	return snapTol;
}

/*public static*/
double GeometrySnapper::computeOverlaySnapTolerance(const geom::Geometry &g) {
	double snapTolerance = computeSizeBasedSnapTolerance(g);

	/*
	 * Overlay is carried out in the precision model
	 * of the two inputs.
	 * If this precision model is of type FIXED, then the snap tolerance
	 * must reflect the precision grid size.
	 * Specifically, the snap tolerance should be at least
	 * the distance from a corner of a precision grid cell
	 * to the centre point of the cell.
	 */
	assert(g.getPrecisionModel());
	const PrecisionModel &pm = *(g.getPrecisionModel());
	if (pm.getType() == PrecisionModel::FIXED) {
		double fixedSnapTol = (1 / pm.getScale()) * 2 / 1.415;
		if (fixedSnapTol > snapTolerance) {
			snapTolerance = fixedSnapTol;
		}
	}
	return snapTolerance;
}

/*public static*/
double GeometrySnapper::computeOverlaySnapTolerance(const geom::Geometry &g1, const geom::Geometry &g2) {
	return std::min(computeOverlaySnapTolerance(g1), computeOverlaySnapTolerance(g2));
}

/*public*/
std::unique_ptr<geom::Geometry> GeometrySnapper::snapTo(const geom::Geometry &g, double snapTolerance) {

	using geom::util::GeometryTransformer;
	using std::unique_ptr;

	// Get snap points
	std::unique_ptr<Coordinate::ConstVect> snapPts = extractTargetCoordinates(g);

	// Apply a SnapTransformer to source geometry
	// (we need a pointer for dynamic polymorphism)
	std::unique_ptr<GeometryTransformer> snapTrans(new SnapTransformer(snapTolerance, *snapPts));
	return snapTrans->transform(&srcGeom);
}

/*private*/
std::unique_ptr<Coordinate::ConstVect> GeometrySnapper::extractTargetCoordinates(const Geometry &g) {
	auto snapPts = detail::make_unique<Coordinate::ConstVect>();
	util::UniqueCoordinateArrayFilter filter(*snapPts);
	g.apply_ro(&filter);
	// integrity check
	assert(snapPts->size() <= g.getNumPoints());
	return snapPts;
}

} // namespace snap
} // namespace overlay
} // namespace operation
} // namespace geos
