#include "math.h"

//#define M_PI 3.14159265358979323846

#include <math.h>
#include <float.h>

static double *sins;
void Math::init() {
	sins = new double[91]();
	for (int i = 0; i <= 90; ++i) {
		sins[i] = sin(i * (M_PI / 180));
	}
}

double Math::getSin(int i) {
	if (!i) return 0;

	i = i % 360;
	if (i < 0) {
		i += 360;
	}

	double k = i < 180 ? 1 : -1;
	i = i % 180;

	return sins[i < 90 ? i : 180 - i] * k;
}

double Math::getCos(int i) {
	if (!i) return 1;

	i = (i - 90) % 360;
	if (i < 0) {
		i += 360;
	}

	double k = i > 180 ? 1 : -1;
	i = i % 180;

	return sins[i < 90 ? i : 180 - i] * k;
}

bool Math::doublesAreEq(double a, double b) {
	return (a - b) < DBL_EPSILON;
}
