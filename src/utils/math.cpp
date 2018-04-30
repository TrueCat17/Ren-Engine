#include "math.h"

#include <math.h>

double* Math::sins = new double[360];
double* Math::coss = new double[360];

void Math::init() {
	for (unsigned int i = 0; i < 360; ++i) {
		//on MinGW sin&cos not in std
		using namespace std;
		sins[i] = sin(i * M_PI / 180);
		coss[i] = cos(i * M_PI / 180);
	}
}
