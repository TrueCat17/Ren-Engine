#ifndef STRING_H
#define STRING_H

#include <string>
#include <vector>

class String {
public:
	static std::string repeat(std::string_view str, size_t count);

	static int toInt(std::string_view str, int base = 10);
	static double toDouble(std::string_view str);

	static bool isNumber(std::string_view str);
	static bool isSimpleString(std::string_view str);

	static size_t firstNotInQuotes(std::string_view str, char c);

	static bool startsWith(std::string_view str, std::string_view substr);
	static bool endsWith(std::string_view str, std::string_view substr);

	static std::string_view stripView(std::string_view s, char space = ' ');
	static std::string strip(std::string_view s, char space = ' ');

	static void deleteAll(std::string &str, std::string_view toRemove);
	static void replaceAll(std::string &str, std::string_view from, std::string_view to);

	static std::vector<std::string> split(const std::string &str, std::string_view separator);
	static std::string join(const std::vector<std::string> &strings, std::string_view separator);

	static inline bool isFirstByte(const char c) {
		//second and next bytes in UTF-8 start from 10 in binary (0b10xxxxxx)
		return !(c & 0b10000000) || c & 0b01000000;
	}
	static size_t getCountBytes(const char first);

	static const std::string* getConstPtr(const std::string &str);
};

#endif // STRING_H
