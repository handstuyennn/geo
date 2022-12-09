/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
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
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateFilter.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <set>
#include <vector>

namespace geos {
namespace util { // geos::util

/*
 *  A CoordinateFilter that fills a vector of Coordinate const pointers.
 *  The set of coordinates contains no duplicate points.
 *
 *  Last port: util/UniqueCoordinateArrayFilter.java rev. 1.17
 */
class GEOS_DLL UniqueCoordinateArrayFilter : public geom::CoordinateFilter {
public:
	/**
	 * Constructs a CoordinateArrayFilter.
	 *
	 * @param target The destination set.
	 */
	UniqueCoordinateArrayFilter(geom::Coordinate::ConstVect &target) : pts(target) {
	}

	/**
	 * Destructor.
	 * Virtual dctor promises appropriate behaviour when someone will
	 * delete a derived-class object via a base-class pointer.
	 * http://www.parashift.com/c++-faq-lite/virtual-functions.html#faq-20.7
	 */
	~UniqueCoordinateArrayFilter() override {
	}

	/**
	 * Performs a filtering operation with or on coord in "read-only" mode.
	 * @param coord The "read-only" Coordinate to which
	 * 				the filter is applied.
	 */
	void filter_ro(const geom::Coordinate *coord) override {
		if (uniqPts.insert(coord).second) {
			pts.push_back(coord);
		}
	}

private:
	geom::Coordinate::ConstVect &pts;   // target set reference
	geom::Coordinate::ConstSet uniqPts; // unique points set

	// Declare type as noncopyable
	UniqueCoordinateArrayFilter(const UniqueCoordinateArrayFilter &other) = delete;
	UniqueCoordinateArrayFilter &operator=(const UniqueCoordinateArrayFilter &rhs) = delete;
};

} // namespace util
} // namespace geos