/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>

namespace geos {
namespace geom { // geos::geom

/**
 * \brief
 * Represents a planar triangle, and provides methods for calculating various
 * properties of triangles.
 */
class GEOS_DLL Triangle {
public:
	CoordinateXY p0, p1, p2;

	Triangle(const CoordinateXY &nP0, const CoordinateXY &nP1, const CoordinateXY &nP2) : p0(nP0), p1(nP1), p2(nP2) {
	}

	/** \brief
	 * The inCentre of a triangle is the point which is equidistant
	 * from the sides of the triangle.
	 *
	 * This is also the point at which the bisectors of the angles meet.
	 *
	 * @param resultPoint the point into which to write the inCentre of the triangle
	 */
	void inCentre(CoordinateXY &resultPoint);
};

} // namespace geom
} // namespace geos
