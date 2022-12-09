/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: io/WKTWriter.java rev. 1.34 (JTS-1.7)
 *
 **********************************************************************/

#include <algorithm> // for min
#include <cassert>
#include <cmath>
#include <cstdio> // should avoid this
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/io/WKTWriter.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>

using namespace geos::geom;

namespace geos {
namespace io { // geos.io

/*static*/
std::string WKTWriter::toLineString(const CoordinateSequence &seq) {
	std::stringstream buf(std::ios_base::in | std::ios_base::out);
	buf << "LINESTRING ";
	auto npts = seq.size();
	if (npts == 0) {
		buf << "EMPTY";
	} else {
		buf << "(";
		for (std::size_t i = 0; i < npts; ++i) {
			if (i) {
				buf << ", ";
			}
			buf << seq.getX(i) << " " << seq.getY(i);
#if PRINT_Z
			buf << seq.getZ(i);
#endif
		}
		buf << ")";
	}

	return buf.str();
}

/*static*/
std::string WKTWriter::toLineString(const Coordinate &p0, const Coordinate &p1) {
	std::stringstream ret(std::ios_base::in | std::ios_base::out);
	ret << "LINESTRING (" << p0.x << " " << p0.y;
#if PRINT_Z
	ret << " " << p0.z;
#endif
	ret << ", " << p1.x << " " << p1.y;
#if PRINT_Z
	ret << " " << p1.z;
#endif
	ret << ")";

	return ret.str();
}

} // namespace io
} // namespace geos
