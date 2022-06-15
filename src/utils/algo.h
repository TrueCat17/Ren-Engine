#ifndef ALGO_H
#define ALGO_H

#include <vector>
#include <string>

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
		//second and next bytes in UTF-8 start from 10 in binary (0b10xxxxxx)
		return !(c & 0b10000000) || c & 0b01000000;
	}

	static size_t getStartArg(const std::string &args, size_t end, const char separator = ' ');
	static size_t getEndArg(const std::string &args, size_t start, const char separator = ' ', bool *isValid = nullptr);
	static std::string clear(std::string s);
	static std::vector<std::string> getArgs(std::string args, const char separator = ' ');

	static bool isLexicalValid(std::string s);
	static bool isValidPyName(const std::string &s);
};

#endif // ALGO_H
