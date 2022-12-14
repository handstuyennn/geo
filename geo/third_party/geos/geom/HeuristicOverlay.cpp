/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2013-2020 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************
 *
 * This file provides a single templated function, taking two
 * const Geometry pointers, applying a binary operator to them
 * and returning a result Geometry in an unique_ptr<>.
 *
 * The binary operator is expected to take two const Geometry pointers
 * and return a newly allocated Geometry pointer, possibly throwing
 * a TopologyException to signal it couldn't succeed due to robustness
 * issues.
 *
 * This function will catch TopologyExceptions and try again with
 * slightly modified versions of the input. The following heuristic
 * is used:
 *
 *  - Try with original input.
 *  - Try removing common bits from input coordinate values
 *  - Try snaping input geometries to each other
 *  - Try snaping input coordinates to a increasing grid (size from 1/25 to 1)
 *  - Try simplifiying input with increasing tolerance (from 0.01 to 0.04)
 *
 * If none of the step succeeds the original exception is thrown.
 *
 * Note that you can skip Grid snapping, Geometry snapping and Simplify policies
 * by a compile-time define when building geos.
 * See USE_TP_SIMPLIFY_POLICY, USE_PRECISION_REDUCTION_POLICY and
 * USE_SNAPPING_POLICY macros below.
 *
 **********************************************************************/

#include <geos/algorithm/BoundaryNodeRule.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/HeuristicOverlay.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geom/PrecisionModel.hpp>
#include <geos/operation/overlay/OverlayOp.hpp>
#include <geos/operation/overlay/snap/GeometrySnapper.hpp>
#include <geos/operation/overlayng/OverlayNG.hpp>
#include <geos/operation/overlayng/OverlayNGRobust.hpp>
#include <geos/operation/valid/IsSimpleOp.hpp>
#include <geos/operation/valid/IsValidOp.hpp>
#include <geos/operation/valid/TopologyValidationError.hpp>
#include <geos/precision/CommonBitsRemover.hpp>
#include <geos/precision/GeometryPrecisionReducer.hpp>
#include <geos/util.hpp>
#include <geos/util/TopologyException.hpp>

/*
 * Define this to use OverlayNG policy with whatever precision
 */
#if !defined(DISABLE_OVERLAYNG) && !defined(USE_OVERLAYNG_SNAPIFNEEDED)
#define USE_OVERLAYNG_SNAPIFNEEDED
#endif

/*
 * Always try original input first
 */
#ifndef USE_ORIGINAL_INPUT
#define USE_ORIGINAL_INPUT 1
#endif

/*
 * Check validity of operation between original geometries
 */
#define GEOS_CHECK_ORIGINAL_RESULT_VALIDITY 0

/*
 * Define this to use OverlayNG policy with fixed precision
 */
#ifndef USE_FIXED_PRECISION_OVERLAYNG
#define USE_FIXED_PRECISION_OVERLAYNG 0
#endif

/*
 * Define this to use PrecisionReduction policy
 * in an attempt at by-passing binary operation
 * robustness problems (handles TopologyExceptions)
 */
#ifndef USE_PRECISION_REDUCTION_POLICY
#define USE_PRECISION_REDUCTION_POLICY 1
#endif

/*
 * Check validity of operation performed
 * by precision reduction policy.
 *
 * Precision reduction policy reduces precision of inputs
 * and restores it in the result. The restore phase may
 * introduce invalidities.
 *
 */
#define GEOS_CHECK_PRECISION_REDUCTION_VALIDITY 0

/*
 * Define this to use TopologyPreserving simplification policy
 * in an attempt at by-passing binary operation
 * robustness problems (handles TopologyExceptions)
 */
#ifndef USE_TP_SIMPLIFY_POLICY
// # define USE_TP_SIMPLIFY_POLICY 1
#endif

/*
 * Use common bits removal policy.
 * If enabled, this would be tried /before/
 * Geometry snapping.
 */
#ifndef USE_COMMONBITS_POLICY
#define USE_COMMONBITS_POLICY 1
#endif

/*
 * Check validity of operation performed
 * by common bits removal policy.
 *
 * This matches what EnhancedPrecisionOp does in JTS
 * and fixes 5 tests of invalid outputs in our testsuite
 * (stmlf-cases-20061020-invalid-output.xml)
 * and breaks 1 test (robustness-invalid-output.xml) so much
 * to prevent a result.
 *
 */
#define GEOS_CHECK_COMMONBITS_VALIDITY 1

/*
 * Use snapping policy
 */
#ifndef USE_SNAPPING_POLICY
#define USE_SNAPPING_POLICY 1
#endif

/* Remove common bits before snapping */
#ifndef CBR_BEFORE_SNAPPING
#define CBR_BEFORE_SNAPPING 1
#endif

/*
 * Check validity of result from SnapOp
 */
#define GEOS_CHECK_SNAPPINGOP_VALIDITY 0

using geos::operation::overlay::OverlayOp;
using geos::operation::overlayng::OverlayNG;

namespace geos {
namespace geom { // geos::geom

inline bool check_valid(const Geometry &g, const std::string &label, bool doThrow = false, bool validOnly = false) {
	if (g.isLineal()) {
		if (!validOnly) {
			operation::valid::IsSimpleOp sop(g, algorithm::BoundaryNodeRule::getBoundaryEndPoint());
			if (!sop.isSimple()) {
				if (doThrow) {
					throw geos::util::TopologyException(label + " is not simple");
				}
				return false;
			}
		}
	} else {
		operation::valid::IsValidOp ivo(&g);
		if (!ivo.isValid()) {
			using operation::valid::TopologyValidationError;
			const TopologyValidationError *err = ivo.getValidationError();
			if (doThrow) {
				throw geos::util::TopologyException(label + " is invalid: " + err->getMessage(),
				                                    Coordinate(err->getCoordinate()));
			}
			return false;
		}
	}
	return true;
}

/*
 * Attempt to fix noding of multilines and
 * self-intersection of multipolygons
 *
 * May return the input untouched.
 */
inline std::unique_ptr<Geometry> fix_self_intersections(std::unique_ptr<Geometry> g, const std::string &label) {
	::geos::ignore_unused_variable_warning(label);

	// Only multi-components can be fixed by UnaryUnion
	if (!dynamic_cast<const GeometryCollection *>(g.get())) {
		return g;
	}

	using operation::valid::IsValidOp;

	IsValidOp ivo(g.get());

	// Polygon is valid, nothing to do
	if (ivo.isValid()) {
		return g;
	}

	// Not all invalidities can be fixed by this code

	using operation::valid::TopologyValidationError;
	const TopologyValidationError *err = ivo.getValidationError();
	switch (err->getErrorType()) {
	case TopologyValidationError::eRingSelfIntersection:
	case TopologyValidationError::eTooFewPoints: // collapsed lines
		g = g->Union();
		return g;
	case TopologyValidationError::eSelfIntersection:
	// this one is within a single component, won't be fixed
	default:
		return g;
	}
}

/// \brief
/// Apply an overlay operation to the given geometries
/// after snapping them to each other after common-bits
/// removal.
///
std::unique_ptr<Geometry> SnapOp(const Geometry *g0, const Geometry *g1, int opCode) {
	typedef std::unique_ptr<Geometry> GeomPtr;

	// using geos::precision::GeometrySnapper;
	using geos::operation::overlay::snap::GeometrySnapper;

	// Snap tolerance must be computed on the original
	// (not commonbits-removed) geoms
	double snapTolerance = GeometrySnapper::computeOverlaySnapTolerance(*g0, *g1);

#if CBR_BEFORE_SNAPPING
	// Compute common bits
	geos::precision::CommonBitsRemover cbr;
	cbr.add(g0);
	cbr.add(g1);

	// Now remove common bits
	GeomPtr rG0 = g0->clone();
	cbr.removeCommonBits(rG0.get());
	GeomPtr rG1 = g1->clone();
	cbr.removeCommonBits(rG1.get());

	const Geometry &operand0 = *rG0;
	const Geometry &operand1 = *rG1;

#else // don't CBR before snapping
    const Geometry& operand0 = *g0;
    const Geometry& operand1 = *g1;
#endif

	GeometrySnapper snapper0(operand0);
	GeomPtr snapG0(snapper0.snapTo(operand1, snapTolerance));
	// snapG0 = fix_self_intersections(snapG0, "SNAP: snapped geom 0");

	// NOTE: second geom is snapped on the snapped first one
	GeometrySnapper snapper1(operand1);
	GeomPtr snapG1(snapper1.snapTo(*snapG0, snapTolerance));
	// snapG1 = fix_self_intersections(snapG1, "SNAP: snapped geom 1");

	// Run the overlay op
	GeomPtr result(OverlayOp::overlayOp(snapG0.get(), snapG1.get(), OverlayOp::OpCode(opCode)));

#if CBR_BEFORE_SNAPPING
	// Add common bits back in
	cbr.addCommonBits(result.get());
	// result = fix_self_intersections(result, "SNAP: result (after common-bits addition)");

	check_valid(*result, "CBR: result (after common-bits addition)", true);

#endif

	return result;
}

std::unique_ptr<Geometry> HeuristicOverlay(const Geometry *g0, const Geometry *g1, int opCode) {
	typedef std::unique_ptr<Geometry> GeomPtr;

	GeomPtr ret;
	geos::util::TopologyException origException;

/**************************************************************************/

/*
 * overlayng::OverlayNGRobust carries out the following steps
 *
 * 1. Perform overlay operation using PrecisionModel(float).
 *    If no exception return result.
 * 2. Perform overlay operation using SnappingNoder(tolerance), starting
 *    with a very very small tolerance and increasing it for 5 iterations.
 *    The SnappingNoder moves only nodes that are within tolerance of
 *    other nodes and lines, leaving all the rest undisturbed, for a very
 *    clean result, if it manages to create one.
 *    If a result is found with no exception, return.
 * 3. Perform overlay operation using a PrecisionModel(scale), which
 *    uses a SnapRoundingNoder. Every vertex will be noded to the snapping
 *    grid, resulting in a modified geometry. The SnapRoundingNoder approach
 *    reliably produces results, assuming valid inputs.
 *
 * Running overlayng::OverlayNGRobust at this stage should guarantee
 * that none of the other heuristics are ever needed.
 */
#ifdef USE_OVERLAYNG_SNAPIFNEEDED

	try {
		if (g0 == nullptr && g1 == nullptr) {
			return std::unique_ptr<Geometry>(nullptr);
		} else if (g0 == nullptr) {
			// Use a uniary union for the one-parameter case, as the pairwise
			// union with one parameter is very intolerant to invalid
			// collections and multi-polygons.
			ret = operation::overlayng::OverlayNGRobust::Union(g1);
		} else if (g1 == nullptr) {
			// Use a uniary union for the one-parameter case, as the pairwise
			// union with one parameter is very intolerant to invalid
			// collections and multi-polygons.
			ret = operation::overlayng::OverlayNGRobust::Union(g0);
		} else {
			ret = operation::overlayng::OverlayNGRobust::Overlay(g0, g1, opCode);
		}

		return ret;
	} catch (const std::exception &ex) {
		::geos::ignore_unused_variable_warning(ex);
	}

	check_valid(*g0, "Input geom 0", true, true);
	check_valid(*g1, "Input geom 1", true, true);

#endif // USE_OVERLAYNG_SNAPIFNEEDED }

	/**************************************************************************/

#ifdef USE_ORIGINAL_INPUT
	// Try with original input
	try {
		ret.reset(OverlayOp::overlayOp(g0, g1, OverlayOp::OpCode(opCode)));
		return ret;
	} catch (const geos::util::TopologyException &ex) {
		origException = ex;
	}
#endif // USE_ORIGINAL_INPUT

	/**************************************************************************/

	check_valid(*g0, "Input geom 0", true, true);
	check_valid(*g1, "Input geom 1", true, true);

#if USE_COMMONBITS_POLICY
	// Try removing common bits (possibly obsoleted by snapping below)
	//
	// NOTE: this policy was _later_ implemented
	//       in JTS as EnhancedPrecisionOp
	// TODO: consider using the now-ported EnhancedPrecisionOp
	//       here too
	//
	try {
		GeomPtr rG0;
		GeomPtr rG1;
		precision::CommonBitsRemover cbr;

		cbr.add(g0);
		cbr.add(g1);

		rG0 = g0->clone();
		cbr.removeCommonBits(rG0.get());

		rG1 = g1->clone();
		cbr.removeCommonBits(rG1.get());

		ret.reset(OverlayOp::overlayOp(rG0.get(), rG1.get(), OverlayOp::OpCode(opCode)));

		cbr.addCommonBits(ret.get());

#if GEOS_CHECK_COMMONBITS_VALIDITY
		check_valid(*ret, "CBR: result (after common-bits addition)", true);
#endif

		return ret;
	} catch (const geos::util::TopologyException &ex) {
		::geos::ignore_unused_variable_warning(ex);
	}
#endif

/**************************************************************************/

// Try with snapping
//
// TODO: possible optimization would be reusing the
//       already common-bit-removed inputs and just
//       apply geometry snapping, whereas the current
//       SnapOp function does both.
// {
#if USE_SNAPPING_POLICY

	try {
		ret = SnapOp(g0, g1, opCode);
		return ret;

	} catch (const geos::util::TopologyException &ex) {
		::geos::ignore_unused_variable_warning(ex);
	}

#endif // USE_SNAPPING_POLICY }

/**************************************************************************/

// {
#if USE_PRECISION_REDUCTION_POLICY

	// Try reducing precision
	try {
		long unsigned int g0scale = static_cast<long unsigned int>(g0->getFactory()->getPrecisionModel()->getScale());
		long unsigned int g1scale = static_cast<long unsigned int>(g1->getFactory()->getPrecisionModel()->getScale());

		double maxScale = 1e16; // TODO: compute from input
		double minScale = 1;    // TODO: compute from input

		// Don't use a scale bigger than the input one
		if (g0scale && static_cast<double>(g0scale) < maxScale) {
			maxScale = static_cast<double>(g0scale);
		}
		if (g1scale && static_cast<double>(g1scale) < maxScale) {
			maxScale = static_cast<double>(g1scale);
		}

		for (double scale = maxScale; scale >= minScale; scale /= 10) {
			PrecisionModel pm(scale);
			GeometryFactory::Ptr gf = GeometryFactory::create(&pm);

			precision::GeometryPrecisionReducer reducer(*gf);
			reducer.setUseAreaReducer(false);
			reducer.setChangePrecisionModel(true);
			GeomPtr rG0(reducer.reduce(*g0));
			GeomPtr rG1(reducer.reduce(*g1));

			try {
				ret.reset(OverlayOp::overlayOp(rG0.get(), rG1.get(), OverlayOp::OpCode(opCode)));
				// restore original precision (least precision between inputs)
				if (g0->getFactory()->getPrecisionModel()->compareTo(g1->getFactory()->getPrecisionModel()) < 0) {
					ret.reset(g0->getFactory()->createGeometry(ret.get()));
				} else {
					ret.reset(g1->getFactory()->createGeometry(ret.get()));
				}
				return ret;
			} catch (const geos::util::TopologyException &ex) {
				(void)ex; // quiet compiler warning about unused variable
				if (scale == 1) {
					throw;
				}
			}
		}

	} catch (const geos::util::TopologyException &ex) {
		::geos::ignore_unused_variable_warning(ex);
	}

#endif
// USE_PRECISION_REDUCTION_POLICY }

/**************************************************************************/

// {
// USE_FIXED_PRECISION_OVERLAYNG }

/**************************************************************************/

// {
	// USE_TP_SIMPLIFY_POLICY }

	/**************************************************************************/

	throw origException;
}

} // namespace geom
} // namespace geos
