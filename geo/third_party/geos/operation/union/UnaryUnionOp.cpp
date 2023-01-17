/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/union/UnaryUnionOp.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <algorithm> // for copy
#include <cassert>   // for assert
#include <geos/algorithm/PointLocator.hpp>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/Location.hpp>
#include <geos/geom/MultiLineString.hpp>
#include <geos/geom/MultiPoint.hpp>
#include <geos/geom/MultiPolygon.hpp>
#include <geos/geom/Point.hpp>
#include <geos/geom/util/GeometryCombiner.hpp>
#include <geos/operation/union/CascadedPolygonUnion.hpp>
#include <geos/operation/union/UnaryUnionOp.hpp>
#include <geos/operation/union/PointGeometryUnion.hpp>
#include <memory> // for unique_ptr

namespace geos {
namespace operation { // geos::operation
namespace geounion {  // geos::operation::geounion

/*private*/
std::unique_ptr<geom::Geometry> UnaryUnionOp::unionWithNull(std::unique_ptr<geom::Geometry> g0,
                                                            std::unique_ptr<geom::Geometry> g1) {
	std::unique_ptr<geom::Geometry> ret;
	if ((!g0.get()) && (!g1.get())) {
		return ret;
	}

	if (!g0.get()) {
		return g1;
	}
	if (!g1.get()) {
		return g0;
	}

	ret = g0->Union(g1.get());
	return ret;
}

/*public*/
std::unique_ptr<geom::Geometry> UnaryUnionOp::Union() {
	typedef std::unique_ptr<geom::Geometry> GeomPtr;

	GeomPtr ret;
	if (!geomFact) {
		return ret;
	}

	/*
	 * For points and lines, only a single union operation is
	 * required, since the OGC model allows self-intersecting
	 * MultiPoint and MultiLineStrings.
	 * This is not the case for polygons, so Cascaded Union is required.
	 */

	GeomPtr unionPoints;
	if (!points.empty()) {
		GeomPtr ptGeom = geomFact->buildGeometry(points.begin(), points.end());
		unionPoints = unionNoOpt(*ptGeom);
	}

	GeomPtr unionLines;
	if (!lines.empty()) {
		auto combinedLines = geomFact->buildGeometry(lines.begin(), lines.end());
		unionLines = unionNoOpt(*combinedLines);
	}

	GeomPtr unionPolygons;
	if (!polygons.empty()) {
		unionPolygons = CascadedPolygonUnion::Union(polygons.begin(), polygons.end(), unionFunction);
	}

	/*
	 * Performing two unions is somewhat inefficient,
	 * but is mitigated by unioning lines and points first
	 */

	GeomPtr unionLA = unionWithNull(std::move(unionLines), std::move(unionPolygons));

	if (!unionPoints.get()) {
		ret = std::move(unionLA);
	} else if (!unionLA.get()) {
		ret = std::move(unionPoints);
	} else {
		ret = PointGeometryUnion::Union(*unionPoints, *unionLA);
	}

	if (!ret.get()) {
		ret = geomFact->createGeometryCollection();
	}

	return ret;
}

} // namespace geounion
} // namespace operation
} // namespace geos
