/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Crunchy Data
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <cmath>
#include <geos/math/DD.hpp>

namespace geos {
namespace math { // geos.util

/* public */
void DD::selfAdd(const DD &y) {
	return selfAdd(y.hi, y.lo);
}

/* public */
void DD::selfAdd(double yhi, double ylo) {
	double H, h, T, t, S, s, e, f;
	S = hi + yhi;
	T = lo + ylo;
	e = S - hi;
	f = T - lo;
	s = S - e;
	t = T - f;
	s = (yhi - e) + (hi - s);
	t = (ylo - f) + (lo - t);
	e = s + T;
	H = S + e;
	h = e + (S - H);
	e = t + h;

	double zhi = H + e;
	double zlo = e + (H - zhi);
	hi = zhi;
	lo = zlo;
	return;
}

/* public */
void DD::selfAdd(double y) {
	double H, h, S, s, e, f;
	S = hi + y;
	e = S - hi;
	s = S - e;
	s = (y - e) + (hi - s);
	f = s + lo;
	H = S + f;
	h = f + (S - H);
	hi = H + h;
	lo = h + (H - hi);
	return;
}

/* public */
DD operator+(const DD &lhs, const DD &rhs) {
	DD rv(lhs.hi, lhs.lo);
	rv.selfAdd(rhs);
	return rv;
}

/* public */
void DD::selfSubtract(const DD &d) {
	return selfAdd(-1 * d.hi, -1 * d.lo);
}

/* public */
void DD::selfSubtract(double p_hi, double p_lo) {
	return selfAdd(-1 * p_hi, -1 * p_lo);
}

/* public */
void DD::selfSubtract(double y) {
	return selfAdd(-1 * y, 0.0);
}

/* public */
DD operator-(const DD &lhs, const DD &rhs) {
	DD rv(lhs.hi, lhs.lo);
	rv.selfSubtract(rhs);
	return rv;
}

/* public */
void DD::selfMultiply(double yhi, double ylo) {
	double hx, tx, hy, ty, C, c;
	C = SPLIT * hi;
	hx = C - hi;
	c = SPLIT * yhi;
	hx = C - hx;
	tx = hi - hx;
	hy = c - yhi;
	C = hi * yhi;
	hy = c - hy;
	ty = yhi - hy;
	c = ((((hx * hy - C) + hx * ty) + tx * hy) + tx * ty) + (hi * ylo + lo * yhi);
	double zhi = C + c;
	hx = C - zhi;
	double zlo = c + hx;
	hi = zhi;
	lo = zlo;
	return;
}

/* public */
void DD::selfMultiply(DD const &d) {
	return selfMultiply(d.hi, d.lo);
}

/* public */
void DD::selfMultiply(double y) {
	return selfMultiply(y, 0.0);
}

/* public */
DD operator*(const DD &lhs, const DD &rhs) {
	DD rv(lhs.hi, lhs.lo);
	rv.selfMultiply(rhs);
	return rv;
}

} // namespace math
} // namespace geos
