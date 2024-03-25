#ifndef MATH_H
#define MATH_H

#include <cfloat>
#include <cmath>

class Math {
public:
	static void init();
	static float getSin(int angle);
	static float getCos(int angle);

	static bool floatsAreEq(float a, float b) {
		return std::abs(a - b) < FLT_EPSILON;
	}
	static bool doublesAreEq(double a, double b) {
		return std::abs(a - b) < DBL_EPSILON;
	}

	template<typename T, typename MIN, typename MAX>
	static T inBounds(T value, MIN min, MAX max) {
		return (value < T(min)) ? T(min) : (value > T(max)) ? T(max) : value;
	}
};

#endif // MATH_H
