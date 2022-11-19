#include "postgis/lwgeom_dump.hpp"

#include "liblwgeom/gserialized.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"
#include "liblwgeom/lwinline.hpp"
#include "libpgcommon/lwgeom_pg.hpp"

#include <cstring>
#include <string>

namespace duckdb {

std::vector<GSERIALIZED *> dump_recursive(LWGEOM *lwgeom);

std::vector<GSERIALIZED *> dump_recursive(LWGEOM *lwgeom) {
	if (lwgeom_is_empty(lwgeom))
		return {};

	if (!lwgeom_is_collection(lwgeom)) {
		auto ret = geometry_serialize(lwgeom);
		return {ret};
	}

	std::vector<GSERIALIZED *> ret;
	LWCOLLECTION *lwcoll = (LWCOLLECTION *)lwgeom;
	uint32_t i;
	LWGEOM *subgeom;

	for (i = 0; i < lwcoll->ngeoms; i++) {
		subgeom = lwcoll->geoms[i];
		auto subVec = dump_recursive(subgeom);
		for (auto gser : subVec) {
			ret.push_back(gser);
		}
	}

    return ret;
}

std::vector<GSERIALIZED *> LWGEOM_dump(GSERIALIZED *geom) {
	LWGEOM *lwgeom;

	lwgeom = lwgeom_from_gserialized(geom);

	/* Return nothing for empties */
	if (lwgeom_is_empty(lwgeom))
		return {};
    
	auto ret = dump_recursive(lwgeom);
    lwgeom_free(lwgeom);

    return ret;
}

} // namespace duckdb
