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

#include <geos/export.hpp>
#include <geos/geom/Geometry.hpp> // to be removed when we have the .inl
#include <geos/geom/GeometryComponentFilter.hpp>
#include <geos/geom/LineString.hpp> // to be removed when we have the .inl
#include <vector>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * Extracts all the 1-dimensional (LineString) components from a Geometry.
 */
class GEOS_DLL LinearComponentExtracter : public GeometryComponentFilter {
private:
	LineString::ConstVect &comps;

public:
	/**
	 * Push the linear components from a single geometry into
	 * the provided vector.
	 * If more than one geometry is to be processed, it is more
	 * efficient to create a single LinearComponentExtracterFilter instance
	 * and pass it to multiple geometries.
	 */
	static void getLines(const Geometry &geom, std::vector<const LineString *> &ret);

	/**
	 * Constructs a LinearComponentExtracterFilter with a list in which
	 * to store LineStrings found.
	 */
	LinearComponentExtracter(std::vector<const LineString *> &newComps);

	void filter_rw(Geometry *geom) override;

	void filter_ro(const Geometry *geom) override;
};

} // namespace util
} // namespace geom
} // namespace geos
