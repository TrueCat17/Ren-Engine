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

	static std::string clear(std::string_view s);
	static std::vector<std::string> getArgs(std::string_view args, const char separator = ' ');

	static bool isLexicalValid(std::string_view s);
	static bool isValidPyName(std::string_view s);
};

#endif // ALGO_H
