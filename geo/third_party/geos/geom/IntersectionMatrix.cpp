/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/IntersectionMatrix.java rev. 1.18
 *
 **********************************************************************/

#include <cassert>
#include <geos/geom/Dimension.hpp>
#include <geos/geom/IntersectionMatrix.hpp>
#include <geos/geom/Location.hpp>
#include <geos/util/IllegalArgumentException.hpp>
#include <sstream>

namespace geos {
namespace geom { // geos::geom

const int IntersectionMatrix::firstDim = 3;
const int IntersectionMatrix::secondDim = 3;

/*public*/
IntersectionMatrix::IntersectionMatrix() {
	// matrix = new int[3][3];
	setAll(Dimension::False);
}

/*public*/
IntersectionMatrix::IntersectionMatrix(const std::string &elements) {
	setAll(Dimension::False);
	set(elements);
}

/*public*/
IntersectionMatrix::IntersectionMatrix(const IntersectionMatrix &other) {
	matrix = other.matrix;
}

/*public*/
void IntersectionMatrix::add(IntersectionMatrix *other) {
	for (std::size_t i = 0; i < firstDim; i++) {
		for (std::size_t j = 0; j < secondDim; j++) {
			setAtLeast(static_cast<Location>(i), static_cast<Location>(j),
			           other->get(static_cast<Location>(i), static_cast<Location>(j)));
		}
	}
}

/*public*/
bool IntersectionMatrix::matches(const std::string &requiredDimensionSymbols) const {
	if (requiredDimensionSymbols.length() != 9) {
		std::ostringstream s;
		s << "IllegalArgumentException: Should be length 9, is "
		  << "[" << requiredDimensionSymbols << "] instead" << std::endl;
		throw util::IllegalArgumentException(s.str());
	}
	for (std::size_t ai = 0; ai < firstDim; ai++) {
		for (std::size_t bi = 0; bi < secondDim; bi++) {
			if (!matches(matrix[ai][bi], requiredDimensionSymbols[3 * ai + bi])) {
				return false;
			}
		}
	}
	return true;
}

/*public static*/
bool IntersectionMatrix::matches(int actualDimensionValue, char requiredDimensionSymbol) {

	if (requiredDimensionSymbol == '*') {
		return true;
	}

	if (requiredDimensionSymbol == 'T' && (actualDimensionValue >= 0 || actualDimensionValue == Dimension::True)) {
		return true;
	}

	if (requiredDimensionSymbol == 'F' && actualDimensionValue == Dimension::False) {
		return true;
	}

	if (requiredDimensionSymbol == '0' && actualDimensionValue == Dimension::P) {
		return true;
	}

	if (requiredDimensionSymbol == '1' && actualDimensionValue == Dimension::L) {
		return true;
	}

	if (requiredDimensionSymbol == '2' && actualDimensionValue == Dimension::A) {
		return true;
	}

	return false;
}

/*public static*/
bool IntersectionMatrix::matches(const std::string &actualDimensionSymbols,
                                 const std::string &requiredDimensionSymbols) {
	IntersectionMatrix m(actualDimensionSymbols);
	bool result = m.matches(requiredDimensionSymbols);

	return result;
}

/*public*/
void IntersectionMatrix::set(Location row, Location col, int dimensionValue) {
	matrix[static_cast<size_t>(row)][static_cast<size_t>(col)] = dimensionValue;
}

/*public*/
void IntersectionMatrix::set(const std::string &dimensionSymbols) {
	auto limit = dimensionSymbols.length();

	for (std::size_t i = 0; i < limit; i++) {
		auto row = i / firstDim;
		auto col = i % secondDim;
		matrix[row][col] = Dimension::toDimensionValue(dimensionSymbols[i]);
	}
}

/*public*/
void IntersectionMatrix::setAtLeast(Location row, Location col, int minimumDimensionValue) {
	if (get(row, col) < minimumDimensionValue) {
		set(row, col, minimumDimensionValue);
	}
}

/*public*/
void IntersectionMatrix::setAtLeastIfValid(Location row, Location col, int minimumDimensionValue) {
	if (row != Location::NONE && col != Location::NONE) {
		setAtLeast(row, col, minimumDimensionValue);
	}
}

/*public*/
void IntersectionMatrix::setAtLeast(std::string minimumDimensionSymbols) {
	auto limit = minimumDimensionSymbols.length();

	for (std::size_t i = 0; i < limit; i++) {
		auto row = static_cast<Location>(i / firstDim);
		auto col = static_cast<Location>(i % secondDim);
		setAtLeast(row, col, Dimension::toDimensionValue(minimumDimensionSymbols[i]));
	}
}

/*public*/
void IntersectionMatrix::setAll(int dimensionValue) {
	for (int ai = 0; ai < firstDim; ai++) {
		for (int bi = 0; bi < secondDim; bi++) {
			set(static_cast<Location>(ai), static_cast<Location>(bi), dimensionValue);
		}
	}
}

/*public*/
bool IntersectionMatrix::isEquals(int dimensionOfGeometryA, int dimensionOfGeometryB) const {
	if (dimensionOfGeometryA != dimensionOfGeometryB) {
		return false;
	}
	return matches(get(Location::INTERIOR, Location::INTERIOR), 'T') &&
	       get(Location::EXTERIOR, Location::INTERIOR) == Dimension::False &&
	       get(Location::INTERIOR, Location::EXTERIOR) == Dimension::False &&
	       get(Location::EXTERIOR, Location::BOUNDARY) == Dimension::False &&
	       get(Location::BOUNDARY, Location::EXTERIOR) == Dimension::False;
}

/*public*/
bool IntersectionMatrix::isContains() const {
	return matches(get(Location::INTERIOR, Location::INTERIOR), 'T') &&
	       get(Location::EXTERIOR, Location::INTERIOR) == Dimension::False &&
	       get(Location::EXTERIOR, Location::BOUNDARY) == Dimension::False;
}

/*public*/
bool IntersectionMatrix::isTouches(int dimensionOfGeometryA, int dimensionOfGeometryB) const {
	if (dimensionOfGeometryA > dimensionOfGeometryB) {
		// no need to get transpose because pattern matrix is symmetrical
		return isTouches(dimensionOfGeometryB, dimensionOfGeometryA);
	}
	if ((dimensionOfGeometryA == Dimension::A && dimensionOfGeometryB == Dimension::A) ||
	    (dimensionOfGeometryA == Dimension::L && dimensionOfGeometryB == Dimension::L) ||
	    (dimensionOfGeometryA == Dimension::L && dimensionOfGeometryB == Dimension::A) ||
	    (dimensionOfGeometryA == Dimension::P && dimensionOfGeometryB == Dimension::A) ||
	    (dimensionOfGeometryA == Dimension::P && dimensionOfGeometryB == Dimension::L)) {
		return get(Location::INTERIOR, Location::INTERIOR) == Dimension::False &&
		       (matches(get(Location::INTERIOR, Location::BOUNDARY), 'T') ||
		        matches(get(Location::BOUNDARY, Location::INTERIOR), 'T') ||
		        matches(get(Location::BOUNDARY, Location::BOUNDARY), 'T'));
	}
	return false;
}

} // namespace geom
} // namespace geos
