#ifndef MATH_H
#define MATH_H

class Math {
public:
	static void init();
	static double getSin(int angle);
	static double getCos(int angle);

	static bool doublesAreEq(double a, double b);

	template<typename T, typename MIN, typename MAX>
	static T inBounds(T value, MIN min, MAX max) { return (value < T(min)) ? min : (value > T(max)) ? max : value; }
};

#endif // MATH_H
