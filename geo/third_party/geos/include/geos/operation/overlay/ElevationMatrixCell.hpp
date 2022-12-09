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
 ***********************************************************************
 *
 * Last port: original (by strk)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <set>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
} // namespace geos

namespace geos {
namespace operation { // geos::operation
namespace overlay {   // geos::operation::overlay

class GEOS_DLL ElevationMatrixCell {
public:
	ElevationMatrixCell();
	~ElevationMatrixCell() = default;
	void add(const geom::Coordinate &c);
	void add(double z);
	double getAvg(void) const;
	double getTotal(void) const;
	std::string print() const;

private:
	std::set<double> zvals;
	double ztot;
};

} // namespace overlay
} // namespace operation
} // namespace geos
