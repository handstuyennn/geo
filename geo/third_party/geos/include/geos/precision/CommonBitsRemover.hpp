/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.hpp>
#include <geos/geom/Coordinate.hpp> // for composition

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
namespace precision {
class CommonBitsRemover;
class CommonCoordinateFilter;
} // namespace precision
} // namespace geos

namespace geos {
namespace precision { // geos.precision

/** \brief
 * Allow computing and removing common mantissa bits from one or
 * more Geometries.
 *
 */
class GEOS_DLL CommonBitsRemover {

private:
	geom::Coordinate commonCoord;

	CommonCoordinateFilter *ccFilter;

public:
	CommonBitsRemover();

	~CommonBitsRemover();

	/**
	 * Add a geometry to the set of geometries whose common bits are
	 * being computed.  After this method has executed the
	 * common coordinate reflects the common bits of all added
	 * geometries.
	 *
	 * @param geom a Geometry to test for common bits
	 */
	void add(const geom::Geometry *geom);

	/** \brief
	 * Removes the common coordinate bits from a Geometry.
	 * The coordinates of the Geometry are changed.
	 *
	 * @param geom the Geometry from which to remove the common
	 *             coordinate bits
	 */
	void removeCommonBits(geom::Geometry *geom);

	/** \brief
	 * Adds the common coordinate bits back into a Geometry.
	 * The coordinates of the Geometry are changed.
	 *
	 * @param geom the Geometry to which to add the common coordinate bits
	 * @return the shifted Geometry
	 */
	geom::Geometry *addCommonBits(geom::Geometry *geom);
};

} // namespace precision
} // namespace geos
