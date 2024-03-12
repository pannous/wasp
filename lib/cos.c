/* origin: FreeBSD /usr/src/lib/msun/src/k_cos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * is preserved.
 * ====================================================
 */
static const double
		C1 = 4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
C2 = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
C3 = 2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
C4 = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
C5 = 2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
C6 = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */


// * kernel cos function on [-pi/4, pi/4], pi/4 ~ 0.785398164
// * Input x is assumed to be bounded by ~pi/4 in magnitude.
// * Input y is the tail of x.
double __cos(double x, double y) {
	double hz, z, r, w;

	z = x * x;
	w = z * z;
	r = z * (C1 + z * (C2 + z * C3)) + w * w * (C4 + z * (C5 + z * C6));
	hz = 0.5 * z;
	w = 1.0 - hz;
	return w + (((1.0 - w) - hz) + (z * r - x * y));
}
