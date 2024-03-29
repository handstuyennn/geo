/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/MCIndexNoder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/index/chain/MonotoneChain.hpp>
#include <geos/index/chain/MonotoneChainOverlapAction.hpp> // for inheritance
#include <geos/index/strtree/TemplateSTRtree.hpp>          // for composition
#include <geos/noding/NodedSegmentString.hpp>
#include <geos/noding/SinglePassNoder.hpp> // for inheritance
#include <iostream>
#include <vector>
#include <typeinfo>

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Nodes a set of SegmentString using a index based
 * on [MonotoneChain](@ref index::chain::MonotoneChain)
 * and a [SpatialIndex](@ref index::SpatialIndex).
 *
 * The [SpatialIndex](@ref index::SpatialIndex) used should be something that supports
 * envelope (range) queries efficiently (such as a [Quadtree](@ref index::quadtree::Quadtree)
 * or [STRtree](@ref index::strtree::STRtree)).
 *
 * Last port: noding/MCIndexNoder.java rev. 1.4 (JTS-1.7)
 */
class GEOS_DLL MCIndexNoder : public SinglePassNoder {
private:
	std::vector<index::chain::MonotoneChain> monoChains;
	index::strtree::TemplateSTRtree<const index::chain::MonotoneChain *> index;
	std::vector<SegmentString *> *nodedSegStrings;
	// statistics
	int nOverlaps;
	double overlapTolerance;
	bool indexBuilt;

	void intersectChains();

	void add(SegmentString *segStr);

public:
	MCIndexNoder(SegmentIntersector *nSegInt = nullptr, double p_overlapTolerance = 0.0)
	    : SinglePassNoder(nSegInt), nodedSegStrings(nullptr), nOverlaps(0), overlapTolerance(p_overlapTolerance),
	      indexBuilt(false) {
	}

	~MCIndexNoder() override {};

	void computeNodes(std::vector<SegmentString *> *inputSegmentStrings) override;

	class SegmentOverlapAction : public index::chain::MonotoneChainOverlapAction {
	public:
		SegmentOverlapAction(SegmentIntersector &newSi) : index::chain::MonotoneChainOverlapAction(), si(newSi) {
		}

		void overlap(const index::chain::MonotoneChain &mc1, std::size_t start1, const index::chain::MonotoneChain &mc2,
		             std::size_t start2) override;

	private:
		SegmentIntersector &si;

		// Declare type as noncopyable
		SegmentOverlapAction(const SegmentOverlapAction &other) = delete;
		SegmentOverlapAction &operator=(const SegmentOverlapAction &rhs) = delete;
	};

	std::vector<SegmentString *> *getNodedSubstrings() const override {
		assert(nodedSegStrings); // must have called computeNodes before!
		return NodedSegmentString::getNodedSubstrings(*nodedSegStrings);
	}
};

} // namespace noding
} // namespace geos
