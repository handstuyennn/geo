/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/index/strtree/SimpleSTRtree.hpp>
// #include <geos/index/strtree/SimpleSTRdistance.h>
#include <algorithm> // std::sort
#include <cassert>
#include <cmath>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/index/ItemVisitor.hpp>
#include <geos/util.hpp>
#include <geos/util/GEOSException.hpp>
#include <iostream> // for debugging
#include <limits>
#include <vector>

using namespace geos::geom;

namespace geos {
namespace index {   // geos.index
namespace strtree { // geos.index.strtree

/* private */
SimpleSTRnode *SimpleSTRtree::createNode(int newLevel, const geom::Envelope *itemEnv, void *item) {
	nodesQue.emplace_back(newLevel, itemEnv, item, nodeCapacity);
	SimpleSTRnode &node = nodesQue.back();
	return &node;
}

/* private */
SimpleSTRnode *SimpleSTRtree::createNode(int newLevel) {
	return createNode(newLevel, nullptr, nullptr);
}

/* private */
void SimpleSTRtree::addParentNodesFromVerticalSlice(std::vector<SimpleSTRnode *> &verticalSlice, int newLevel,
                                                    std::vector<SimpleSTRnode *> &parentNodes) {
	sortNodesY(verticalSlice);

	SimpleSTRnode *parent = nullptr;
	for (auto *node : verticalSlice) {
		if (!parent) {
			parent = createNode(newLevel);
		}
		parent->addChildNode(node);
		if (parent->size() == nodeCapacity) {
			parentNodes.push_back(parent);
			parent = nullptr;
		}
	}
	if (parent)
		parentNodes.push_back(parent);

	return;
}

/* private */
std::vector<SimpleSTRnode *> SimpleSTRtree::createHigherLevels(std::vector<SimpleSTRnode *> &nodesOfALevel, int level) {
	int nextLevel = level + 1;
	std::vector<SimpleSTRnode *> parentNodes = createParentNodes(nodesOfALevel, nextLevel);
	if (parentNodes.size() == 1) {
		return parentNodes;
	}
	return createHigherLevels(parentNodes, nextLevel);
}

/* private */
std::vector<SimpleSTRnode *> SimpleSTRtree::createParentNodes(std::vector<SimpleSTRnode *> &childNodes, int newLevel) {
	assert(!childNodes.empty());

	std::size_t minLeafCount = (std::size_t)std::ceil((double)(childNodes.size()) / (double)nodeCapacity);
	std::size_t sliceCount = (std::size_t)std::ceil(std::sqrt((double)minLeafCount));
	std::size_t sliceCapacity = (std::size_t)std::ceil((double)(childNodes.size()) / (double)sliceCount);

	sortNodesX(childNodes);

	std::size_t i = 0;
	std::size_t nChildren = childNodes.size();
	std::vector<SimpleSTRnode *> parentNodes;
	std::vector<SimpleSTRnode *> verticalSlice(sliceCapacity);
	for (std::size_t j = 0; j < sliceCount; j++) {
		verticalSlice.clear();
		std::size_t nodesAddedToSlice = 0;
		while (i < nChildren && nodesAddedToSlice < sliceCapacity) {
			verticalSlice.push_back(childNodes[i++]);
			++nodesAddedToSlice;
		}
		addParentNodesFromVerticalSlice(verticalSlice, newLevel, parentNodes);
	}
	return parentNodes;
}

/* private */
void SimpleSTRtree::build() {
	if (built)
		return;

	if (nodes.empty()) {
		root = nullptr;
	} else {
		std::vector<SimpleSTRnode *> nodeTree = createHigherLevels(nodes, 0);
		assert(nodeTree.size() == 1);
		root = nodeTree[0];
	}
	built = true;
}

/* private static */
void SimpleSTRtree::sortNodesY(std::vector<SimpleSTRnode *> &nodeList) {
	struct {
		bool operator()(SimpleSTRnode *a, SimpleSTRnode *b) const {
			const geom::Envelope &ea = a->getEnvelope();
			const geom::Envelope &eb = b->getEnvelope();
			double ya = (ea.getMinY() + ea.getMaxY()) / 2.0;
			double yb = (eb.getMinY() + eb.getMaxY()) / 2.0;
			return ya < yb;
		}
	} nodeSortByY;

	std::sort(nodeList.begin(), nodeList.end(), nodeSortByY);
}

/* private static */
void SimpleSTRtree::sortNodesX(std::vector<SimpleSTRnode *> &nodeList) {
	struct {
		bool operator()(SimpleSTRnode *a, SimpleSTRnode *b) const {
			const geom::Envelope &ea = a->getEnvelope();
			const geom::Envelope &eb = b->getEnvelope();
			double xa = (ea.getMinX() + ea.getMaxX()) / 2.0;
			double xb = (eb.getMinX() + eb.getMaxX()) / 2.0;
			return xa < xb;
		}
	} nodeSortByX;

	std::sort(nodeList.begin(), nodeList.end(), nodeSortByX);
}

/* public */
void SimpleSTRtree::insert(const geom::Envelope *itemEnv, void *item) {
	if (itemEnv->isNull())
		return;
	SimpleSTRnode *node = createNode(0, itemEnv, item);
	nodes.push_back(node);
}

/* public */
void SimpleSTRtree::query(const geom::Envelope *searchEnv, ItemVisitor &visitor) {
	build();

	if (nodes.empty() || !root) {
		return;
	}

	if (root->getEnvelope().intersects(searchEnv)) {
		query(searchEnv, root, visitor);
	}
}

/* private */
void SimpleSTRtree::query(const geom::Envelope *searchEnv, const SimpleSTRnode *node, ItemVisitor &visitor) {
	for (auto *childNode : node->getChildNodes()) {

		if (!childNode->getEnvelope().intersects(searchEnv)) {
			continue;
		}

		if (childNode->isLeaf()) {
			visitor.visitItem(childNode->getItem());
		} else {
			query(searchEnv, childNode, visitor);
		}
	}
}

/* public */
void SimpleSTRtree::query(const geom::Envelope *searchEnv, std::vector<void *> &matches) {
	build();

	if (nodes.empty() || !root) {
		return;
	}

	if (root->getEnvelope().intersects(searchEnv)) {
		query(searchEnv, root, matches);
	}
}

/* private */
void SimpleSTRtree::query(const geom::Envelope *searchEnv, const SimpleSTRnode *node, std::vector<void *> &matches) {
	assert(node);

	for (auto *childNode : node->getChildNodes()) {

		if (!childNode->getEnvelope().intersects(searchEnv)) {
			continue;
		}

		if (childNode->isLeaf()) {
			matches.push_back(childNode->getItem());
		} else {
			query(searchEnv, childNode, matches);
		}
	}
}

/* public */
bool SimpleSTRtree::remove(const geom::Envelope *searchBounds, void *item) {
	build();
	if (root->getEnvelope().intersects(searchBounds)) {
		return remove(searchBounds, root, item);
	}
	return false;
}

/* private */
bool SimpleSTRtree::remove(const geom::Envelope *searchBounds, SimpleSTRnode *node, void *item) {
	bool found = node->removeItem(item);
	if (found)
		return true;

	SimpleSTRnode *childToPrune = nullptr;
	auto childNodes = node->getChildNodes();
	for (auto *child : childNodes) {
		if (!searchBounds->intersects(child->getEnvelope())) {
			continue;
		}
		if (!child->isLeaf()) {
			found = remove(searchBounds, child, item);
			if (found) {
				childToPrune = child;
				break;
			}
		}
	}
	if (childToPrune != nullptr) {
		// Only remove empty child nodes
		if (childToPrune->getChildNodes().empty()) {
			node->removeChild(childToPrune);
		}
	}
	return found;
}

} // namespace strtree
} // namespace index
} // namespace geos
