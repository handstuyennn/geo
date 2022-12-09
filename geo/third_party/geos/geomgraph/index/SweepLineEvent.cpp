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
 **********************************************************************/

#include <geos/geomgraph/index/SweepLineEvent.hpp>
#include <geos/geomgraph/index/SweepLineEventObj.hpp>
#include <sstream>

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index {     // geos.geomgraph.index

SweepLineEvent::SweepLineEvent(void *newEdgeSet, double x, SweepLineEvent *newInsertEvent, SweepLineEventOBJ *newObj)
    : edgeSet(newEdgeSet), obj(newObj), xValue(x), insertEvent(newInsertEvent), deleteEventIndex(0) {
}

} // namespace index
} // namespace geomgraph
} // namespace geos
