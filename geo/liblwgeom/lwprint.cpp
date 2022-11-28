/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * PostGIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * PostGIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostGIS.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 *
 * Copyright (C) 2010-2015 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 *
 **********************************************************************/

#include "lib/ryu.hpp"
#include "liblwgeom/liblwgeom_internal.hpp"

namespace duckdb {

/*
 * Print an ordinate value using at most **maxdd** number of decimal digits
 * The actual number of printed decimal digits may be less than the
 * requested ones if out of significant digits.
 *
 * The function will write at most OUT_DOUBLE_BUFFER_SIZE bytes, including the
 * terminating NULL.
 * It returns the number of bytes written (exluding the final NULL)
 *
 */
int lwprint_double(double d, int maxdd, char *buf) {
	int length;
	double ad = fabs(d);
	int precision = FP_MAX(0, maxdd);

	if (ad <= OUT_MIN_DOUBLE || ad >= OUT_MAX_DOUBLE) {
		length = d2sexp_buffered_n(d, precision, buf);
	} else {
		length = d2sfixed_buffered_n(d, precision, buf);
	}
	buf[length] = '\0';

	return length;
}

} // namespace duckdb
