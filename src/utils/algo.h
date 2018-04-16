#ifndef ALGO_H
#define ALGO_H

#include "utils/string.h"

class Algo {
public:

	template<typename T, typename C>
	static bool in(const T &value, const C &container) {
		for (const T &t : container) {
			if (t == value) return true;
		}
		return false;
	}

	static inline bool isFirstByte(const char c) {
		//2-й и последующие байты в 2-чном представлении в UTF-8 начинаются с 10 (0b10xxxxxx)
		return !(c & 0b10000000) || c & 0b01000000;
	}

	static size_t getStartArg(const String &args, size_t end, const char separator = ' ');
	static size_t getEndArg(const String &args, size_t start, const char separator = ' ');
	static String clear(String s);
	static std::vector<String> getArgs(String args, const char separator = ' ');

};

#endif // ALGO_H
