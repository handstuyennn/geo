#include "liblwgeom/liblwgeom_internal.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace duckdb {

/*
 * TODO rewrite all this stuff to be based on a truly topological model
 */

struct struct_psurface_arcs {
	double ax, ay, az;
	double bx, by, bz;
	uint32_t cnt, face;
};
typedef struct struct_psurface_arcs *psurface_arcs;

/* We supposed that the geometry is valid
we could have wrong result if not */
int lwpsurface_is_closed(const LWPSURFACE *psurface) {
	uint32_t i, j, k;
	uint32_t narcs, carc;
	int found;
	psurface_arcs arcs;
	POINT4D pa, pb;
	LWPOLY *patch;

	/* If surface is not 3D, it's can't be closed */
	if (!FLAGS_GET_Z(psurface->flags))
		return 0;

	/* If surface is less than 4 faces hard to be closed too */
	if (psurface->ngeoms < 4)
		return 0;

	/* Max theoretical arcs number if no one is shared ... */
	for (i = 0, narcs = 0; i < psurface->ngeoms; i++) {
		patch = (LWPOLY *)psurface->geoms[i];
		narcs += patch->rings[0]->npoints - 1;
	}

	arcs = (psurface_arcs)lwalloc(sizeof(struct struct_psurface_arcs) * narcs);
	for (i = 0, carc = 0; i < psurface->ngeoms; i++) {

		patch = (LWPOLY *)psurface->geoms[i];
		for (j = 0; j < patch->rings[0]->npoints - 1; j++) {

			getPoint4d_p(patch->rings[0], j, &pa);
			getPoint4d_p(patch->rings[0], j + 1, &pb);

			/* remove redundant points if any */
			if (pa.x == pb.x && pa.y == pb.y && pa.z == pb.z)
				continue;

			/* Make sure to order the 'lower' point first */
			if ((pa.x > pb.x) || (pa.x == pb.x && pa.y > pb.y) || (pa.x == pb.x && pa.y == pb.y && pa.z > pb.z)) {
				pa = pb;
				getPoint4d_p(patch->rings[0], j, &pb);
			}

			for (found = 0, k = 0; k < carc; k++) {

				if ((arcs[k].ax == pa.x && arcs[k].ay == pa.y && arcs[k].az == pa.z && arcs[k].bx == pb.x &&
				     arcs[k].by == pb.y && arcs[k].bz == pb.z && arcs[k].face != i)) {
					arcs[k].cnt++;
					found = 1;

					/* Look like an invalid PolyhedralSurface
					      anyway not a closed one */
					if (arcs[k].cnt > 2) {
						lwfree(arcs);
						return 0;
					}
				}
			}

			if (!found) {
				arcs[carc].cnt = 1;
				arcs[carc].face = i;
				arcs[carc].ax = pa.x;
				arcs[carc].ay = pa.y;
				arcs[carc].az = pa.z;
				arcs[carc].bx = pb.x;
				arcs[carc].by = pb.y;
				arcs[carc].bz = pb.z;
				carc++;

				/* Look like an invalid PolyhedralSurface
				      anyway not a closed one */
				if (carc > narcs) {
					lwfree(arcs);
					return 0;
				}
			}
		}
	}

	/* A polyhedron is closed if each edge
	       is shared by exactly 2 faces */
	for (k = 0; k < carc; k++) {
		if (arcs[k].cnt != 2) {
			lwfree(arcs);
			return 0;
		}
	}
	lwfree(arcs);

	/* Invalid Polyhedral case */
	if (carc < psurface->ngeoms)
		return 0;

	return 1;
}

} // namespace duckdb
