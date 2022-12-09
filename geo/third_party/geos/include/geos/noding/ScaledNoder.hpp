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
 * Last port: noding/ScaledNoder.java rev. 1.3 (JTS-1.7.1)
 *
 **********************************************************************/

#pragma once

#include <cassert>
#include <geos/export.hpp>
#include <geos/noding/Noder.hpp> // for inheritance
#include <geos/util.hpp>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
} // namespace geom
namespace noding {
class SegmentString;
}
} // namespace geos

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Wraps a {@link Noder} and transforms its input
 * into the integer domain.
 *
 * This is intended for use with Snap-Rounding noders,
 * which typically are only intended to work in the integer domain.
 * Offsets can be provided to increase the number of digits of
 * available precision.
 *
 */
class GEOS_DLL ScaledNoder : public Noder { // , public geom::CoordinateFilter { // implements Noder
public:
	ScaledNoder(Noder &n, double nScaleFactor, double nOffsetX = 0.0, double nOffsetY = 0.0)
	    : noder(n), scaleFactor(nScaleFactor), offsetX(nOffsetX), offsetY(nOffsetY), isScaled(nScaleFactor != 1.0) {
	}

	~ScaledNoder() override;

	std::vector<SegmentString *> *getNodedSubstrings() const override;

	void computeNodes(std::vector<SegmentString *> *inputSegStr) override;

private:
	Noder &noder;

	double scaleFactor;

	double offsetX;

	double offsetY;

	bool isScaled;

	void rescale(std::vector<SegmentString *> &segStrings) const;

	void scale(std::vector<SegmentString *> &segStrings) const;

	class Scaler;

	class ReScaler;

	friend class ScaledNoder::Scaler;

	friend class ScaledNoder::ReScaler;

	mutable std::vector<geom::CoordinateSequence *> newCoordSeq;

	// Declare type as noncopyable
	ScaledNoder(const ScaledNoder &other) = delete;
	ScaledNoder &operator=(const ScaledNoder &rhs) = delete;
};

} // namespace noding
} // namespace geos