/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2010  Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/GeometrySnapper.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.hpp>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
// class PrecisionModel;
class Geometry;
class CoordinateSequence;
} // namespace geom
} // namespace geos

namespace geos {
namespace operation { // geos::operation
namespace overlay {   // geos::operation::overlay
namespace snap {      // geos::operation::overlay::snap

/** \brief
 * Snaps the vertices and segments of a {@link geom::Geometry}
 * to another Geometry's vertices.
 *
 * A snap distance tolerance is used to control where snapping is performed.
 * Snapping one geometry to another can improve
 * robustness for overlay operations by eliminating
 * nearly-coincident edges
 * (which cause problems during noding and intersection calculation).
 * Too much snapping can result in invalid topology
 * being created, so the number and location of snapped vertices
 * is decided using heuristics to determine when it
 * is safe to snap.
 * This can result in some potential snaps being omitted, however.
 */
class GEOS_DLL GeometrySnapper {
public:
	/**
	 * Creates a new snapper acting on the given geometry
	 *
	 * @param g the geometry to snap
	 */
	GeometrySnapper(const geom::Geometry &g) : srcGeom(g) {
	}

	/** \brief
	 * Estimates the snap tolerance for a Geometry, taking into account
	 * its precision model.
	 *
	 * @param g a Geometry
	 * @return the estimated snap tolerance
	 */
	static double computeOverlaySnapTolerance(const geom::Geometry &g);

	static double computeSizeBasedSnapTolerance(const geom::Geometry &g);

	/** \brief
	 * Computes the snap tolerance based on input geometries;
	 */
	static double computeOverlaySnapTolerance(const geom::Geometry &g1, const geom::Geometry &g2);

	/** \brief
	 * Snaps the vertices in the component {@link geom::LineString}s
	 * of the source geometry to the vertices of the given snap geometry
	 * with a given snap tolerance
	 *
	 * @param g a geometry to snap the source to
	 * @param snapTolerance
	 * @return a new snapped Geometry
	 */
	std::unique_ptr<geom::Geometry> snapTo(const geom::Geometry &g, double snapTolerance);

private:
	// eventually this will be determined from the geometry topology
	// static const double snapTol; //  = 0.000001;

	static const double snapPrecisionFactor; //  = 10e-10

	const geom::Geometry &srcGeom;

	/// Extract target (unique) coordinates
	std::unique_ptr<geom::Coordinate::ConstVect> extractTargetCoordinates(const geom::Geometry &g);
};

} // namespace snap
} // namespace overlay
} // namespace operation
} // namespace geos
