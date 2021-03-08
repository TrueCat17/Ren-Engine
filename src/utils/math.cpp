#include "math.h"

#include <math.h>
#include <float.h>

static float *sins;
void Math::init() {
	const double PI = atan(1) * 4;

	sins = new float[91]();
	for (int i = 0; i <= 90; ++i) {
		sins[i] = float(sin(i * (PI / 180)));
	}
}

float Math::getSin(int i) {
	if (!i) return 0;

	i = i % 360;
	if (i < 0) {
		i += 360;
	}

	float k = i < 180 ? 1 : -1;
	i = i % 180;

	return sins[i < 90 ? i : 180 - i] * k;
}

float Math::getCos(int i) {
	if (!i) return 1;

	i = (i - 90) % 360;
	if (i < 0) {
		i += 360;
	}

	float k = i > 180 ? 1 : -1;
	i = i % 180;

	return sins[i < 90 ? i : 180 - i] * k;
}

bool Math::floatsAreEq(float a, float b) {
	return abs(a - b) < FLT_EPSILON;
}
bool Math::doublesAreEq(double a, double b) {
	return abs(a - b) < DBL_EPSILON;
}
