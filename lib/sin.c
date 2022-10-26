double pi = 3.141592653589793;

double mod_d(double x, double y) {
	return x - trunc(x / y) * y;
}

double sin(double x) {
	double tau = 6.283185307179586;// 2*pi
	// double pi_fourth=0.7853981633974483;
	double z, r, v, w;
	double
			S1 = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
	S2 = 8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
	S3 = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
	S4 = 2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
	S5 = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
	S6 = 1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */
	x = mod_d(x, tau);
	// if(x<0) return -sin(x);
	// if(x%tau > pi) return -sin(x%tau);
	z = x * x;
	w = z * z;
	r = S2 + z * (S3 + z * S4) + z * w * (S5 + z * S6);
	return x + z * x * (S1 + z * r);
}

double cos(double x) {
	double pi_half = 1.5707963267948966;
	return sin(x + pi / 2);// todo: eval compiler!
}

void test_sin() {
	assert(sin(0) == 0);
	assert(sin(pi / 2) == 1);
	assert(sin(pi) == 0);
	assert(sin(3 * pi / 2) == -1);
	assert(sin(2 * pi) == 0);

	assert(cos(-pi / 2 + 0) == 0);
	assert(cos(-pi / 2 + pi / 2) == 1);
	assert(cos(-pi / 2 + pi) == 0);
	assert(cos(-pi / 2 + 3 * pi / 2) == -1);
	assert(cos(-pi / 2 + 2 * pi) == 0);
}