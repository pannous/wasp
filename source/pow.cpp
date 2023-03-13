#import "pow.h"

/* Function to calculate x raised to the power y

    Time Complexity: O(n)
    Space Complexity: O(1)
    Algorithmic Paradigm: Divide and conquer.
*/
int power1(int x, unsigned int y) {
	if (y == 0)
		return 1;
	else if ((y % 2) == 0)
		return power1(x, y / 2) * power1(x, y / 2);
	else
		return x * power1(x, y / 2) * power1(x, y / 2);

}

/* Function to calculate x raised to the power y in O(logn)
    Time Complexity of optimized solution: O(logn)
*/
int power2(int x, unsigned int y) {
	int temp;
	if (y == 0)
		return 1;

	temp = power2(x, y / 2);
	if ((y % 2) == 0)
		return temp * temp;
	else
		return x * temp * temp;
}

/* Extended version of power function that can work
for float x and negative y
*/
float powerf(float x, int y) {
	float temp;
	if (y == 0)
		return 1;
	temp = powerf(x, y / 2);
	if ((y % 2) == 0) {
		return temp * temp;
	} else {
		if (y > 0)
			return x * temp * temp;
		else
			return (temp * temp) / x;
	}
}

/* Extended version of power function that can work
for double x and negative y
*/
double powerd(double x, int y) {
	double temp;
	if (y == 0)
		return 1;
	temp = powerd(x, y / 2);
	if ((y % 2) == 0) {
		return temp * temp;
	} else {
		if (y > 0)
			return x * temp * temp;
		else
			return (temp * temp) / x;
	}
}

unsigned long long factorial(int n) {
	unsigned long long fact = 1;
	if (n > 64)error1("long factorial limit 64!"); // overflow
	for (int i = 2; i <= n; i++) {
		fact *= i;
	}
	return fact;
}

double exp(double x, int precision) {
//	if(precision>50)precision=50; // numerical limit of long long
	double sum = 1.0f; // initialize sum of series
	for (int i = precision; i > 0; --i)
		sum = 1 + x * sum / i;
	return sum;
}

float ln(float y);// shitty, to do!
double log_e(double x, int precision) {
	if (x >= 2)return ln(x);
	if (x <= 0) return 0;// todo infinities
	auto k = x - 1;
	auto y = k;
	double res = y;
	for (int i = 2; i < precision; i++) {
		y *= -k;
		res += y / i;
	}
	return res;
}

double powerd(double x, double y) {
	return exp(y * log_e(x, 40));
}