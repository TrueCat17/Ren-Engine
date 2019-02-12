#ifndef STRING_H
#define STRING_H

#include <string>
#include <vector>

class String {
public:
	static std::string repeat(const std::string &str, size_t count);

	static int toInt(const std::string &str, int base = 10);
	static double toDouble(const std::string &str);

	static bool isNumber(const std::string &str);
	static bool isSimpleString(const std::string &str);

	static size_t firstNotInQuotes(const std::string &str, char c);

	static bool startsWith(const std::string &str, const std::string &substr, bool skipSpaces = false);
	static bool endsWith(const std::string &str, const std::string &substr);

	static void deleteAll(std::string &str, const std::string &toRemove);
	static void replaceAll(std::string &str, const std::string &from, const std::string &to);

	static std::vector<std::string> split(const std::string &str, const std::string &separator);
	static std::string join(const std::vector<std::string> &strings, const std::string &separator);
};

#endif // STRING_H
