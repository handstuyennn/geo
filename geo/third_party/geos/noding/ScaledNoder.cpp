/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/ScaledNoder.java rev. 1.3 (JTS-1.7.1)
 *
 **********************************************************************/

#include <cassert>
#include <functional>
#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateFilter.hpp>   // for inheritance
#include <geos/geom/CoordinateSequence.hpp> // for apply and delete
#include <geos/noding/NodedSegmentString.hpp>
#include <geos/noding/ScaledNoder.hpp>
#include <geos/noding/SegmentString.hpp>
#include <geos/operation/valid/RepeatedPointRemover.hpp>
#include <geos/operation/valid/RepeatedPointTester.hpp>
#include <geos/util.hpp>
#include <geos/util/math.hpp>
#include <vector>

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

class ScaledNoder::Scaler : public geom::CoordinateFilter {
public:
	const ScaledNoder &sn;
	Scaler(const ScaledNoder &n) : sn(n) {
	}

	// void filter_ro(const geom::Coordinate* c) { assert(0); }

	void filter_rw(geom::Coordinate *c) const override {
		c->x = util::round((c->x - sn.offsetX) * sn.scaleFactor);
		c->y = util::round((c->y - sn.offsetY) * sn.scaleFactor);
	}

private:
	// Declare type as noncopyable
	Scaler(const Scaler &other) = delete;
	Scaler &operator=(const Scaler &rhs) = delete;
};

class ScaledNoder::ReScaler : public geom::CoordinateFilter {
public:
	const ScaledNoder &sn;
	ReScaler(const ScaledNoder &n) : sn(n) {
	}

	void filter_ro(const geom::Coordinate *c) override {
		::geos::ignore_unused_variable_warning(c);
		assert(0);
	}

	void filter_rw(geom::Coordinate *c) const override {
		c->x = c->x / sn.scaleFactor + sn.offsetX;
		c->y = c->y / sn.scaleFactor + sn.offsetY;
	}

private:
	// Declare type as noncopyable
	ReScaler(const ReScaler &other);
	ReScaler &operator=(const ReScaler &rhs);
};

ScaledNoder::~ScaledNoder() {
	for (std::vector<geom::CoordinateSequence *>::const_iterator it = newCoordSeq.begin(), end = newCoordSeq.end();
	     it != end; ++it) {
		delete *it;
	}
}

/*public*/
SegmentString::NonConstVect *ScaledNoder::getNodedSubstrings() const {
	SegmentString::NonConstVect *splitSS = noder.getNodedSubstrings();

	if (isScaled) {
		rescale(*splitSS);
	}

	return splitSS;
}

/*public*/
void ScaledNoder::computeNodes(SegmentString::NonConstVect *inputSegStr) {
	if (isScaled) {
		scale(*inputSegStr);
	}

	noder.computeNodes(inputSegStr);
}

/*private*/
void ScaledNoder::scale(SegmentString::NonConstVect &segStrings) const {
	Scaler scaler(*this);
	for (std::size_t i = 0; i < segStrings.size(); i++) {
		SegmentString *ss = segStrings[i];

		CoordinateSequence *cs = ss->getCoordinates();
		cs->apply_rw(&scaler);
		assert(cs->size() == npts);

		operation::valid::RepeatedPointTester rpt;
		if (rpt.hasRepeatedPoint(cs)) {
			auto cs2 = operation::valid::RepeatedPointRemover::removeRepeatedPoints(cs);
			segStrings[i] = new NodedSegmentString(cs2.release(), ss->getData());
			delete ss;
		}
	}
}

/*private*/
void ScaledNoder::rescale(SegmentString::NonConstVect &segStrings) const {
	ReScaler rescaler(*this);
	for (SegmentString::NonConstVect::const_iterator i0 = segStrings.begin(), i0End = segStrings.end(); i0 != i0End;
	     ++i0) {

		SegmentString *ss = *i0;

		ss->getCoordinates()->apply_rw(&rescaler);
	}
}

} // namespace noding
} // namespace geos
