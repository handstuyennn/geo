/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: precision/GeometryPrecisionReducer.cpp rev. 1.10 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.hpp>
#include <geos/geom/CoordinateSequence.hpp>
#include <geos/geom/CoordinateSequenceFactory.hpp>
#include <geos/geom/GeometryFactory.hpp>
#include <geos/geom/LineString.hpp>
#include <geos/geom/LinearRing.hpp>
#include <geos/geom/PrecisionModel.hpp>
#include <geos/geom/util/GeometryEditor.hpp>
#include <geos/geom/util/NoOpGeometryOperation.hpp>
#include <geos/operation/overlayng/PrecisionReducer.hpp>
#include <geos/precision/GeometryPrecisionReducer.hpp>
#include <geos/precision/PointwisePrecisionReducerTransformer.hpp>
#include <geos/precision/PrecisionReducerCoordinateOperation.hpp>
#include <geos/precision/PrecisionReducerTransformer.hpp>
#include <typeinfo>
#include <vector>

using namespace geos::geom;
using namespace geos::geom::util;

namespace geos {
namespace precision { // geos.precision

/* public */
std::unique_ptr<Geometry> GeometryPrecisionReducer::reduce(const Geometry &geom) {
	std::unique_ptr<Geometry> reduced;
	if (isPointwise) {
		reduced = PointwisePrecisionReducerTransformer::reduce(geom, targetPM);
	} else {
		reduced = PrecisionReducerTransformer::reduce(geom, targetPM, removeCollapsed);
	}

	// TODO: incorporate this in the Transformer above
	if (changePrecisionModel && (&targetPM != geom.getFactory()->getPrecisionModel())) {
		return changePM(reduced.get(), targetPM);
	}

	return reduced;
}

/* private */
std::unique_ptr<Geometry> GeometryPrecisionReducer::changePM(const Geometry *geom, const geom::PrecisionModel &newPM) {
	const GeometryFactory *previousFactory = geom->getFactory();
	GeometryFactory::Ptr changedFactory = createFactory(*previousFactory, newPM);
	GeometryEditor geomEdit(changedFactory.get());

	// this operation changes the PM for the entire geometry tree
	NoOpGeometryOperation noop;
	return geomEdit.edit(geom, &noop);
}

/* public static */
std::unique_ptr<Geometry> GeometryPrecisionReducer::reducePointwise(const Geometry &g,
                                                                    const geom::PrecisionModel &precModel) {
	GeometryPrecisionReducer reducer(precModel);
	reducer.setPointwise(true);
	return reducer.reduce(g);
}

/* public static */
std::unique_ptr<Geometry> GeometryPrecisionReducer::reduceKeepCollapsed(const Geometry &g,
                                                                        const geom::PrecisionModel &precModel) {
	GeometryPrecisionReducer reducer(precModel);
	reducer.setRemoveCollapsedComponents(false);
	return reducer.reduce(g);
}

/* public static */
std::unique_ptr<geom::Geometry> GeometryPrecisionReducer::reduce(const geom::Geometry &g,
                                                                 const geom::PrecisionModel &precModel) {
	GeometryPrecisionReducer reducer(precModel);
	return reducer.reduce(g);
}

/* private */
GeometryFactory::Ptr GeometryPrecisionReducer::createFactory(const GeometryFactory &oldGF,
                                                             const PrecisionModel &newPM) {
	GeometryFactory::Ptr p_newFactory(GeometryFactory::create(
	    &newPM, oldGF.getSRID(), const_cast<CoordinateSequenceFactory *>(oldGF.getCoordinateSequenceFactory())));
	return p_newFactory;
}

} // namespace precision
} // namespace geos
