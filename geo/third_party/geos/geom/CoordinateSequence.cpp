/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/CoordinateSequence.hpp>
// FIXME: we should probably not be using CoordinateArraySequenceFactory
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <iterator>
#include <sstream>
#include <vector>

namespace geos {
namespace geom { // geos::geom

double CoordinateSequence::getOrdinate(std::size_t index, std::size_t ordinateIndex) const {
	switch (ordinateIndex) {
	case CoordinateSequence::X:
		return getAt(index).x;
	case CoordinateSequence::Y:
		return getAt(index).y;
	case CoordinateSequence::Z:
		return getAt(index).z;
	default:
		return DoubleNotANumber;
	}
}

bool CoordinateSequence::hasRepeatedPoints() const {
	const std::size_t p_size = getSize();
	for (std::size_t i = 1; i < p_size; i++) {
		if (getAt(i - 1) == getAt(i)) {
			return true;
		}
	}
	return false;
}

bool CoordinateSequence::hasRepeatedPoints(const CoordinateSequence *cl) {
	const std::size_t size = cl->getSize();
	for (std::size_t i = 1; i < size; i++) {
		if (cl->getAt(i - 1) == cl->getAt(i)) {
			return true;
		}
	}
	return false;
}

int CoordinateSequence::increasingDirection(const CoordinateSequence &pts) {
	std::size_t ptsize = pts.size();
	for (std::size_t i = 0, n = ptsize / 2; i < n; ++i) {
		std::size_t j = ptsize - 1 - i;
		// skip equal points on both ends
		int comp = pts[i].compareTo(pts[j]);
		if (comp != 0) {
			return comp;
		}
	}
	// array must be a palindrome - defined to be in positive direction
	return 1;
}

/* public */
bool CoordinateSequence::isRing() const {
	if (size() < 4)
		return false;

	if (getAt(0) != getAt(size() - 1))
		return false;

	return true;
}

void CoordinateSequence::reverse(CoordinateSequence *cl) {
	// FIXME: use a standard algorithm
	auto last = cl->size() - 1;
	auto mid = last / 2;
	for (std::size_t i = 0; i <= mid; i++) {
		const Coordinate tmp = cl->getAt(i);
		cl->setAt(cl->getAt(last - i), i);
		cl->setAt(tmp, last - i);
	}
}

void CoordinateSequence::expandEnvelope(Envelope &env) const {
	const std::size_t p_size = getSize();
	for (std::size_t i = 0; i < p_size; i++) {
		env.expandToInclude(getAt(i));
	}
}

Envelope CoordinateSequence::getEnvelope() const {
	Envelope e;
	expandEnvelope(e);
	return e;
}

} // namespace geom
} // namespace geos
