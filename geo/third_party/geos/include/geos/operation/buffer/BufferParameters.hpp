/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009  Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferParameters.java r378 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>

// #include <vector>

// #include <geos/algorithm/LineIntersector.h> // for composition
// #include <geos/geom/Coordinate.h> // for composition
// #include <geos/geom/LineSegment.h> // for composition

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
class PrecisionModel;
} // namespace geom
namespace operation {
namespace buffer {
class OffsetCurveVertexList;
}
} // namespace operation
} // namespace geos

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

/** \brief
 * Contains the parameters which
 * describe how a buffer should be constructed.
 *
 */
class GEOS_DLL BufferParameters {
public:
	/// End cap styles
	enum EndCapStyle {

		/// Specifies a round line buffer end cap style.
		CAP_ROUND = 1,

		/// Specifies a flat line buffer end cap style.
		CAP_FLAT = 2,

		/// Specifies a square line buffer end cap style.
		CAP_SQUARE = 3
	};

	/// Join styles
	enum JoinStyle {

		/// Specifies a round join style.
		JOIN_ROUND = 1,

		/// Specifies a mitre join style.
		JOIN_MITRE = 2,

		/// Specifies a bevel join style.
		JOIN_BEVEL = 3
	};

	/// \brief
	/// The default number of facets into which to divide a fillet
	/// of 90 degrees.
	///
	/// A value of 8 gives less than 2% max error in the buffer distance.
	/// For a max error of < 1%, use QS = 12.
	/// For a max error of < 0.1%, use QS = 18.
	///
	static const int DEFAULT_QUADRANT_SEGMENTS = 8;

	/// The default mitre limit
	///
	/// Allows fairly pointy mitres.
	///
	static const double DEFAULT_MITRE_LIMIT; // 5.0 (in .cpp file)

	/// \brief
	/// Sets the number of line segments used to approximate
	/// an angle fillet.
	///
	/// - If <tt>quadSegs</tt> >= 1, joins are round,
	///   and <tt>quadSegs</tt> indicates the number of
	///   segments to use to approximate a quarter-circle.
	/// - If <tt>quadSegs</tt> = 0, joins are bevelled (flat)
	/// - If <tt>quadSegs</tt> < 0, joins are mitred, and the value of qs
	///   indicates the mitre ration limit as
	///   <pre>
	///    mitreLimit = |<tt>quadSegs</tt>|
	///    </pre>
	///
	/// For round joins, <tt>quadSegs</tt> determines the maximum
	/// error in the approximation to the true buffer curve.
	///
	/// The default value of 8 gives less than 2% max error in the
	/// buffer distance.
	///
	/// For a max error of < 1%, use QS = 12.
	/// For a max error of < 0.1%, use QS = 18.
	/// The error is always less than the buffer distance
	/// (in other words, the computed buffer curve is always inside
	///  the true curve).
	///
	/// @param quadSegs the number of segments in a fillet for a quadrant
	void setQuadrantSegments(int quadSegs);

	/// Specifies the end cap style of the generated buffer.
	///
	/// The styles supported are CAP_ROUND, CAP_BUTT,
	/// and CAP_SQUARE.
	///
	/// The default is CAP_ROUND.
	///
	/// @param style the end cap style to specify
	///
	void setEndCapStyle(EndCapStyle style) {
		endCapStyle = style;
	}

	/// Gets the join style.
	///
	/// @return the join style
	///
	JoinStyle getJoinStyle() const {
		return joinStyle;
	}

	/// Gets the mitre ratio limit.
	///
	/// @return the limit value
	///
	double getMitreLimit() const {
		return mitreLimit;
	}

	/**
	 * Tests whether the buffer is to be generated on a single side only.
	 *
	 * @return true if the generated buffer is to be single-sided
	 */
	bool isSingleSided() const {
		return _isSingleSided;
	}

	/// Gets the end cap style.
	///
	/// @return the end cap style
	///
	EndCapStyle getEndCapStyle() const {
		return endCapStyle;
	}

	/// Gets the number of quadrant segments which will be used
	///
	/// @return the number of quadrant segments
	///
	int getQuadrantSegments() const {
		return quadrantSegments;
	}

private:
	/// Defaults to DEFAULT_QUADRANT_SEGMENTS;
	int quadrantSegments;

	/// Defaults to CAP_ROUND;
	EndCapStyle endCapStyle;

	/// Defaults to JOIN_ROUND;
	JoinStyle joinStyle;

	/// Defaults to DEFAULT_MITRE_LIMIT;
	double mitreLimit;

	bool _isSingleSided;
};

} // namespace buffer
} // namespace operation
} // namespace geos