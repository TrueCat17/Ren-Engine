#include "math.h"

#include <math.h>
#include <float.h>

double* Math::sins = nullptr;
double* Math::coss = nullptr;

void Math::init() {
	sins = new double[360 * 2];
	coss = new double[360 * 2];

	for (size_t i = 0; i < 360; ++i) {
		//on MinGW sin&cos not in std
		using namespace std;
		sins[i] = sins[i + 360] = sin(i * M_PI / 180);
		coss[i] = coss[i + 360] = cos(i * M_PI / 180);
	}

	//to sins[-1], that eq to sins[359]
	sins += 360;
	coss += 360;
}

bool Math::doublesAreEq(double a, double b) {
	return (a - b) < DBL_EPSILON;
}
