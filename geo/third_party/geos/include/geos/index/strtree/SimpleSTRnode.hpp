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

#pragma once

#include <cassert>
#include <geos/export.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/index/strtree/ItemBoundable.hpp>
#include <vector>

namespace geos {
namespace index {   // geos::index
namespace strtree { // geos::index::strtree

/** \brief
 * A node of the STR tree.
 *
 */
class GEOS_DLL SimpleSTRnode : public ItemBoundable {
private:
	std::vector<SimpleSTRnode *> childNodes;
	void *item;
	geom::Envelope bounds;
	std::size_t level;

public:
	/*
	 * Constructs an AbstractNode at the given level in the tree
	 */
	SimpleSTRnode(std::size_t newLevel, const geom::Envelope *p_env, void *p_item, std::size_t capacity = 10)
	    : ItemBoundable(p_env, p_item), item(p_item), bounds(), level(newLevel) {
		childNodes.reserve(capacity);
		if (p_env) {
			bounds = *p_env;
		}
	}

	SimpleSTRnode(std::size_t newLevel) : SimpleSTRnode(newLevel, nullptr, nullptr) {
	}

	/**
	 * Returns a representation of space that encloses this Node
	 */
	const inline geom::Envelope &getEnvelope() const {
		return bounds;
	}

	std::size_t size() const {
		return childNodes.size();
	}

	const std::vector<SimpleSTRnode *> &getChildNodes() const {
		return childNodes;
	}

	/**
	 * Adds either an AbstractNode, or if this is a leaf node, a data object
	 * (wrapped in an ItemBoundable)
	 */
	void addChildNode(SimpleSTRnode *childNode);

	bool isLeaf() const override {
		return level == 0;
	}

	void *getItem() const {
		return item;
	}

	bool removeItem(void *item);
	bool removeChild(SimpleSTRnode *child);
};

} // namespace strtree
} // namespace index
} // namespace geos
