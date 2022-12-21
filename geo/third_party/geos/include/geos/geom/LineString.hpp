/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/LineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/CoordinateSequence.hpp> // for proper use of unique_ptr<>
#include <geos/geom/Dimension.hpp>          // for Dimension::DimensionType
#include <geos/geom/Envelope.hpp>           // for proper use of unique_ptr<>
#include <geos/geom/Geometry.hpp>           // for inheritance
#include <memory>                           // for unique_ptr
#include <string>
#include <vector>

namespace geos {
namespace geom {
class Coordinate;
class CoordinateArraySequence;
class CoordinateSequenceFilter;
} // namespace geom
} // namespace geos

namespace geos {
namespace geom { // geos::geom

/**
 *  Models an OGC-style <code>LineString</code>.
 *
 *  A LineString consists of a sequence of two or more vertices,
 *  along with all points along the linearly-interpolated curves
 *  (line segments) between each
 *  pair of consecutive vertices.
 *  Consecutive vertices may be equal.
 *  The line segments in the line may intersect each other (in other words,
 *  the linestring may "curl back" in itself and self-intersect).
 *  Linestrings with exactly two identical points are invalid.
 *
 *  A linestring must have either 0 or 2 or more points.
 *  If these conditions are not met, the constructors throw
 *  an {@link util::IllegalArgumentException}.
 */
class GEOS_DLL LineString : public Geometry {

public:
	friend class GeometryFactory;

	/// A vector of const LineString pointers
	typedef std::vector<const LineString *> ConstVect;

	~LineString() override;

	/**
	 * \brief
	 * Creates and returns a full copy of this {@link LineString} object
	 * (including all coordinates contained by it)
	 *
	 * @return A clone of this instance
	 */
	std::unique_ptr<LineString> clone() const {
		return std::unique_ptr<LineString>(cloneImpl());
	}

	/// Returns a read-only pointer to internal CoordinateSequence
	const CoordinateSequence *getCoordinatesRO() const;

	virtual const Coordinate &getCoordinateN(std::size_t n) const;

	/**
	 * \brief
	 * Take ownership of the CoordinateSequence managed by this geometry.
	 * After releasing the coordinates, the geometry should be considered
	 * in a moved-from state and should not be accessed.
	 * @return this Geometry's CoordinateSequence.
	 */
	std::unique_ptr<CoordinateSequence> releaseCoordinates();

	virtual std::unique_ptr<Point> getPointN(std::size_t n) const;

	/// \brief
	/// Return the start point of the LineString
	/// or NULL if this is an EMPTY LineString.
	///
	virtual std::unique_ptr<Point> getStartPoint() const;

	/// \brief
	/// Return the end point of the LineString
	/// or NULL if this is an EMPTY LineString.
	///
	virtual std::unique_ptr<Point> getEndPoint() const;

	/// Returns line dimension (1)
	Dimension::DimensionType getDimension() const override;

	/**
	 * \brief
	 * Returns Dimension::False for a closed LineString,
	 * 0 otherwise (LineString boundary is a MultiPoint)
	 */
	int getBoundaryDimension() const override;

	/// Returns coordinate dimension.
	uint8_t getCoordinateDimension() const override;

	bool isEmpty() const override;

	std::size_t getNumPoints() const override;

	virtual bool isClosed() const;

	virtual bool isRing() const;

	std::string getGeometryType() const override;

	GeometryTypeId getGeometryTypeId() const override;

	void apply_rw(const CoordinateFilter *filter) override;

	void apply_ro(CoordinateFilter *filter) const override;

	void apply_rw(GeometryFilter *filter) override;

	void apply_ro(GeometryFilter *filter) const override;

	void apply_rw(GeometryComponentFilter *filter) override;

	void apply_ro(GeometryComponentFilter *filter) const override;

	void apply_rw(CoordinateSequenceFilter &filter) override;

	void apply_ro(CoordinateSequenceFilter &filter) const override;

	const CoordinateXY *getCoordinate() const override;

	/**
	 * \brief
	 * Returns a MultiPoint.
	 * Empty for closed LineString, a Point for each vertex otherwise.
	 */
	std::unique_ptr<Geometry> getBoundary() const override;

	double getLength() const override;

protected:
	LineString(const LineString &ls);

	/// \brief
	/// Constructs a LineString taking ownership the
	/// given CoordinateSequence.
	LineString(CoordinateSequence *pts, const GeometryFactory *newFactory);

	/// Hopefully cleaner version of the above
	LineString(CoordinateSequence::Ptr &&pts, const GeometryFactory &newFactory);

	LineString(std::vector<Coordinate> &&pts, const GeometryFactory &newFactory);

	LineString *cloneImpl() const override {
		return new LineString(*this);
	}

	Envelope::Ptr computeEnvelopeInternal() const override;

	CoordinateSequence::Ptr points;

	int getSortIndex() const override {
		return SORTINDEX_LINESTRING;
	};

private:
	void validateConstruction();
	void normalizeClosed();
};

} // namespace geom
} // namespace geos
