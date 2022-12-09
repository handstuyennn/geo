/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009    Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/OrientedCoordinateArray.java rev. 1.1 (JTS-1.9)
 *
 **********************************************************************/

#pragma once

#include <cstddef>
#include <geos/export.hpp>

// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
}
namespace noding {
// class SegmentString;
}
} // namespace geos

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Allows comparing {@link geom::CoordinateSequence}s
 * in an orientation-independent way.
 */
class GEOS_DLL OrientedCoordinateArray {
public:
	/**
	 * Creates a new {@link OrientedCoordinateArray}
	 * for the given {@link geom::CoordinateSequence}.
	 *
	 * @param p_pts the coordinates to orient
	 */
	OrientedCoordinateArray(const geom::CoordinateSequence &p_pts) : pts(&p_pts), orientationVar(orientation(p_pts)) {
	}

	bool operator==(const OrientedCoordinateArray &other) const;

	struct GEOS_DLL HashCode {
		std::size_t operator()(const OrientedCoordinateArray &oca) const;
	};

private:
	/**
	 * Computes the canonical orientation for a coordinate array.
	 *
	 * @param pts the array to test
	 * @return <code>true</code> if the points are oriented forwards
	 * @return <code>false</code> if the points are oriented in reverse
	 */
	static bool orientation(const geom::CoordinateSequence &pts);

	/// Externally owned
	const geom::CoordinateSequence *pts;

	bool orientationVar;
};
} // namespace noding
} // namespace geos
