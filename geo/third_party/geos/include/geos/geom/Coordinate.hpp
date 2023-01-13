/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/constants.hpp> // for DoubleNotANumber
#include <geos/export.hpp>
#include <limits>
#include <set>
#include <stack>
#include <string>
#include <vector> // for typedefs

namespace geos {
namespace geom { // geos.geom

struct CoordinateLessThen;

class GEOS_DLL CoordinateXY {

	static CoordinateXY _nullCoord;

public:
	CoordinateXY() : x(0.0), y(0.0) {
	}

	CoordinateXY(double xNew, double yNew) : x(xNew), y(yNew) {
	}

	/// x-coordinate
	double x;

	/// y-coordinate
	double y;

	bool isNull() const {
		return (std::isnan(x) && std::isnan(y));
	};

	bool isValid() const {
		return std::isfinite(x) && std::isfinite(y);
	};

	bool equals2D(const CoordinateXY &other) const {
		if (x != other.x) {
			return false;
		}
		if (y != other.y) {
			return false;
		}
		return true;
	};

	bool equals2D(const CoordinateXY &other, double tolerance) const {
		if (std::abs(x - other.x) > tolerance) {
			return false;
		}
		if (std::abs(y - other.y) > tolerance) {
			return false;
		}
		return true;
	};

	/// 2D only
	bool equals(const CoordinateXY &other) const {
		return equals2D(other);
	};

	static CoordinateXY &getNull();

	double distance(const CoordinateXY &p) const {
		double dx = x - p.x;
		double dy = y - p.y;
		return std::sqrt(dx * dx + dy * dy);
	};

	double distanceSquared(const CoordinateXY &p) const {
		double dx = x - p.x;
		double dy = y - p.y;
		return dx * dx + dy * dy;
	};

	/// Equality operator for Coordinate. 2D only.
	GEOS_DLL friend bool operator==(const CoordinateXY &a, const CoordinateXY &b) {
		return a.equals2D(b);
	};

	/// Inequality operator for Coordinate. 2D only.
	GEOS_DLL friend bool operator!=(const CoordinateXY &a, const CoordinateXY &b) {
		return !a.equals2D(b);
	};

	/// TODO: deprecate this, move logic to CoordinateLessThen instead
	int compareTo(const CoordinateXY &other) const {
		if (x < other.x) {
			return -1;
		}
		if (x > other.x) {
			return 1;
		}
		if (y < other.y) {
			return -1;
		}
		if (y > other.y) {
			return 1;
		}
		return 0;
	};

	struct GEOS_DLL HashCode {
		std::size_t operator()(const CoordinateXY &c) const {
			size_t h = std::hash<double> {}(c.x);
			h ^= std::hash<double> {}(c.y) << 1;
			// z ordinate ignored for consistency with operator==
			return h;
		};
	};

	void setNull() {
		x = DoubleNotANumber;
		y = DoubleNotANumber;
	};

	/// Output function
	GEOS_DLL friend std::ostream &operator<<(std::ostream &os, const CoordinateXY &c);
};

/**
 * \class Coordinate geom.h geos.h
 *
 * \brief
 * Coordinate is the lightweight class used to store coordinates.
 *
 * It is distinct from Point, which is a subclass of Geometry.
 * Unlike objects of type Point (which contain additional
 * information such as an envelope, a precision model, and spatial
 * reference system information), a Coordinate only contains
 * ordinate values and accessor methods.
 *
 * Coordinate objects are two-dimensional points, with an additional
 * z-ordinate. JTS does not support any operations on the z-ordinate except
 * the basic accessor functions.
 *
 * Constructed coordinates will have a z-ordinate of DoubleNotANumber.
 * The standard comparison functions will ignore the z-ordinate.
 *
 */
// Define the following to make assignments and copy constructions
// NON-(will let profilers report usages)
// #define PROFILE_COORDINATE_COPIES 1
class GEOS_DLL Coordinate : public CoordinateXY {
private:
	static Coordinate _nullCoord;

public:
	/// A set of const Coordinate pointers
	typedef std::set<const Coordinate *, CoordinateLessThen> ConstSet;

	/// A vector of const Coordinate pointers
	typedef std::vector<const Coordinate *> ConstVect;

	/// A vector of Coordinate objects (real object, not pointers)
	typedef std::vector<Coordinate> Vect;

	/// z-coordinate
	double z;

	/// Output function
	GEOS_DLL friend std::ostream &operator<<(std::ostream &os, const Coordinate &c);

	Coordinate() : CoordinateXY(0.0, 0.0), z(DoubleNotANumber) {};

	Coordinate(double xNew, double yNew, double zNew = DoubleNotANumber) : CoordinateXY(xNew, yNew), z(zNew) {};

	explicit Coordinate(const CoordinateXY &other) : CoordinateXY(other), z(DoubleNotANumber) {};

	static Coordinate &getNull();

	void setNull() {
		CoordinateXY::setNull();
		z = DoubleNotANumber;
	};
};

/// Strict weak ordering Functor for Coordinate
struct GEOS_DLL CoordinateLessThen {

	bool operator()(const CoordinateXY *a, const CoordinateXY *b) const {
		if (a->compareTo(*b) < 0) {
			return true;
		} else {
			return false;
		}
	};

	bool operator()(const CoordinateXY &a, const CoordinateXY &b) const {
		if (a.compareTo(b) < 0) {
			return true;
		} else {
			return false;
		}
	};
};

/// Strict weak ordering operator for Coordinate
inline bool operator<(const CoordinateXY &a, const CoordinateXY &b) {
	return CoordinateLessThen()(a, b);
}

} // namespace geom
} // namespace geos
