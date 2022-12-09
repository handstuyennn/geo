/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <array>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.hpp>
#include <geos/algorithm/locate/PointOnGeometryLocator.hpp>
#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/Location.hpp>

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Manages the input geometries for an overlay operation.
 * The second geometry is allowed to be null,
 * to support for instance precision reduction.
 *
 * @author Martin Davis
 *
 */

using namespace geos::algorithm::locate;
using namespace geos::geom;

class GEOS_DLL InputGeometry {

private:
	// Members
	std::array<const Geometry *, 2> geom;
	std::unique_ptr<PointOnGeometryLocator> ptLocatorA;
	std::unique_ptr<PointOnGeometryLocator> ptLocatorB;
	std::array<bool, 2> isCollapsed;

public:
	InputGeometry(const Geometry *geomA, const Geometry *geomB);

	bool isSingle() const;
	int getDimension(uint8_t index) const;
	const Geometry *getGeometry(uint8_t geomIndex) const;
	const Envelope *getEnvelope(uint8_t geomIndex) const;
	bool isEmpty(uint8_t geomIndex) const;
	bool isArea(uint8_t geomIndex) const;
	int getAreaIndex() const;
	bool isLine(uint8_t geomIndex) const;
	bool isAllPoints() const;
	bool hasPoints() const;

	/**
	 * Tests if an input geometry has edges.
	 * This indicates that topology needs to be computed for them.
	 *
	 * @param geomIndex
	 * @return true if the input geometry has edges
	 */
	bool hasEdges(uint8_t geomIndex) const;

	/**
	 * Determines the location within an area geometry.
	 * This allows disconnected edges to be fully
	 * located.
	 *
	 * @param geomIndex the index of the geometry
	 * @param pt the coordinate to locate
	 * @return the location of the coordinate
	 *
	 * @see Location
	 */
	Location locatePointInArea(uint8_t geomIndex, const Coordinate &pt);

	PointOnGeometryLocator *getLocator(uint8_t geomIndex);
	void setCollapsed(uint8_t geomIndex, bool isGeomCollapsed);
};

} // namespace overlayng
} // namespace operation
} // namespace geos
