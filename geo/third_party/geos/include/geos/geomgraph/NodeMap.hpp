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
 * Last port: geomgraph/NodeMap.java rev. 1.3 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp> // for CoordinateLessThen
#include <geos/geomgraph/Node.hpp>  // for testInvariant
#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
namespace geos {
namespace geomgraph {
class Node;
class EdgeEnd;
class NodeFactory;
} // namespace geomgraph
} // namespace geos

namespace geos {
namespace geomgraph { // geos.geomgraph

class GEOS_DLL NodeMap {
public:
	typedef std::map<geom::Coordinate *, std::unique_ptr<Node>, geom::CoordinateLessThen> container;

	typedef container::iterator iterator;

	typedef container::const_iterator const_iterator;

	container nodeMap;

	const NodeFactory &nodeFact;

	/// \brief
	/// NodeMap will keep a reference to the NodeFactory,
	/// keep it alive for the whole NodeMap lifetime
	NodeMap(const NodeFactory &newNodeFact);

	virtual ~NodeMap();

	Node *addNode(const geom::Coordinate &coord);

	Node *addNode(Node *n);

	/// \brief
	/// Adds a node for the start point of this EdgeEnd
	/// (if one does not already exist in this map).
	/// Adds the EdgeEnd to the (possibly new) node.
	///
	/// If ownership of the EdgeEnd should be transferred
	/// to the Node, use the unique_ptr overload instead.
	void add(EdgeEnd *e);

	void add(std::unique_ptr<EdgeEnd> &&e);

	Node *find(const geom::Coordinate &coord) const;

	const_iterator begin() const {
		return nodeMap.begin();
	}

	const_iterator end() const {
		return nodeMap.end();
	}

	iterator begin() {
		return nodeMap.begin();
	}

	iterator end() {
		return nodeMap.end();
	}

	void getBoundaryNodes(uint8_t geomIndex, std::vector<Node *> &bdyNodes) const;

private:
	// Declare type as noncopyable
	NodeMap(const NodeMap &other) = delete;
	NodeMap &operator=(const NodeMap &rhs) = delete;
};

} // namespace geomgraph
} // namespace geos