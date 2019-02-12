#ifndef MATH_H
#define MATH_H

class Math {
private:
	static double* sins;
	static double* coss;

public:
	static void init();

	static double getSin(int angle) { return sins[((angle % 360) + 360) % 360]; }
	static double getCos(int angle) { return coss[((angle % 360) + 360) % 360]; }

	template<typename T, typename MIN, typename MAX>
	static T inBounds(T value, MIN min, MAX max) { return (value < T(min)) ? min : (value > T(max)) ? max : value; }
};

#endif // MATH_H
