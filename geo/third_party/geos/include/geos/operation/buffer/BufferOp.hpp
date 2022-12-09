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

#pragma once

#include <geos/export.hpp>
#include <geos/operation/buffer/BufferParameters.hpp> // for enum values
#include <geos/util/TopologyException.hpp>            // for composition
#include <memory>                                     // for unique_ptr
#include <vector>                                     // for vector

// Forward declarations
namespace geos {
namespace geom {
class PrecisionModel;
class Geometry;
} // namespace geom
} // namespace geos

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

/**
 * \class BufferOp
 *
 * \brief
 * Computes the buffer of a geometry, for both positive and negative
 * buffer distances.
 *
 * In GIS, the positive (or negative) buffer of a geometry is defined as
 * the Minkowski sum (or difference) of the geometry
 * with a circle with radius equal to the absolute value of the buffer
 * distance.
 * In the CAD/CAM world buffers are known as *offset curves*.
 * In morphological analysis the operation of positive and negative buffering
 * is referred to as *erosion* and *dilation*.
 *
 * The buffer operation always returns a polygonal result.
 * The negative or zero-distance buffer of lines and points is always
 * an empty Polygon.
 *
 * Since true buffer curves may contain circular arcs,
 * computed buffer polygons can only be approximations to the true geometry.
 * The user can control the accuracy of the curve approximation by specifying
 * the number of linear segments with which to approximate a curve.
 *
 * The end cap style of a linear buffer may be specified.
 * The following end cap styles are supported:
 * - CAP_ROUND - the usual round end caps
 * - CAP_BUTT - end caps are truncated flat at the line ends
 * - CAP_SQUARE - end caps are squared off at the buffer distance
 *   beyond the line ends
 *
 */
class GEOS_DLL BufferOp {
private:
	/** \brief
	 *  A number of digits of precision which leaves some computational "headroom"
	 *  for floating point operations.
	 *
	 *  This value should be less than the decimal precision of double-precision values (16).
	 */
	static constexpr int MAX_PRECISION_DIGITS = 12;

	const geom::Geometry *argGeom;

	util::TopologyException saveException;

	double distance;

	BufferParameters bufParams;

	std::unique_ptr<geom::Geometry> resultGeometry;

	bool isInvertOrientation = false;

	/**
	 * Compute a reasonable scale factor to limit the precision of
	 * a given combination of Geometry and buffer distance.
	 * The scale factor is based on a heuristic.
	 *
	 * @param g the Geometry being buffered
	 *
	 * @param distance the buffer distance
	 *
	 * @param maxPrecisionDigits the mzx # of digits that should be
	 *        allowed by the precision determined by the
	 *        computed scale factor
	 *
	 * @return a scale factor that allows a reasonable amount of
	 *         precision for the buffer computation
	 */
	static double precisionScaleFactor(const geom::Geometry *g, double distance, int maxPrecisionDigits);

	void computeGeometry();

	void bufferOriginalPrecision();

	void bufferReducedPrecision(int precisionDigits);

	void bufferReducedPrecision();

	void bufferFixedPrecision(const geom::PrecisionModel &fixedPM);

public:
	enum {
		/// Specifies a round line buffer end cap style.
		/// @deprecated use BufferParameters
		CAP_ROUND = BufferParameters::CAP_ROUND,

		/// Specifies a butt (or flat) line buffer end cap style.
		/// @deprecated use BufferParameters
		CAP_BUTT = BufferParameters::CAP_FLAT,

		/// Specifies a square line buffer end cap style.
		/// @deprecated use BufferParameters
		CAP_SQUARE = BufferParameters::CAP_SQUARE
	};

	/** \brief
	 * Computes the buffer for a geometry for a given buffer distance
	 * and accuracy of approximation.
	 *
	 * @param g the geometry to buffer
	 * @param distance the buffer distance
	 * @param quadrantSegments the number of segments used to
	 *        approximate a quarter circle
	 * @param endCapStyle line buffer end cap style (default: BufferParameters::CAP_ROUND)
	 * @return the buffer of the input geometry
	 *
	 */
	static std::unique_ptr<geom::Geometry> bufferOp(const geom::Geometry *g, double distance,
	                                                int quadrantSegments = BufferParameters::DEFAULT_QUADRANT_SEGMENTS,
	                                                int endCapStyle = BufferParameters::CAP_ROUND);

	static std::unique_ptr<geom::Geometry> bufferOp(const geom::Geometry *g, double distance,
	                                                BufferParameters &bufParms);

	/** \brief
	 * Initializes a buffer computation for the given geometry.
	 *
	 * @param g the geometry to buffer
	 */
	BufferOp(const geom::Geometry *g) : argGeom(g), bufParams(), resultGeometry(nullptr), isInvertOrientation(false) {
	}

	/** \brief
	 * Initializes a buffer computation for the given geometry
	 * with the given set of parameters.
	 *
	 * @param g the geometry to buffer
	 * @param params the buffer parameters to use. This class will
	 *               copy it to private memory.
	 */
	BufferOp(const geom::Geometry *g, const BufferParameters &params)
	    : argGeom(g), bufParams(params), resultGeometry(nullptr), isInvertOrientation(false) {
	}

	/** \brief
	 * Specifies the end cap style of the generated buffer.
	 *
	 * The styles supported are CAP_ROUND, CAP_BUTT, and CAP_SQUARE.
	 * The default is CAP_ROUND.
	 *
	 * @param nEndCapStyle the end cap style to specify
	 */
	inline void setEndCapStyle(int nEndCapStyle);

	/** \brief
	 * Sets the number of segments used to approximate a angle fillet
	 *
	 * @param nQuadrantSegments the number of segments in a fillet for a quadrant
	 */
	inline void setQuadrantSegments(int nQuadrantSegments);

	/** \brief
	 * Returns the buffer computed for a geometry for a given buffer
	 * distance.
	 *
	 * @param nDistance the buffer distance
	 * @return the buffer of the input geometry
	 */
	std::unique_ptr<geom::Geometry> getResultGeometry(double nDistance);
};

void BufferOp::setEndCapStyle(int s) {
	bufParams.setEndCapStyle((BufferParameters::EndCapStyle)s);
}

// BufferOp inlines
void BufferOp::setQuadrantSegments(int q) {
	bufParams.setQuadrantSegments(q);
}

} // namespace buffer
} // namespace operation
} // namespace geos
