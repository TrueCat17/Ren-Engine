#ifndef ALGO_H
#define ALGO_H

#include <vector>
#include <string>

#include <SDL3/SDL_rect.h>

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

	static bool pointInRect(int x, int y, const SDL_Rect &rect) {
		if (x < rect.x) return false;
		if (y < rect.y) return false;
		if (x >= rect.x + rect.w) return false;
		if (y >= rect.y + rect.h) return false;
		return true;
	}
	static bool pointInRect(float x, float y, const SDL_FRect &rect) {
		if (x < rect.x) return false;
		if (y < rect.y) return false;
		if (x >= rect.x + rect.w) return false;
		if (y >= rect.y + rect.h) return false;
		return true;
	}
};

#endif // ALGO_H
