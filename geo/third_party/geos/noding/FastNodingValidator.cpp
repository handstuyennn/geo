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
 * Last port: noding/FastNodingValidator.java rev. ??? (JTS-1.8)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/noding/FastNodingValidator.hpp>
#include <geos/noding/MCIndexNoder.hpp> // for checkInteriorIntersections()
#include <geos/noding/NodingIntersectionFinder.hpp>
#include <geos/util/TopologyException.hpp> // for checkValid()
#include <iostream>
#include <string>

namespace geos {
namespace noding { // geos.noding

/*private*/
void FastNodingValidator::checkInteriorIntersections() {
	isValidVar = true;
	segInt.reset(new NodingIntersectionFinder(li));
	MCIndexNoder noder;
	noder.setSegmentIntersector(segInt.get());
	noder.computeNodes(&segStrings);
	if (segInt->hasIntersection()) {
		isValidVar = false;
		return;
	}
}

void FastNodingValidator::checkValid() {
	execute();
	if (!isValidVar) {
		// std::cerr << "Not valid: " << getErrorMessage() << " interior intersection: " <<
		// segInt->getInteriorIntersection() << std::endl;
		throw util::TopologyException(getErrorMessage(), segInt->getInteriorIntersection());
	}
}

/*public*/
std::string FastNodingValidator::getErrorMessage() const {
	using geos::geom::Coordinate;

	if (isValidVar) {
		return std::string("no intersections found");
	}

	// return std::string("found non-noded intersection etc etc");

	const std::vector<Coordinate> &intSegs = segInt->getIntersectionSegments();
	assert(intSegs.size() == 4);
	return "found non-noded intersection between LineString and LineString";
}

} // namespace noding
} // namespace geos
