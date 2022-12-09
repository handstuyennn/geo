/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/BufferSubgraph.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <cassert>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/Position.hpp>
#include <geos/geomgraph/DirectedEdge.hpp>
#include <geos/geomgraph/DirectedEdgeStar.hpp>
#include <geos/geomgraph/Edge.hpp>
#include <geos/geomgraph/EdgeEndStar.hpp>
#include <geos/geomgraph/Node.hpp>
#include <geos/operation/buffer/BufferSubgraph.hpp>
#include <geos/util.hpp>
#include <geos/util/TopologyException.hpp>
#include <iostream>
#include <list>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::operation;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer {    // geos.operation.buffer

// Argument is unused
BufferSubgraph::BufferSubgraph() : finder(), dirEdgeList(), nodes(), rightMostCoord(nullptr), env(nullptr) {
}

BufferSubgraph::~BufferSubgraph() {
	delete env;
}

/*public*/
void BufferSubgraph::computeDepth(int outsideDepth) {
	clearVisitedEdges();
	// find an outside edge to assign depth to
	DirectedEdge *de = finder.getEdge();
	// Node *n=de->getNode();
	// Label *label=de->getLabel();

	// right side of line returned by finder is on the outside
	de->setEdgeDepths(Position::RIGHT, outsideDepth);
	copySymDepths(de);

	// computeNodeDepth(n, de);
	computeDepths(de);
}

/*private*/
void BufferSubgraph::copySymDepths(DirectedEdge *de) {
	DirectedEdge *sym = de->getSym();
	sym->setDepth(Position::LEFT, de->getDepth(Position::RIGHT));
	sym->setDepth(Position::RIGHT, de->getDepth(Position::LEFT));
}

/*private*/
void BufferSubgraph::computeDepths(DirectedEdge *startEdge) {
	std::set<Node *> nodesVisited;
	std::list<Node *> nodeQueue; // Used to be a vector
	Node *startNode = startEdge->getNode();
	nodeQueue.push_back(startNode);
	// nodesVisited.push_back(startNode);
	nodesVisited.insert(startNode);
	startEdge->setVisited(true);

	while (!nodeQueue.empty()) {
		// System.out.println(nodes.size() + " queue: " + nodeQueue.size());
		Node *n = nodeQueue.front(); // [0];
		// nodeQueue.erase(nodeQueue.begin());
		nodeQueue.pop_front();

		nodesVisited.insert(n);

		// compute depths around node, starting at this edge since it has depths assigned
		computeNodeDepth(n);

		// add all adjacent nodes to process queue,
		// unless the node has been visited already
		EdgeEndStar *ees = n->getEdges();
		EdgeEndStar::iterator endIt = ees->end();
		EdgeEndStar::iterator it = ees->begin();
		for (; it != endIt; ++it) {
			DirectedEdge *de = detail::down_cast<DirectedEdge *>(*it);
			DirectedEdge *sym = de->getSym();
			if (sym->isVisited()) {
				continue;
			}
			Node *adjNode = sym->getNode();

			// if (! contains(nodesVisited,adjNode))
			if (nodesVisited.insert(adjNode).second) {
				nodeQueue.push_back(adjNode);
				// nodesVisited.insert(adjNode);
			}
		}
	}
}

void BufferSubgraph::computeNodeDepth(Node *n)
// throw(TopologyException *)
{
	// find a visited dirEdge to start at
	DirectedEdge *startEdge = nullptr;

	DirectedEdgeStar *ees = detail::down_cast<DirectedEdgeStar *>(n->getEdges());

	EdgeEndStar::iterator endIt = ees->end();

	EdgeEndStar::iterator it = ees->begin();
	for (; it != endIt; ++it) {
		DirectedEdge *de = detail::down_cast<DirectedEdge *>(*it);
		if (de->isVisited() || de->getSym()->isVisited()) {
			startEdge = de;
			break;
		}
	}
	// MD - testing  Result: breaks algorithm
	// if (startEdge==null) return;

	// only compute string append if assertion would fail
	if (startEdge == nullptr) {
		throw util::TopologyException("unable to find edge to compute depths at", n->getCoordinate());
	}

	ees->computeDepths(startEdge);

	// copy depths to sym edges
	for (it = ees->begin(); it != endIt; ++it) {
		DirectedEdge *de = detail::down_cast<DirectedEdge *>(*it);
		de->setVisited(true);
		copySymDepths(de);
	}
}

/*private*/
void BufferSubgraph::clearVisitedEdges() {
	for (std::size_t i = 0, n = dirEdgeList.size(); i < n; ++i) {
		DirectedEdge *de = dirEdgeList[i];
		de->setVisited(false);
	}
}

/*public*/
void BufferSubgraph::create(Node *node) {
	addReachable(node);

	// We are assuming that dirEdgeList
	// contains *at leas* ONE forward DirectedEdge
	finder.findEdge(&dirEdgeList);

	rightMostCoord = &(finder.getCoordinate());

	// this is what happen if no forward DirectedEdge
	// is passed to the RightmostEdgeFinder
	assert(rightMostCoord);
}

/*private*/
void BufferSubgraph::addReachable(Node *startNode) {
	std::vector<Node *> nodeStack;
	nodeStack.push_back(startNode);
	while (!nodeStack.empty()) {
		Node *node = nodeStack.back();
		nodeStack.pop_back();
		add(node, &nodeStack);
	}
}

/*private*/
void BufferSubgraph::add(Node *node, std::vector<Node *> *nodeStack) {
	node->setVisited(true);
	nodes.push_back(node);
	EdgeEndStar *ees = node->getEdges();
	EdgeEndStar::iterator it = ees->begin();
	EdgeEndStar::iterator endIt = ees->end();
	for (; it != endIt; ++it) {
		DirectedEdge *de = detail::down_cast<DirectedEdge *>(*it);
		dirEdgeList.push_back(de);
		DirectedEdge *sym = de->getSym();
		Node *symNode = sym->getNode();
		/*
		 * NOTE: this is a depth-first traversal of the graph.
		 * This will cause a large depth of recursion.
		 * It might be better to do a breadth-first traversal.
		 */
		if (!symNode->isVisited()) {
			nodeStack->push_back(symNode);
		}
	}
}

/*public*/
int BufferSubgraph::compareTo(BufferSubgraph *graph) {
	assert(rightMostCoord);
	if (rightMostCoord->x < graph->rightMostCoord->x) {
		return -1;
	}
	if (rightMostCoord->x > graph->rightMostCoord->x) {
		return 1;
	}
	return 0;
}

/*public*/
Envelope *BufferSubgraph::getEnvelope() {
	if (env == nullptr) {
		env = new Envelope();
		std::size_t const size = dirEdgeList.size();
		for (std::size_t i = 0; i < size; ++i) {
			DirectedEdge *dirEdge = dirEdgeList[i];
			const CoordinateSequence *pts = dirEdge->getEdge()->getCoordinates();
			std::size_t const n = pts->getSize() - 1;
			for (std::size_t j = 0; j < n; ++j) {
				env->expandToInclude(pts->getAt(j));
			}
		}
	}
	return env;
}

/*public*/
void BufferSubgraph::findResultEdges() {
	for (std::size_t i = 0, n = dirEdgeList.size(); i < n; ++i) {
		DirectedEdge *de = dirEdgeList[i];

		/*
		 * Select edges which have an interior depth on the RHS
		 * and an exterior depth on the LHS.
		 * Note that because of weird rounding effects there may be
		 * edges which have negative depths!  Negative depths
		 * count as "outside".
		 */
		// <FIX> - handle negative depths
		if (de->getDepth(Position::RIGHT) >= 1 && de->getDepth(Position::LEFT) <= 0 && !de->isInteriorAreaEdge()) {
			de->setInResult(true);
		}
	}
}

} // namespace buffer
} // namespace operation
} // namespace geos