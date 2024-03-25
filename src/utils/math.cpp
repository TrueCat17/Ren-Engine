#include "math.h"

static float *sins;
void Math::init() {
	const double PI = std::atan(1) * 4;

	sins = new float[91]();
	for (int i = 0; i <= 90; ++i) {
		sins[i] = float(std::sin(i * (PI / 180)));
	}
}

float Math::getSin(int i) {
	if (!i) return 0;

	i = i % 360;
	if (i < 0) {
		i += 360;
	}

	float k;
	if (i < 180) {
		k = 1;
	}else {
		k = -1;
		i -= 180;
	}

	return sins[i < 90 ? i : 180 - i] * k;
}

float Math::getCos(int i) {
	if (!i) return 1;

	i = (i + 90) % 360;
	if (i < 0) {
		i += 360;
	}

	float k;
	if (i < 180) {
		k = 1;
	}else {
		k = -1;
		i -= 180;
	}

	return sins[i < 90 ? i : 180 - i] * k;
}
