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
 **********************************************************************
 *
 * Last port: operation/overlay/MinimalEdgeRing.java rev. 1.13 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geomgraph/DirectedEdge.hpp> // for inlines
#include <geos/geomgraph/EdgeRing.hpp>     // for inheritance
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
}
namespace geomgraph {
class DirectedEdge;
class EdgeRing;
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace operation { // geos::operation
namespace overlay {   // geos::operation::overlay

/** \brief
 * A ring of [Edges](@ref geomgraph::Edge) with the property that no node
 * has degree greater than 2.
 *
 * These are the form of rings required to represent polygons
 * under the OGC SFS spatial data model.
 *
 * @see operation::overlay::MaximalEdgeRing
 *
 */
class GEOS_DLL MinimalEdgeRing : public geomgraph::EdgeRing {
public:
	MinimalEdgeRing(geomgraph::DirectedEdge *start, const geom::GeometryFactory *geometryFactory);

	~MinimalEdgeRing() override {};

	geomgraph::DirectedEdge *getNext(geomgraph::DirectedEdge *de) override {
		return de->getNextMin();
	};

	void setEdgeRing(geomgraph::DirectedEdge *de, geomgraph::EdgeRing *er) override {
		de->setMinEdgeRing(er);
	};
};
} // namespace overlay
} // namespace operation
} // namespace geos
