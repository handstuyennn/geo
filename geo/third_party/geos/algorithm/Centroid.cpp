/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2013 Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Centroid.java r728 (JTS-0.13+)
 *
 **********************************************************************/

#include <cmath> // for std::abs
#include <geos/algorithm/Centroid.hpp>
#include <geos/algorithm/Orientation.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Geometry.hpp>
#include <geos/geom/GeometryCollection.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/Point.hpp>
#include <geos/geom/Polygon.hpp>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/* static public */
bool Centroid::getCentroid(const Geometry &geom, CoordinateXY &pt) {
	Centroid cent(geom);
	return cent.getCentroid(pt);
}

/* public */
bool Centroid::getCentroid(CoordinateXY &cent) const {
	if (std::abs(areasum2) > 0.0) {
		cent.x = cg3.x / 3 / areasum2;
		cent.y = cg3.y / 3 / areasum2;
	} else if (totalLength > 0.0) {
		// if polygon was degenerate, compute linear centroid instead
		cent.x = lineCentSum.x / totalLength;
		cent.y = lineCentSum.y / totalLength;
	} else if (ptCount > 0) {
		cent.x = ptCentSum.x / ptCount;
		cent.y = ptCentSum.y / ptCount;
	} else {
		return false;
	}
	return true;
}

/* private */
void Centroid::add(const Geometry &geom) {
	if (geom.isEmpty()) {
		return;
	}

	if (const Point *pt = dynamic_cast<const Point *>(&geom)) {
		addPoint(*pt->getCoordinate());
	} else if (const LineString *ls = dynamic_cast<const LineString *>(&geom)) {
		addLineSegments(*ls->getCoordinatesRO());
	} else if (const Polygon *p = dynamic_cast<const Polygon *>(&geom)) {
		add(*p);
	} else if (const GeometryCollection *g = dynamic_cast<const GeometryCollection *>(&geom)) {
		for (std::size_t i = 0; i < g->getNumGeometries(); i++) {
			add(*g->getGeometryN(i));
		}
	}
}

/* private */
void Centroid::addLineSegments(const CoordinateSequence &pts) {
	std::size_t npts = pts.size();
	double lineLen = 0.0;
	for (std::size_t i = 0; i < npts - 1; i++) {
		double segmentLen = pts[i].distance(pts[i + 1]);
		if (segmentLen == 0.0) {
			continue;
		}

		lineLen += segmentLen;

		double midx = (pts[i].x + pts[i + 1].x) / 2;
		lineCentSum.x += segmentLen * midx;
		double midy = (pts[i].y + pts[i + 1].y) / 2;
		lineCentSum.y += segmentLen * midy;
	}
	totalLength += lineLen;
	if (lineLen == 0.0 && npts > 0) {
		addPoint(pts[0]);
	}
}

/* private */
void Centroid::addPoint(const CoordinateXY &pt) {
	ptCount += 1;
	ptCentSum.x += pt.x;
	ptCentSum.y += pt.y;
}

} // namespace algorithm
} // namespace geos
