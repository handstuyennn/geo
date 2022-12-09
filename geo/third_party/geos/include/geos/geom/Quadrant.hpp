/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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
 * Last port: geom/Quadrant.java rev. 1.8 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Quadrant.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <sstream>
#include <string>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
} // namespace geos

namespace geos {
namespace geom { // geos.geom

/** \brief
 * Utility functions for working with quadrants.
 *
 * The quadrants are numbered as follows:
 * <pre>
 * 1 | 0
 * --+--
 * 2 | 3
 * </pre>
 *
 */
class GEOS_DLL Quadrant {
public:
	static const int NE = 0;
	static const int NW = 1;
	static const int SW = 2;
	static const int SE = 3;

	/**
	 * Returns the quadrant of a directed line segment
	 * (specified as x and y displacements, which cannot both be 0).
	 *
	 * @throws IllegalArgumentException if the displacements are both 0
	 */
	static int quadrant(double dx, double dy) {
		if (dx == 0.0 && dy == 0.0) {
			std::ostringstream s;
			s << "Cannot compute the quadrant for point ";
			s << "(" << dx << "," << dy << ")" << std::endl;
			throw util::IllegalArgumentException(s.str());
		}
		if (dx >= 0) {
			if (dy >= 0) {
				return NE;
			} else {
				return SE;
			}
		} else {
			if (dy >= 0) {
				return NW;
			} else {
				return SW;
			}
		}
	};

	/**
	 * Returns the quadrant of a directed line segment from p0 to p1.
	 *
	 * @throws IllegalArgumentException if the points are equal
	 */
	static int quadrant(const geom::Coordinate &p0, const geom::Coordinate &p1) {
		if (p1.x == p0.x && p1.y == p0.y) {
			throw util::IllegalArgumentException("Cannot compute the quadrant for two identical points ");
		}

		if (p1.x >= p0.x) {
			if (p1.y >= p0.y) {
				return NE;
			} else {
				return SE;
			}
		} else {
			if (p1.y >= p0.y) {
				return NW;
			} else {
				return SW;
			}
		}
	};

	/**
	 * Returns true if the given quadrant is 0 or 1.
	 */
	static bool isNorthern(int quad) {
		return quad == NE || quad == NW;
	};
};

} // namespace geom
} // namespace geos
