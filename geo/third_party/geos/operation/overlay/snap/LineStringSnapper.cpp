/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2009-2010  Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/LineStringSnapper.java r320 (JTS-1.12)
 *
 * NOTE: algorithm changed to improve output quality by reducing
 *       probability of self-intersections
 *
 **********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateList.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/Envelope.hpp>
#include <geos/geom/LineSegment.hpp>
#include <geos/operation/overlay/snap/LineStringSnapper.hpp>
#include <geos/util/Interrupt.hpp>
#include <geos/util/UniqueCoordinateArrayFilter.hpp>
#include <memory>
#include <vector>

//
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay {   // geos.operation.overlay
namespace snap {      // geos.operation.overlay.snap

/*public*/
std::unique_ptr<Coordinate::Vect> LineStringSnapper::snapTo(const geom::Coordinate::ConstVect &snapPts) {
	geom::CoordinateList coordList(srcPts);

	snapVertices(coordList, snapPts);
	snapSegments(coordList, snapPts);

	return coordList.toCoordinateArray();
}

/*private*/
void LineStringSnapper::snapVertices(geom::CoordinateList &srcCoords, const geom::Coordinate::ConstVect &snapPts) {
	// nothing to do if there are no source coords..
	if (srcCoords.empty()) {
		return;
	}

	for (Coordinate::ConstVect::const_iterator it = snapPts.begin(), end = snapPts.end(); it != end; ++it) {
		GEOS_CHECK_FOR_INTERRUPTS();
		assert(*it);
		const Coordinate &snapPt = *(*it);

		CoordinateList::iterator too_far = srcCoords.end();
		if (isClosed) {
			--too_far;
		}
		CoordinateList::iterator vertpos = findVertexToSnap(snapPt, srcCoords.begin(), too_far);
		if (vertpos == too_far) {
			continue;
		}
		*vertpos = snapPt;

		// keep final closing point in synch (rings only)
		if (vertpos == srcCoords.begin() && isClosed) {
			vertpos = srcCoords.end();
			--vertpos;
			*vertpos = snapPt;
		}
	}
}

/*private*/
CoordinateList::iterator LineStringSnapper::findVertexToSnap(const Coordinate &snapPt, CoordinateList::iterator from,
                                                             CoordinateList::iterator too_far) {
	double minDist = snapTolerance; // make sure the first closer then
	// snapTolerance is accepted
	CoordinateList::iterator match = too_far;

	for (; from != too_far; ++from) {
		Coordinate &c0 = *from;

		double dist = c0.distance(snapPt);
		if (dist >= minDist) {
			continue;
		}

		if (dist == 0.0) {
			return from; // can't find any closer
		}

		match = from;
		minDist = dist;
	}

	return match;
}

/*private*/
void LineStringSnapper::snapSegments(geom::CoordinateList &srcCoords, const geom::Coordinate::ConstVect &snapPts) {

	// nothing to do if there are no source coords..
	if (srcCoords.empty()) {
		return;
	}

	GEOS_CHECK_FOR_INTERRUPTS();

	for (Coordinate::ConstVect::const_iterator it = snapPts.begin(), end = snapPts.end(); it != end; ++it) {
		assert(*it);
		const Coordinate &snapPt = *(*it);

		CoordinateList::iterator too_far = srcCoords.end();
		--too_far;
		CoordinateList::iterator segpos = findSegmentToSnap(snapPt, srcCoords.begin(), too_far);
		if (segpos == too_far) {
			continue;
		}

		/* Check if the snap point falls outside of the segment */
		// If the snap point is outside, this means that an endpoint
		// was not snap where it should have been
		// so what we should do is re-snap the endpoint to this
		// snapPt and then snap the closest between this and
		// previous (for pf < 0.0) or next (for pf > 1.0) segment
		// to the old endpoint.
		//     --strk May 2013
		//
		// TODO: simplify this code, make more readable
		//
		CoordinateList::iterator to = segpos;
		++to;
		LineSegment seg(*segpos, *to);
		double pf = seg.projectionFactor(snapPt);
		if (pf >= 1.0) {
			Coordinate newSnapPt = seg.p1;
			*to = seg.p1 = snapPt;
			// now snap from-to (segpos) or to-next (segpos++) to newSnapPt
			if (to == too_far) {
				if (isClosed) {
					*(srcCoords.begin()) = snapPt; // sync to start point
					to = srcCoords.begin();
				} else {
					srcCoords.insert(to, newSnapPt);
					continue;
				}
			}
			++to;
			LineSegment nextSeg(seg.p1, *to);
			if (nextSeg.distance(newSnapPt) < seg.distance(newSnapPt)) {
				// insert into next segment
				srcCoords.insert(to, newSnapPt);
			} else {
				// insert must happen one-past first point (before next point)
				++segpos;
				srcCoords.insert(segpos, newSnapPt);
			}
		} else if (pf <= 0.0) {
			Coordinate newSnapPt = seg.p0;
			*segpos = seg.p0 = snapPt;
			// now snap prev-from (--segpos) or from-to (segpos) to newSnapPt
			if (segpos == srcCoords.begin()) {
				if (isClosed) {
					segpos = srcCoords.end();
					--segpos;
					*segpos = snapPt; // sync to end point
				} else {
					++segpos;
					srcCoords.insert(segpos, newSnapPt);
					continue;
				}
			}

			--segpos;
			LineSegment prevSeg(*segpos, seg.p0);
			if (prevSeg.distance(newSnapPt) < seg.distance(newSnapPt)) {
				// insert into prev segment
				++segpos;
				srcCoords.insert(segpos, newSnapPt);
			} else {
				// insert must happen one-past first point (before next point)
				srcCoords.insert(to, newSnapPt);
			}
		} else {
			// assert(pf != 0.0);
			//  insert must happen one-past first point (before next point)
			++segpos;
			srcCoords.insert(segpos, snapPt);
		}
	}
}

/*private*/
/* NOTE: this is called findSegmentIndexToSnap in JTS */
CoordinateList::iterator LineStringSnapper::findSegmentToSnap(const Coordinate &snapPt, CoordinateList::iterator from,
                                                              CoordinateList::iterator too_far) {
	LineSegment seg;
	double minDist = snapTolerance; // make sure the first closer then
	// snapTolerance is accepted
	CoordinateList::iterator match = too_far;

	// TODO: use std::find_if
	for (; from != too_far; ++from) {
		seg.p0 = *from;
		CoordinateList::iterator to = from;
		++to;
		seg.p1 = *to;

		/*
		 * Check if the snap pt is equal to one of
		 * the segment endpoints.
		 *
		 * If the snap pt is already in the src list,
		 * don't snap at all (unless allowSnappingToSourceVertices
		 * is set to true)
		 */
		if (seg.p0.equals2D(snapPt) || seg.p1.equals2D(snapPt)) {

			if (allowSnappingToSourceVertices) {
				continue;
			} else {
				return too_far;
			}
		}

		if (Envelope::distanceSquaredToCoordinate(snapPt, seg.p0, seg.p1) >= minDist * minDist) {
			continue;
		}

		double dist = seg.distance(snapPt);
		if (dist >= minDist) {
			continue;
		}

		if (dist == 0.0) {
			return from; // can't find any closer
		}

		match = from;
		minDist = dist;
	}

	return match;
}

} // namespace snap
} // namespace overlay
} // namespace operation
} // namespace geos
