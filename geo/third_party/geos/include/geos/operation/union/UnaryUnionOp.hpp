/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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

#pragma once

#include <geos/export.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/Point.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geom/util/GeometryExtracter.hpp>
#include <geos/operation/overlay/OverlayOp.hpp>
#include <geos/operation/union/CascadedPolygonUnion.hpp>
#include <memory>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class Geometry;
} // namespace geom
} // namespace geos

namespace geos {
namespace operation { // geos::operation
namespace geounion {  // geos::operation::geounion

/** \brief
 * Unions a collection of Geometry or a single Geometry
 * (which may be a collection) together.
 *
 * By using this special-purpose operation over a collection of
 * geometries it is possible to take advantage of various optimizations
 * to improve performance.
 * Heterogeneous [GeometryCollections](@ref geom::GeometryCollection)
 * are fully supported.
 *
 * The result obeys the following contract:
 *
 * - Unioning a set of overlapping [Polygons](@ref geom::Polygon) has the effect
 *   of merging the areas (i.e. the same effect as
 *   iteratively unioning all individual polygons together).
 * - Unioning a set of [LineStrings](@ref geom::LineString) has the effect of
 *   **fully noding** and **dissolving** the input linework.
 *   In this context "fully noded" means that there will be a node or
 *   endpoint in the output for every endpoint or line segment crossing
 *   in the input.
 *   "Dissolved" means that any duplicate (e.g. coincident) line segments
 *   or portions of line segments will be reduced to a single line segment
 *   in the output.
 *   This is consistent with the semantics of the
 *   [Geometry::Union(Geometry* )](@ref geom::Geometry::Union())
 *   operation. If **merged** linework is required, the
 *   [LineMerger](@ref operation::linemerge::LineMerger) class
 *   can be used.
 * - Unioning a set of [Points](@ref geom::Point) has the effect of merging
 *   all identical points (producing a set with no duplicates).
 *
 * `UnaryUnion` always operates on the individual components of
 * MultiGeometries.
 * So it is possible to use it to "clean" invalid self-intersecting
 * MultiPolygons (although the polygon components must all still be
 * individually valid.)
 */
class GEOS_DLL UnaryUnionOp {
public:
	template <typename T>
	static std::unique_ptr<geom::Geometry> Union(const T &geoms) {
		UnaryUnionOp op(geoms);
		return op.Union();
	}

	template <class T>
	static std::unique_ptr<geom::Geometry> Union(const T &geoms, geom::GeometryFactory &geomFact) {
		UnaryUnionOp op(geoms, geomFact);
		return op.Union();
	}

	static std::unique_ptr<geom::Geometry> Union(const geom::Geometry &geom) {
		UnaryUnionOp op(geom);
		return op.Union();
	}

	template <class T>
	UnaryUnionOp(const T &geoms, geom::GeometryFactory &geomFactIn)
	    : geomFact(&geomFactIn), unionFunction(&defaultUnionFunction) {
		extractGeoms(geoms);
	}

	template <class T>
	UnaryUnionOp(const T &geoms) : geomFact(nullptr), unionFunction(&defaultUnionFunction) {
		extractGeoms(geoms);
	}

	UnaryUnionOp(const geom::Geometry &geom) : geomFact(geom.getFactory()), unionFunction(&defaultUnionFunction) {
		extract(geom);
	}

	void setUnionFunction(UnionStrategy *unionFun) {
		unionFunction = unionFun;
	}

	/**
	 * \brief
	 * Gets the union of the input geometries.
	 *
	 * If no input geometries were provided, an empty GEOMETRYCOLLECTION is returned.
	 *
	 * @return a Geometry containing the union
	 * @return an empty GEOMETRYCOLLECTION if no geometries were provided
	 *         in the input
	 */
	std::unique_ptr<geom::Geometry> Union();

private:
	void extract(const geom::Geometry &geom) {
		using namespace geom::util;

		if (!geomFact) {
			geomFact = geom.getFactory();
		}

		GeometryExtracter::extract<geom::Polygon>(geom, polygons);
		GeometryExtracter::extract<geom::LineString>(geom, lines);
		GeometryExtracter::extract<geom::Point>(geom, points);
	}

	/**
	 * Computes a unary union with no extra optimization,
	 * and no short-circuiting.
	 * Due to the way the overlay operations
	 * are implemented, this is still efficient in the case of linear
	 * and puntal geometries.
	 * Uses robust version of overlay operation
	 * to ensure identical behaviour to the <tt>union(Geometry)</tt> operation.
	 *
	 * @param g0 a geometry
	 * @return the union of the input geometry
	 */
	std::unique_ptr<geom::Geometry> unionNoOpt(const geom::Geometry &g0) {
		using geos::operation::overlay::OverlayOp;

		if (!empty.get()) {
			empty = geomFact->createEmptyGeometry();
		}
		return unionFunction->Union(&g0, empty.get());
	}

	/**
	 * Computes the union of two geometries,
	 * either of both of which may be null.
	 *
	 * @param g0 a Geometry (ownership transferred)
	 * @param g1 a Geometry (ownership transferred)
	 * @return the union of the input(s)
	 * @return null if both inputs are null
	 */
	std::unique_ptr<geom::Geometry> unionWithNull(std::unique_ptr<geom::Geometry> g0,
	                                              std::unique_ptr<geom::Geometry> g1);

	// Members
	std::vector<const geom::Polygon *> polygons;
	std::vector<const geom::LineString *> lines;
	std::vector<const geom::Point *> points;

	const geom::GeometryFactory *geomFact;
	std::unique_ptr<geom::Geometry> empty;

	UnionStrategy *unionFunction;
	ClassicUnionStrategy defaultUnionFunction;
};

} // namespace geounion
} // namespace operation
} // namespace geos
