/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2010 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/CoordinateList.java ?? (never been in complete sync)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp>
#include <list>
#include <memory>  // for unique_ptr
#include <ostream> // for operator<<

namespace geos {
namespace geom { // geos::geom

/** \brief
 * A list of {@link Coordinate}s, which may
 * be set to prevent repeated coordinates from occuring in the list.
 *
 * Use this class when fast insertions and removal at arbitrary
 * position is needed.
 * The class keeps ownership of the Coordinates.
 *
 */
class GEOS_DLL CoordinateList {
public:
	typedef std::list<Coordinate>::iterator iterator;
	typedef std::list<Coordinate>::const_iterator const_iterator;

	friend std::ostream &operator<<(std::ostream &os, const CoordinateList &cl);

	/** \brief
	 * Constructs a new list from an array of Coordinates, allowing
	 * repeated points.
	 *
	 * (I.e. this constructor produces a {@link CoordinateList} with
	 * exactly the same set of points as the input array.)
	 *
	 * @param v the initial coordinates
	 */
	CoordinateList(const std::vector<Coordinate> &v) : coords(v.begin(), v.end()) {
	}

	CoordinateList() : coords() {
	}

	std::unique_ptr<Coordinate::Vect> toCoordinateArray() const {
		std::unique_ptr<Coordinate::Vect> ret(new Coordinate::Vect);
		ret->assign(coords.begin(), coords.end());
		return ret;
	}

	bool empty() const {
		return coords.empty();
	}

	iterator begin() {
		return coords.begin();
	}

	iterator end() {
		return coords.end();
	}

	const_iterator begin() const {
		return coords.begin();
	}

	const_iterator end() const {
		return coords.end();
	}

	/** \brief
	 * Inserts the specified coordinate at the specified position in this list.
	 *
	 * @param pos the position at which to insert
	 * @param c the coordinate to insert
	 * @param allowRepeated if set to false, repeated coordinates are collapsed
	 *
	 * @return an iterator to the newly installed coordinate
	 *         (or previous, if equal and repeated are not allowed)
	 *
	 * NOTE: when allowRepeated is false _next_ point is not checked
	 *       this matches JTS behavior
	 */
	iterator insert(iterator pos, const Coordinate &c, bool allowRepeated) {
		if (!allowRepeated && pos != coords.begin()) {
			iterator prev = pos;
			--prev;
			if (c.equals2D(*prev)) {
				return prev;
			}
		}
		return coords.insert(pos, c);
	}

	iterator insert(iterator pos, const Coordinate &c) {
		return coords.insert(pos, c);
	}

private:
	std::list<Coordinate> coords;
};

} // namespace geom
} // namespace geos
