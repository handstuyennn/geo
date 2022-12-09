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
 * Last port: geom/Envelope.java rev 1.46 (JTS-1.10)
 *
 **********************************************************************/

#include <algorithm>
#include <cmath>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Envelope.hpp>
#include <sstream>

namespace geos {
namespace geom { // geos::geom

/* public */
std::ostream &operator<<(std::ostream &os, const Envelope &o) {
	os << "Env[" << o.minx << ":" << o.maxx << "," << o.miny << ":" << o.maxy << "]";
	return os;
}

/*public*/
bool Envelope::intersects(const CoordinateXY &p1, const CoordinateXY &p2, const CoordinateXY &q) {
	// OptimizeIt shows that Math#min and Math#max here are a bottleneck.
	// Replace with direct comparisons. [Jon Aquino]
	if (((q.x >= (p1.x < p2.x ? p1.x : p2.x)) && (q.x <= (p1.x > p2.x ? p1.x : p2.x))) &&
	    ((q.y >= (p1.y < p2.y ? p1.y : p2.y)) && (q.y <= (p1.y > p2.y ? p1.y : p2.y)))) {
		return true;
	}
	return false;
}

/*public*/
bool Envelope::intersects(const CoordinateXY &a, const CoordinateXY &b) const {
	// These comparisons look redundant, but an alternative using
	// std::minmax performs no better and compiles down to more
	// instructions.
	double envminx = (a.x < b.x) ? a.x : b.x;
	if (!(maxx >= envminx)) { // awkward comparison catches cases where this->isNull()
		return false;
	}

	double envmaxx = (a.x > b.x) ? a.x : b.x;
	if (envmaxx < minx) {
		return false;
	}

	double envminy = (a.y < b.y) ? a.y : b.y;
	if (envminy > maxy) {
		return false;
	}

	double envmaxy = (a.y > b.y) ? a.y : b.y;
	if (envmaxy < miny) {
		return false;
	}

	return true;
}

/*public*/
void Envelope::expandBy(double deltaX, double deltaY) {
	minx -= deltaX;
	maxx += deltaX;
	miny -= deltaY;
	maxy += deltaY;

	// check for envelope disappearing
	if (minx > maxx || miny > maxy) {
		setToNull();
	}
}

/*public*/
bool Envelope::covers(const Envelope &other) const {
	return other.minx >= minx && other.maxx <= maxx && other.miny >= miny && other.maxy <= maxy;
}

/*public*/
bool Envelope::intersection(const Envelope &env, Envelope &result) const {
	if (isNull() || env.isNull() || !intersects(env)) {
		return false;
	}

	double intMinX = minx > env.minx ? minx : env.minx;
	double intMinY = miny > env.miny ? miny : env.miny;
	double intMaxX = maxx < env.maxx ? maxx : env.maxx;
	double intMaxY = maxy < env.maxy ? maxy : env.maxy;
	result.init(intMinX, intMaxX, intMinY, intMaxY);
	return true;
}

/*public*/
bool Envelope::equals(const Envelope *other) const {
	if (isNull()) {
		return other->isNull();
	}
	return other->minx == minx && other->maxx == maxx && other->miny == miny && other->maxy == maxy;
}

/*public*/
std::string Envelope::toString() const {
	std::ostringstream s;
	s << *this;
	return s.str();
}

} // namespace geom
} // namespace geos
