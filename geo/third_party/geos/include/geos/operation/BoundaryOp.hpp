/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/BoundaryOp.java fd5aebb
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/BoundaryNodeRule.hpp>
#include <geos/geom/Geometry.hpp>
#include <map>

namespace geos {
namespace geom {
class LineString;
class MultiLineString;
} // namespace geom
} // namespace geos

namespace geos {
namespace operation {

/**
 * Computes the boundary of a Geometry.
 * Allows specifying the BoundaryNodeRule to be used.
 * This operation will always return a Geometry of the appropriate
 * dimension for the boundary (even if the input geometry is empty).
 * The boundary of zero-dimensional geometries (Points) is
 * always the empty GeometryCollection.
 *
 * @author Martin Davis
 * @version 1.7
 */
class GEOS_DLL BoundaryOp {
public:
	/**
	 * Creates a new instance for the given geometry.
	 *
	 * @param geom the input geometry
	 */
	BoundaryOp(const geom::Geometry &geom);

	/**
	 * Gets the computed boundary.
	 *
	 * @return the boundary geometry
	 */
	std::unique_ptr<geom::Geometry> getBoundary();

private:
	const geom::Geometry &m_geom;
	const geom::GeometryFactory &m_geomFact;
	const algorithm::BoundaryNodeRule &m_bnRule;

	std::unique_ptr<geom::Geometry> boundaryMultiLineString(const geom::MultiLineString &mLine);

	std::vector<geom::Coordinate> computeBoundaryCoordinates(const geom::MultiLineString &mLine);

	std::unique_ptr<geom::Geometry> boundaryLineString(const geom::LineString &line);
};

} // namespace operation
} // namespace geos
