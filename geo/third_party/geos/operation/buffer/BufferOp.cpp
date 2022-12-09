/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2007 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferOp.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm>
#include <cmath>
#include <geos/constants.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/PrecisionModel.hpp>
#include <geos/noding/ScaledNoder.hpp>
#include <geos/noding/snapround/MCIndexPointSnapper.hpp>
#include <geos/noding/snapround/MCIndexSnapRounder.hpp>
#include <geos/noding/snapround/SnapRoundingNoder.hpp>
#include <geos/operation/buffer/BufferBuilder.hpp>
#include <geos/operation/buffer/BufferOp.hpp>
#include <geos/precision/GeometryPrecisionReducer.hpp>

// FIXME: for temporary use, see other FIXME in file
#include <geos/algorithm/LineIntersector.hpp>
#include <geos/noding/IntersectionAdder.hpp>
#include <geos/noding/MCIndexNoder.hpp>

// #define PROFILE 1

using namespace geos::noding;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

/*private*/
double BufferOp::precisionScaleFactor(const Geometry *g, double distance, int maxPrecisionDigits) {
	const Envelope *env = g->getEnvelopeInternal();
	double envMax = std::max(std::max(fabs(env->getMaxX()), fabs(env->getMinX())),
	                         std::max(fabs(env->getMaxY()), fabs(env->getMinY())));

	double expandByDistance = distance > 0.0 ? distance : 0.0;
	double bufEnvMax = envMax + 2 * expandByDistance;

	// the smallest power of 10 greater than the buffer envelope
	int bufEnvPrecisionDigits = (int)(std::log(bufEnvMax) / std::log(10.0) + 1.0);
	int minUnitLog10 = maxPrecisionDigits - bufEnvPrecisionDigits;

	double scaleFactor = std::pow(10.0, minUnitLog10);

	return scaleFactor;
}

/*public static*/
std::unique_ptr<Geometry> BufferOp::bufferOp(const Geometry *g, double dist, int quadrantSegments, int nEndCapStyle) {
	BufferOp bufOp(g);
	bufOp.setQuadrantSegments(quadrantSegments);
	bufOp.setEndCapStyle(nEndCapStyle);
	return bufOp.getResultGeometry(dist);
}

/*public static*/
std::unique_ptr<geom::Geometry> BufferOp::bufferOp(const geom::Geometry *g, double dist, BufferParameters &bufParms) {
	BufferOp bufOp(g, bufParms);
	return bufOp.getResultGeometry(dist);
}

/*public*/
std::unique_ptr<Geometry> BufferOp::getResultGeometry(double nDistance) {
	distance = nDistance;
	computeGeometry();
	return std::unique_ptr<Geometry>(resultGeometry.release());
}

/*private*/
void BufferOp::computeGeometry() {
	bufferOriginalPrecision();

	if (resultGeometry != nullptr) {
		return;
	}

	const PrecisionModel &argPM = *(argGeom->getFactory()->getPrecisionModel());
	if (argPM.getType() == PrecisionModel::FIXED) {
		bufferFixedPrecision(argPM);
	} else {
		bufferReducedPrecision();
	}
}

/*private*/
void BufferOp::bufferOriginalPrecision() {
	BufferBuilder bufBuilder(bufParams);
	bufBuilder.setInvertOrientation(isInvertOrientation);

	try {
		resultGeometry = bufBuilder.buffer(argGeom, distance);
	} catch (const util::TopologyException &ex) {
		// don't propagate the exception - it will be detected by
		// fact that resultGeometry is null
		saveException = ex;
	}
}

/*private*/
void BufferOp::bufferFixedPrecision(const PrecisionModel &fixedPM) {
	PrecisionModel pm(1.0); // fixed as well

	// Reduce precision using SnapRoundingNoder
	//
	// This more closely aligns with JTS implementation,
	// and avoids reducing the precision of the input
	// geometry.
	//
	// TODO: Add a finer fallback sequence. Full
	//       precision, then SnappingNoder, then
	//       SnapRoundingNoder.

	snapround::SnapRoundingNoder inoder(&pm);
	ScaledNoder noder(inoder, fixedPM.getScale());
	BufferBuilder bufBuilder(bufParams);
	bufBuilder.setWorkingPrecisionModel(&fixedPM);
	bufBuilder.setNoder(&noder);
	bufBuilder.setInvertOrientation(isInvertOrientation);
	resultGeometry = bufBuilder.buffer(argGeom, distance);
}

/*private*/
void BufferOp::bufferReducedPrecision() {

	// try and compute with decreasing precision,
	// up to a min, to avoid gross results
	// (not in JTS, see http://trac.osgeo.org/geos/ticket/605)
#define MIN_PRECISION_DIGITS 6
	for (int precDigits = MAX_PRECISION_DIGITS; precDigits >= MIN_PRECISION_DIGITS; precDigits--) {
		try {
			bufferReducedPrecision(precDigits);
		} catch (const util::TopologyException &ex) {
			saveException = ex;
			// don't propagate the exception - it will be detected by fact that resultGeometry is null
		}

		if (resultGeometry != nullptr) {
			return;
		}
	}
	// tried everything - have to bail
	throw saveException;
}

void BufferOp::bufferReducedPrecision(int precisionDigits) {
	double sizeBasedScaleFactor = precisionScaleFactor(argGeom, distance, precisionDigits);

	assert(sizeBasedScaleFactor > 0);
	PrecisionModel fixedPM(sizeBasedScaleFactor);
	bufferFixedPrecision(fixedPM);
}

} // namespace buffer
} // namespace operation
} // namespace geos
