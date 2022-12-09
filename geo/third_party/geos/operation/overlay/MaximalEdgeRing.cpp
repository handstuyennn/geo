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
 * Last port: operation/overlay/MaximalEdgeRing.java rev. 1.15 (JTS-1.10)
 *
 **********************************************************************/

#include <cassert>
#include <geos/geomgraph/DirectedEdge.hpp>
#include <geos/geomgraph/DirectedEdgeStar.hpp>
#include <geos/geomgraph/EdgeEndStar.hpp>
#include <geos/geomgraph/EdgeRing.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/operation/overlay/MaximalEdgeRing.hpp>
#include <geos/operation/overlay/MinimalEdgeRing.hpp>
#include <geos/util.hpp>
#include <vector>

using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay {   // geos.operation.overlay

/*public*/
MaximalEdgeRing::MaximalEdgeRing(DirectedEdge *start, const GeometryFactory *p_geometryFactory)
    // throw(const TopologyException &)
    : EdgeRing(start, p_geometryFactory) {
	computePoints(start);
	computeRing();
}

/*public*/
DirectedEdge *MaximalEdgeRing::getNext(DirectedEdge *de) {
	return de->getNext();
}

/*public*/
void MaximalEdgeRing::setEdgeRing(DirectedEdge *de, EdgeRing *er) {
	de->setEdgeRing(er);
}

/*public*/
void MaximalEdgeRing::linkDirectedEdgesForMinimalEdgeRings() {
	DirectedEdge *de = startDe;
	do {
		Node *node = de->getNode();
		EdgeEndStar *ees = node->getEdges();

		DirectedEdgeStar *des = detail::down_cast<DirectedEdgeStar *>(ees);

		des->linkMinimalDirectedEdges(this);

		de = de->getNext();

	} while (de != startDe);
}

/*public*/
std::vector<MinimalEdgeRing *> *MaximalEdgeRing::buildMinimalRings() {
	std::vector<MinimalEdgeRing *> *minEdgeRings = new std::vector<MinimalEdgeRing *>;
	buildMinimalRings(*minEdgeRings);
	return minEdgeRings;
}

/*public*/
void MaximalEdgeRing::buildMinimalRings(std::vector<MinimalEdgeRing *> &minEdgeRings) {
	DirectedEdge *de = startDe;
	do {
		if (de->getMinEdgeRing() == nullptr) {
			MinimalEdgeRing *minEr = new MinimalEdgeRing(de, geometryFactory);
			minEdgeRings.push_back(minEr);
		}
		de = de->getNext();
	} while (de != startDe);
}

/*public*/
void MaximalEdgeRing::buildMinimalRings(std::vector<EdgeRing *> &minEdgeRings) {
	DirectedEdge *de = startDe;
	do {
		if (de->getMinEdgeRing() == nullptr) {
			MinimalEdgeRing *minEr = new MinimalEdgeRing(de, geometryFactory);
			minEdgeRings.push_back(minEr);
		}
		de = de->getNext();
	} while (de != startDe);
}

} // namespace overlay
} // namespace operation
} // namespace geos
