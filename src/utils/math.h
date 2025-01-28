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
		double max = std::max(1.0f, std::max(std::abs(a) , std::abs(b)));
		return std::abs(a - b) <= FLT_EPSILON * max;
	}
	static bool doublesAreEq(double a, double b) {
		double max = std::max(1.0, std::max(std::abs(a) , std::abs(b)));
		return std::abs(a - b) <= DBL_EPSILON * max;
	}

	template<typename T, typename MIN, typename MAX>
	static T inBounds(T value, MIN min, MAX max) {
		return (value < T(min)) ? T(min) : (value > T(max)) ? T(max) : value;
	}
};

#endif // MATH_H
