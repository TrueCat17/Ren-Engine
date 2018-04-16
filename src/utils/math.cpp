#include "math.h"

#include <math.h>

double* Math::sins = new double[360];
double* Math::coss = new double[360];

void Math::init() {
	for (unsigned int i = 0; i < 360; ++i) {
		sins[i] = std::sin(i * M_PI / 180);
		coss[i] = std::cos(i * M_PI / 180);
	}
}
