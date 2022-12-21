/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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
 * Last port: geom/GeometryCollection.java rev. 1.41
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Dimension.hpp> // for Dimension::DimensionType
#include <geos/geom/Envelope.hpp>  // for proper use of unique_ptr<>
#include <geos/geom/Geometry.hpp>  // for inheritance
#include <memory>                  // for unique_ptr
#include <string>
#include <vector>

// Forward declarations
namespace geos {
namespace geom { // geos::geom
class Coordinate;
class CoordinateArraySequence;
class CoordinateSequenceFilter;
} // namespace geom
} // namespace geos

namespace geos {
namespace geom { // geos::geom

/**
 * \class GeometryCollection geom.h geos.h
 *
 * \brief Represents a collection of heterogeneous Geometry objects.
 *
 * Collections of Geometry of the same type are
 * represented by GeometryCollection subclasses MultiPoint,
 * MultiLineString, MultiPolygon.
 */
class GEOS_DLL GeometryCollection : public Geometry {

public:
	friend class GeometryFactory;

	typedef std::vector<std::unique_ptr<Geometry>>::const_iterator const_iterator;

	typedef std::vector<std::unique_ptr<Geometry>>::iterator iterator;

	const_iterator begin() const {
		return geometries.begin();
	};

	const_iterator end() const {
		return geometries.end();
	};

	/**
	 * Creates and returns a full copy of this GeometryCollection object.
	 * (including all coordinates contained by it).
	 *
	 * @return a clone of this instance
	 */
	std::unique_ptr<GeometryCollection> clone() const {
		return std::unique_ptr<GeometryCollection>(cloneImpl());
	}

	~GeometryCollection() override = default;

	void setSRID(int) override;

	bool isEmpty() const override;

	/**
	 * \brief
	 * Returns the maximum dimension of geometries in this collection
	 * (0=point, 1=line, 2=surface)
	 *
	 * @see Dimension::DimensionType
	 */
	Dimension::DimensionType getDimension() const override;

	bool isDimensionStrict(Dimension::DimensionType d) const override;

	/// Returns coordinate dimension.
	uint8_t getCoordinateDimension() const override;

	std::size_t getNumPoints() const override;

	std::string getGeometryType() const override;

	GeometryTypeId getGeometryTypeId() const override;

	void apply_ro(CoordinateFilter *filter) const override;

	void apply_rw(const CoordinateFilter *filter) override;

	void apply_ro(GeometryFilter *filter) const override;

	void apply_rw(GeometryFilter *filter) override;

	void apply_ro(GeometryComponentFilter *filter) const override;

	void apply_rw(GeometryComponentFilter *filter) override;

	void apply_rw(CoordinateSequenceFilter &filter) override;

	void apply_ro(CoordinateSequenceFilter &filter) const override;

	const CoordinateXY *getCoordinate() const override;

	/// Returns the number of geometries in this collection
	std::size_t getNumGeometries() const override;

	/// Returns a pointer to the nth Geometry in this collection
	const Geometry *getGeometryN(std::size_t n) const override;

	/**
	 * \brief
	 * Take ownership of the sub-geometries managed by this GeometryCollection.
	 * After releasing the sub-geometries, the collection should be considered
	 * in a moved-from state and should not be accessed.
	 * @return vector of sub-geometries
	 */
	std::vector<std::unique_ptr<Geometry>> releaseGeometries();

	/// Returns the total area of this collection
	double getArea() const override;

	std::unique_ptr<Geometry> getBoundary() const override;

	/**
	 * \brief
	 * Returns the maximum boundary dimension of geometries in
	 * this collection.
	 */
	int getBoundaryDimension() const override;

	/// Returns the total length of this collection
	double getLength() const override;

protected:
	GeometryCollection(const GeometryCollection &gc);

	/** \brief
	 * Construct a GeometryCollection with the given GeometryFactory.
	 * Will keep a reference to the factory, so don't
	 * delete it until al Geometry objects referring to
	 * it are deleted.
	 * Will take ownership of the Geometry vector.
	 *
	 * @param newGeoms
	 *	The <code>Geometry</code>s for this
	 *	<code>GeometryCollection</code>,
	 *	or <code>null</code> or an empty array to
	 *	create the empty geometry.
	 *	Elements may be empty <code>Geometry</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	If construction succeed the created object will take
	 *	ownership of newGeoms vector and elements.
	 *
	 *	If construction	fails "IllegalArgumentException *"
	 *	is thrown and it is your responsibility to delete newGeoms
	 *	vector and content.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 */
	GeometryCollection(std::vector<Geometry *> *newGeoms, const GeometryFactory *newFactory);

	GeometryCollection(std::vector<std::unique_ptr<Geometry>> &&newGeoms, const GeometryFactory &newFactory);

	/// Convenience constructor to build a GeometryCollection from vector of Geometry subclass pointers
	template <typename T>
	GeometryCollection(std::vector<std::unique_ptr<T>> &&newGeoms, const GeometryFactory &newFactory)
	    : GeometryCollection(toGeometryArray(std::move(newGeoms)), newFactory) {
	}

	GeometryCollection *cloneImpl() const override {
		return new GeometryCollection(*this);
	}

	int getSortIndex() const override {
		return SORTINDEX_GEOMETRYCOLLECTION;
	};

	std::vector<std::unique_ptr<Geometry>> geometries;

	Envelope::Ptr computeEnvelopeInternal() const override;
};

} // namespace geom
} // namespace geos
