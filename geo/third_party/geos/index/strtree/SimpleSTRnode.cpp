/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Envelope.hpp>
#include <geos/index/strtree/SimpleSTRnode.hpp>
#include <iostream>

using namespace geos::geom;

namespace geos {
namespace index {   // geos.index
namespace strtree { // geos.index.strtree

/*public*/
void SimpleSTRnode::addChildNode(SimpleSTRnode *childNode) {
	if (bounds.isNull())
		bounds = childNode->getEnvelope();
	else
		bounds.expandToInclude(childNode->getEnvelope());

	childNodes.push_back(childNode);
}

bool SimpleSTRnode::removeItem(void *itemToRemove) {
	for (auto it = childNodes.begin(); it != childNodes.end(); ++it) {
		if ((*it)->getItem() == itemToRemove) {
			childNodes.erase(it);
			return true;
		}
	}
	return false;
}

bool SimpleSTRnode::removeChild(SimpleSTRnode *child) {
	for (auto it = childNodes.begin(); it != childNodes.end(); ++it) {
		if ((*it) == child) {
			childNodes.erase(it);
			return true;
		}
	}
	return false;
}

} // namespace strtree
} // namespace index
} // namespace geos
