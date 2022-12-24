/************************************************************************
 *
 *
 * C-Wrapper for GEOS library
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2010-2012 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2016-2021 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 * Author: Sandro Santilli <strk@kbt.io>
 * Thread Safety modifications: Chuck Thibert <charles.thibert@ingres.com>
 *
 ***********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/FixedSizeCoordinateSequence.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/Point.hpp>
#include <geos/operation/buffer/BufferOp.hpp>
#include <geos/operation/buffer/BufferParameters.hpp>
#include <geos/operation/overlayng/OverlayNG.hpp>
#include <geos/operation/overlayng/OverlayNGRobust.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <geos/util/Interrupt.hpp>
#include <geos/util/Machine.hpp>

// This should go away
#include <cmath> // finite
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

// Some extra magic to make type declarations in geos_c.h work -
// for cross-checking of types in header.
#define GEOSGeometry      geos::geom::Geometry
#define GEOSCoordSequence geos::geom::CoordinateSequence
#define GEOSBufferParams  geos::operation::buffer::BufferParameters

#include "geos_c.hpp"

// Intentional, to allow non-standard C elements like C99 functions to be
// imported through C++ headers of C library, like <cmath>.
using namespace std;

/// Define this if you want operations triggering Exceptions to
/// be printed.
/// (will use the NOTIFY channel - only implemented for GEOSUnion so far)
///
#undef VERBOSE_EXCEPTIONS

#include "geos/export.hpp"

// import the most frequently used definitions globally
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::Polygon;
using geos::geom::PrecisionModel;

using geos::operation::overlayng::OverlayNG;
using geos::operation::overlayng::OverlayNGRobust;

using geos::operation::buffer::BufferParameters;

using geos::util::IllegalArgumentException;

typedef std::unique_ptr<Geometry> GeomPtr;

typedef struct GEOSContextHandle_HS {
	const GeometryFactory *geomFactory;
	char msgBuffer[1024];
	GEOSMessageHandler noticeMessageOld;
	GEOSMessageHandler_r noticeMessageNew;
	void *noticeData;
	GEOSMessageHandler errorMessageOld;
	GEOSMessageHandler_r errorMessageNew;
	void *errorData;
	uint8_t WKBOutputDims;
	int WKBByteOrder;
	int initialized;
	std::unique_ptr<geos::geom::Point> point2d;

	GEOSContextHandle_HS()
	    : geomFactory(nullptr), noticeMessageOld(nullptr), noticeMessageNew(nullptr), noticeData(nullptr),
	      errorMessageOld(nullptr), errorMessageNew(nullptr), errorData(nullptr), point2d(nullptr) {
		memset(msgBuffer, 0, sizeof(msgBuffer));
		geomFactory = GeometryFactory::getDefaultInstance();
		point2d = geomFactory->createPoint(geos::geom::CoordinateXY {0, 0});
		WKBOutputDims = 2;
		WKBByteOrder = getMachineByteOrder();
		setNoticeHandler(nullptr);
		setErrorHandler(nullptr);
		initialized = 1;
	}

	GEOSMessageHandler setNoticeHandler(GEOSMessageHandler nf) {
		GEOSMessageHandler f = noticeMessageOld;
		noticeMessageOld = nf;
		noticeMessageNew = nullptr;
		noticeData = nullptr;

		return f;
	}

	GEOSMessageHandler setErrorHandler(GEOSMessageHandler nf) {
		GEOSMessageHandler f = errorMessageOld;
		errorMessageOld = nf;
		errorMessageNew = nullptr;
		errorData = nullptr;

		return f;
	}

	GEOSMessageHandler_r setNoticeHandler(GEOSMessageHandler_r nf, void *userData) {
		GEOSMessageHandler_r f = noticeMessageNew;
		noticeMessageOld = nullptr;
		noticeMessageNew = nf;
		noticeData = userData;

		return f;
	}

	GEOSMessageHandler_r setErrorHandler(GEOSMessageHandler_r ef, void *userData) {
		GEOSMessageHandler_r f = errorMessageNew;
		errorMessageOld = nullptr;
		errorMessageNew = ef;
		errorData = userData;

		return f;
	}

	void NOTICE_MESSAGE(const char *fmt, ...) {
		if (nullptr == noticeMessageOld && nullptr == noticeMessageNew) {
			return;
		}

		va_list args;
		va_start(args, fmt);
		int result = vsnprintf(msgBuffer, sizeof(msgBuffer) - 1, fmt, args);
		va_end(args);

		if (result > 0) {
			if (noticeMessageOld) {
				noticeMessageOld("%s", msgBuffer);
			} else {
				noticeMessageNew(msgBuffer, noticeData);
			}
		}
	}

	void ERROR_MESSAGE(const char *fmt, ...) {
		if (nullptr == errorMessageOld && nullptr == errorMessageNew) {
			return;
		}

		va_list args;
		va_start(args, fmt);
		int result = vsnprintf(msgBuffer, sizeof(msgBuffer) - 1, fmt, args);
		va_end(args);

		if (result > 0) {
			if (errorMessageOld) {
				errorMessageOld("%s", msgBuffer);
			} else {
				errorMessageNew(msgBuffer, errorData);
			}
		}
	}
} GEOSContextHandleInternal_t;

// Execute a lambda, using the given context handle to process errors.
// Return errval on error.
// Errval should be of the type returned by f, unless f returns a bool in which case we promote to char.
template <typename F>
inline auto execute(GEOSContextHandle_t extHandle,
                    typename std::conditional<std::is_same<decltype(std::declval<F>()()), bool>::value, char,
                                              decltype(std::declval<F>()())>::type errval,
                    F &&f) -> decltype(errval) {
	if (extHandle == nullptr) {
		return errval;
	}

	GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
	if (!handle->initialized) {
		return errval;
	}

	try {
		return f();
	} catch (const std::exception &e) {
		handle->ERROR_MESSAGE("%s", e.what());
	} catch (...) {
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}

	return errval;
}

// Execute a lambda, using the given context handle to process errors.
// Return nullptr on error.
template <typename F,
          typename std::enable_if<!std::is_void<decltype(std::declval<F>()())>::value, std::nullptr_t>::type = nullptr>
inline auto execute(GEOSContextHandle_t extHandle, F &&f) -> decltype(f()) {
	if (extHandle == nullptr) {
		return nullptr;
	}

	GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
	if (!handle->initialized) {
		return nullptr;
	}

	try {
		return f();
	} catch (const std::exception &e) {
		handle->ERROR_MESSAGE("%s", e.what());
	} catch (...) {
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}

	return nullptr;
}

// Execute a lambda, using the given context handle to process errors.
// No return value.
template <typename F,
          typename std::enable_if<std::is_void<decltype(std::declval<F>()())>::value, std::nullptr_t>::type = nullptr>
inline void execute(GEOSContextHandle_t extHandle, F &&f) {
	GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
	try {
		f();
	} catch (const std::exception &e) {
		handle->ERROR_MESSAGE("%s", e.what());
	} catch (...) {
		handle->ERROR_MESSAGE("Unknown exception thrown");
	}
}

extern "C" {

GEOSContextHandle_t initGEOS_r(GEOSMessageHandler nf, GEOSMessageHandler ef) {
	GEOSContextHandle_t handle = GEOS_init_r();

	if (nullptr != handle) {
		GEOSContext_setNoticeHandler_r(handle, nf);
		GEOSContext_setErrorHandler_r(handle, ef);
	}

	return handle;
}

GEOSContextHandle_t GEOS_init_r() {
	GEOSContextHandleInternal_t *handle = new GEOSContextHandleInternal_t();

	geos::util::Interrupt::cancel();

	return static_cast<GEOSContextHandle_t>(handle);
}

// Return postgis geometry type index
int GEOSGeomTypeId_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, -1, [&]() { return static_cast<int>(g1->getGeometryTypeId()); });
}

int GEOSGetSRID_r(GEOSContextHandle_t extHandle, const Geometry *g) {
	return execute(extHandle, 0, [&]() { return g->getSRID(); });
}

void GEOSSetSRID_r(GEOSContextHandle_t extHandle, Geometry *g, int srid) {
	execute(extHandle, [&]() { g->setSRID(srid); });
}

char GEOSHasZ_r(GEOSContextHandle_t extHandle, const Geometry *g) {
	return execute(extHandle, 2, [&]() {
		if (g->isEmpty()) {
			return false;
		}

		return g->getCoordinateDimension() == 3;
	});
}

char GEOSisEmpty_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, 2, [&]() { return g1->isEmpty(); });
}

const CoordinateSequence *GEOSGeom_getCoordSeq_r(GEOSContextHandle_t extHandle, const Geometry *g) {
	using geos::geom::Point;

	return execute(extHandle, [&]() {
		const LineString *ls = dynamic_cast<const LineString *>(g);
		if (ls) {
			return ls->getCoordinatesRO();
		}

		const Point *p = dynamic_cast<const Point *>(g);
		if (p) {
			return p->getCoordinatesRO();
		}

		throw IllegalArgumentException("Geometry must be a Point or LineString");
	});
}

CoordinateSequence *GEOSCoordSeq_create_r(GEOSContextHandle_t extHandle, unsigned int size, unsigned int dims) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);

		switch (size) {
		case 1:
			return static_cast<CoordinateSequence *>(new geos::geom::FixedSizeCoordinateSequence<1>(dims));
		case 2:
			return static_cast<CoordinateSequence *>(new geos::geom::FixedSizeCoordinateSequence<2>(dims));
		default: {
			const GeometryFactory *gf = handle->geomFactory;
			return gf->getCoordinateSequenceFactory()->create(size, dims).release();
		}
		}
	});
}

int GEOSCoordSeq_setXY_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs, unsigned int idx, double x, double y) {
	return execute(extHandle, 0, [&]() {
		cs->setAt(geos::geom::Coordinate {x, y}, idx);
		return 1;
	});
}

int GEOSCoordSeq_setXYZ_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs, unsigned int idx, double x, double y,
                          double z) {
	return execute(extHandle, 0, [&]() {
		cs->setAt({x, y, z}, idx);
		return 1;
	});
}

int GEOSCoordSeq_setOrdinate_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs, unsigned int idx,
                               unsigned int dim, double val) {
	return execute(extHandle, 0, [&]() {
		cs->setOrdinate(idx, dim, val);
		return 1;
	});
}

int GEOSCoordSeq_getSize_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs, unsigned int *size) {
	return execute(extHandle, 0, [&]() {
		const std::size_t sz = cs->getSize();
		*size = static_cast<unsigned int>(sz);
		return 1;
	});
}

int GEOSCoordSeq_getDimensions_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs, unsigned int *dims) {
	return execute(extHandle, 0, [&]() {
		const std::size_t dim = cs->getDimension();
		*dims = static_cast<unsigned int>(dim);

		return 1;
	});
}

int GEOSCoordSeq_getXY_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs, unsigned int idx, double *x,
                         double *y) {
	return execute(extHandle, 0, [&]() {
		auto &c = cs->getAt(idx);
		*x = c.x;
		*y = c.y;
		return 1;
	});
}

int GEOSCoordSeq_getXYZ_r(GEOSContextHandle_t extHandle, const CoordinateSequence *cs, unsigned int idx, double *x,
                          double *y, double *z) {
	return execute(extHandle, 0, [&]() {
		auto &c = cs->getAt(idx);
		*x = c.x;
		*y = c.y;
		*z = c.z;
		return 1;
	});
}

char GEOSisRing_r(GEOSContextHandle_t extHandle, const Geometry *g) {
	return execute(extHandle, 2, [&]() {
		// both LineString* and LinearRing* can cast to LineString*
		const LineString *ls = dynamic_cast<const LineString *>(g);
		if (ls) {
			return ls->isRing();
		} else {
			return false;
		}
	});
}

int GEOSGetNumInteriorRings_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, -1, [&]() {
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if (!p) {
			throw IllegalArgumentException("Argument is not a Polygon");
		}
		return static_cast<int>(p->getNumInteriorRing());
	});
}

/*
 * Call only on polygon
 * Return a pointer to the internal Geometry.
 */
const Geometry *GEOSGetExteriorRing_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, [&]() {
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if (!p) {
			throw IllegalArgumentException("Invalid argument (must be a Polygon)");
		}
		return p->getExteriorRing();
	});
}

/*
 * Call only on polygon
 * Return a pointer to internal storage, do not destroy it.
 */
const Geometry *GEOSGetInteriorRingN_r(GEOSContextHandle_t extHandle, const Geometry *g1, int n) {
	return execute(extHandle, [&]() {
		const Polygon *p = dynamic_cast<const Polygon *>(g1);
		if (!p) {
			throw IllegalArgumentException("Invalid argument (must be a Polygon)");
		}
		if (n < 0) {
			throw IllegalArgumentException("Index must be non-negative.");
		}
		return p->getInteriorRingN(static_cast<size_t>(n));
	});
}

// returns -1 on error and 1 for non-multi geometries
int GEOSGetNumGeometries_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, -1, [&]() { return static_cast<int>(g1->getNumGeometries()); });
}

/*
 * Call only on GEOMETRYCOLLECTION or MULTI*.
 * Return a pointer to the internal Geometry.
 */
const Geometry *GEOSGetGeometryN_r(GEOSContextHandle_t extHandle, const Geometry *g1, int n) {
	return execute(extHandle, [&]() {
		if (n < 0) {
			throw IllegalArgumentException("Index must be non-negative.");
		}
		return g1->getGeometryN(static_cast<size_t>(n));
	});
}

Geometry *GEOSGeom_createPoint_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
		const GeometryFactory *gf = handle->geomFactory;

		return gf->createPoint(cs);
	});
}

Geometry *GEOSGeom_createPointFromXY_r(GEOSContextHandle_t extHandle, double x, double y) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
		const GeometryFactory *gf = handle->geomFactory;

		geos::geom::Coordinate c(x, y);
		return gf->createPoint(c);
	});
}

Geometry *GEOSGeom_createLinearRing_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
		const GeometryFactory *gf = handle->geomFactory;

		return gf->createLinearRing(cs);
	});
}

Geometry *GEOSGeom_createLineString_r(GEOSContextHandle_t extHandle, CoordinateSequence *cs) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
		const GeometryFactory *gf = handle->geomFactory;

		return gf->createLineString(cs);
	});
}

Geometry *GEOSGeom_createEmptyPolygon_r(GEOSContextHandle_t extHandle) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
		const GeometryFactory *gf = handle->geomFactory;
		return gf->createPolygon().release();
	});
}

Geometry *GEOSGeom_createPolygon_r(GEOSContextHandle_t extHandle, Geometry *shell, Geometry **holes,
                                   unsigned int nholes) {
	using geos::geom::LinearRing;

	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
		const GeometryFactory *gf = handle->geomFactory;
		bool good_holes = true, good_shell = true;

		// Validate input before taking ownership
		for (std::size_t i = 0; i < nholes; i++) {
			if ((!holes) || (!dynamic_cast<LinearRing *>(holes[i]))) {
				good_holes = false;
				break;
			}
		}
		if (!dynamic_cast<LinearRing *>(shell)) {
			good_shell = false;
		}

		// Contract for GEOSGeom_createPolygon is to take ownership of arguments
		// which implies freeing them on exception,
		// see https://trac.osgeo.org/geos/ticket/1111
		if (!(good_holes && good_shell)) {
			if (shell)
				delete shell;
			for (std::size_t i = 0; i < nholes; i++) {
				if (holes && holes[i])
					delete holes[i];
			}
			if (!good_shell)
				throw IllegalArgumentException("Shell is not a LinearRing");
			else
				throw IllegalArgumentException("Hole is not a LinearRing");
		}

		std::unique_ptr<LinearRing> tmpshell(static_cast<LinearRing *>(shell));
		if (nholes) {
			std::vector<std::unique_ptr<LinearRing>> tmpholes(nholes);
			for (size_t i = 0; i < nholes; i++) {
				tmpholes[i].reset(static_cast<LinearRing *>(holes[i]));
			}

			return gf->createPolygon(std::move(tmpshell), std::move(tmpholes)).release();
		}

		return gf->createPolygon(std::move(tmpshell)).release();
	});
}

Geometry *GEOSGeom_createCollection_r(GEOSContextHandle_t extHandle, int type, Geometry **geoms, unsigned int ngeoms) {
	return execute(extHandle, [&]() {
		GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);

		const GeometryFactory *gf = handle->geomFactory;

		std::vector<std::unique_ptr<Geometry>> vgeoms(ngeoms);
		for (std::size_t i = 0; i < ngeoms; i++) {
			vgeoms[i].reset(geoms[i]);
		}

		std::unique_ptr<Geometry> g;
		switch (type) {
		case GEOS_GEOMETRYCOLLECTION:
			g = gf->createGeometryCollection(std::move(vgeoms));
			break;
		case GEOS_MULTIPOINT:
			g = gf->createMultiPoint(std::move(vgeoms));
			break;
		case GEOS_MULTILINESTRING:
			g = gf->createMultiLineString(std::move(vgeoms));
			break;
		case GEOS_MULTIPOLYGON:
			g = gf->createMultiPolygon(std::move(vgeoms));
			break;
		default:
			handle->ERROR_MESSAGE("Unsupported type request for PostGIS2GEOS_collection");
		}

		return g.release();
	});
}

Geometry *GEOSUnion_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, [&]() {
		auto g3 = g1->Union(g2);
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

Geometry *GEOSUnaryUnion_r(GEOSContextHandle_t extHandle, const Geometry *g) {
	return execute(extHandle, [&]() {
		GeomPtr g3(g->Union());
		g3->setSRID(g->getSRID());
		return g3.release();
	});
}

Geometry *GEOSGetCentroid_r(GEOSContextHandle_t extHandle, const Geometry *g) {
	return execute(extHandle, [&]() -> Geometry * {
		auto ret = g->getCentroid();

		if (ret == nullptr) {
			// TODO check if getCentroid() can really return null
			const GeometryFactory *gf = g->getFactory();
			ret = gf->createPoint();
		}
		ret->setSRID(g->getSRID());
		return ret.release();
	});
}

//-------------------------------------------------------------------
// memory management functions
//------------------------------------------------------------------

void GEOSGeom_destroy_r(GEOSContextHandle_t extHandle, Geometry *a) {
	execute(extHandle, [&]() {
		// FIXME: mloskot: Does this try-catch around delete means that
		// destructors in GEOS may throw? If it does, this is a serious
		// violation of "never throw an exception from a destructor" principle
		delete a;
	});
}

//-------------------------------------------------------------------
// GEOS functions that return geometries
//-------------------------------------------------------------------

Geometry *GEOSDifference_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, [&]() {
		auto g3 = g1->difference(g2);
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

Geometry *GEOSDifferencePrec_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, double gridSize) {
	return execute(extHandle, [&]() {
		std::unique_ptr<PrecisionModel> pm;
		if (gridSize != 0) {
			pm.reset(new PrecisionModel(1.0 / gridSize));
		} else {
			pm.reset(new PrecisionModel());
		}
		auto g3 = gridSize != 0 ? OverlayNG::overlay(g1, g2, OverlayNG::DIFFERENCE, pm.get())
		                        : OverlayNGRobust::Overlay(g1, g2, OverlayNG::DIFFERENCE);
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

GEOSMessageHandler GEOSContext_setNoticeHandler_r(GEOSContextHandle_t extHandle, GEOSMessageHandler nf) {
	GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
	if (0 == handle->initialized) {
		return nullptr;
	}

	return handle->setNoticeHandler(nf);
}

GEOSMessageHandler GEOSContext_setErrorHandler_r(GEOSContextHandle_t extHandle, GEOSMessageHandler nf) {
	GEOSContextHandleInternal_t *handle = reinterpret_cast<GEOSContextHandleInternal_t *>(extHandle);
	if (0 == handle->initialized) {
		return nullptr;
	}

	return handle->setErrorHandler(nf);
}

Geometry *GEOSBoundary_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, [&]() {
		auto g3 = g1->getBoundary();
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

Geometry *GEOSConvexHull_r(GEOSContextHandle_t extHandle, const Geometry *g1) {
	return execute(extHandle, [&]() {
		auto g3 = g1->convexHull();
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

Geometry *GEOSIntersection_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, [&]() {
		auto g3 = g1->intersection(g2);
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

Geometry *GEOSIntersectionPrec_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2,
                                 double gridSize) {
	return execute(extHandle, [&]() {
		using geos::geom::PrecisionModel;

		std::unique_ptr<PrecisionModel> pm;
		if (gridSize != 0) {
			pm.reset(new PrecisionModel(1.0 / gridSize));
		} else {
			pm.reset(new PrecisionModel());
		}
		auto g3 = gridSize != 0 ? OverlayNG::overlay(g1, g2, OverlayNG::INTERSECTION, pm.get())
		                        : OverlayNGRobust::Overlay(g1, g2, OverlayNG::INTERSECTION);
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

//-----------------------------------------------------------------
// STRtree
//-----------------------------------------------------------------

BufferParameters *GEOSBufferParams_create_r(GEOSContextHandle_t extHandle) {
	return execute(extHandle, [&]() { return new BufferParameters(); });
}

void GEOSBufferParams_destroy_r(GEOSContextHandle_t extHandle, BufferParameters *p) {
	(void)extHandle;
	delete p;
}

int GEOSBufferParams_setEndCapStyle_r(GEOSContextHandle_t extHandle, GEOSBufferParams *p, int style) {
	return execute(extHandle, 0, [&]() {
		if (style > BufferParameters::CAP_SQUARE) {
			throw IllegalArgumentException("Invalid buffer endCap style");
		}
		p->setEndCapStyle(static_cast<BufferParameters::EndCapStyle>(style));
		return 1;
	});
}

int GEOSBufferParams_setJoinStyle_r(GEOSContextHandle_t extHandle, GEOSBufferParams *p, int style) {
	return execute(extHandle, 0, [&]() {
		if (style > BufferParameters::JOIN_BEVEL) {
			throw IllegalArgumentException("Invalid buffer join style");
		}
		p->setJoinStyle(static_cast<BufferParameters::JoinStyle>(style));

		return 1;
	});
}

int GEOSBufferParams_setMitreLimit_r(GEOSContextHandle_t extHandle, GEOSBufferParams *p, double limit) {
	return execute(extHandle, 0, [&]() {
		p->setMitreLimit(limit);
		return 1;
	});
}

int GEOSBufferParams_setQuadrantSegments_r(GEOSContextHandle_t extHandle, GEOSBufferParams *p, int segs) {
	return execute(extHandle, 0, [&]() {
		p->setQuadrantSegments(segs);
		return 1;
	});
}

int GEOSBufferParams_setSingleSided_r(GEOSContextHandle_t extHandle, GEOSBufferParams *p, int ss) {
	return execute(extHandle, 0, [&]() {
		p->setSingleSided((ss != 0));
		return 1;
	});
}

Geometry *GEOSBufferWithParams_r(GEOSContextHandle_t extHandle, const Geometry *g1, const BufferParameters *bp,
                                 double width) {
	using geos::operation::buffer::BufferOp;

	return execute(extHandle, [&]() {
		BufferOp op(g1, *bp);
		std::unique_ptr<Geometry> g3 = op.getResultGeometry(width);
		g3->setSRID(g1->getSRID());
		return g3.release();
	});
}

//-----------------------------------------------------------------
// general purpose
//-----------------------------------------------------------------

char GEOSContains_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, 2, [&]() { return g1->contains(g2); });
}

char GEOSEquals_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, 2, [&]() { return g1->equals(g2); });
}

char GEOSDisjoint_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, 2, [&]() { return g1->disjoint(g2); });
}

char GEOSTouches_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, 2, [&]() { return g1->touches(g2); });
}

char GEOSIntersects_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2) {
	return execute(extHandle, 2, [&]() { return g1->intersects(g2); });
}

//-------------------------------------------------------------------
// low-level relate functions
//------------------------------------------------------------------

char GEOSRelatePattern_r(GEOSContextHandle_t extHandle, const Geometry *g1, const Geometry *g2, const char *pat) {
	return execute(extHandle, 2, [&]() {
		std::string s(pat);
		return g1->relate(g2, s);
	});
}

} /* extern "C" */
