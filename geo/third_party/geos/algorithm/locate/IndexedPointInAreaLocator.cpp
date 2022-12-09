/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2018 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <algorithm>
#include <geos/algorithm/RayCrossingCounter.hpp>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/LineSegment.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/MultiPolygon.hpp>
#include <geos/geom/Polygon.hpp>
#include <geos/geom/util/LinearComponentExtracter.hpp>
#include <geos/index/strtree/TemplateSTRtree.hpp>
#include <geos/util.hpp>
#include <typeinfo>

namespace geos {
namespace algorithm {
namespace locate {

//
// private:
//
IndexedPointInAreaLocator::IntervalIndexedGeometry::IntervalIndexedGeometry(const geom::Geometry &g) {
	init(g);
}

void IndexedPointInAreaLocator::IntervalIndexedGeometry::init(const geom::Geometry &g) {
	geom::LineString::ConstVect lines;
	geom::util::LinearComponentExtracter::getLines(g, lines);

	// pre-compute size of segment vector
	std::size_t nsegs = 0;
	for (const geom::LineString *line : lines) {
		//-- only include rings of Polygons or LinearRings
		if (!line->isClosed())
			continue;

		nsegs += line->getCoordinatesRO()->size() - 1;
	}
	index = decltype(index)(10, nsegs);

	for (const geom::LineString *line : lines) {
		//-- only include rings of Polygons or LinearRings
		if (!line->isClosed())
			continue;

		addLine(line->getCoordinatesRO());
	}
}

void IndexedPointInAreaLocator::IntervalIndexedGeometry::addLine(const geom::CoordinateSequence *pts) {
	for (std::size_t i = 1, ni = pts->size(); i < ni; i++) {
		SegmentView seg(&pts->getAt(i - 1), &pts->getAt(i));
		auto r = std::minmax(seg.p0().y, seg.p1().y);

		index.insert(index::strtree::Interval(r.first, r.second), seg);
	}
}

void IndexedPointInAreaLocator::buildIndex(const geom::Geometry &g) {
	index = detail::make_unique<IntervalIndexedGeometry>(g);
}

//
// public:
//
IndexedPointInAreaLocator::IndexedPointInAreaLocator(const geom::Geometry &g) : areaGeom(g) {
}

geom::Location IndexedPointInAreaLocator::locate(const geom::CoordinateXY * /*const*/ p) {
	if (index == nullptr) {
		buildIndex(areaGeom);
	}

	algorithm::RayCrossingCounter rcc(*p);

	index->query(p->y, p->y, [&rcc](const SegmentView &ls) { rcc.countSegment(ls.p0(), ls.p1()); });

	return rcc.getLocation();
}

} // namespace locate
} // namespace algorithm
} // namespace geos
