#ifndef STRING_H
#define STRING_H

#include <string>
#include <vector>

class String: public std::string {
public:
	String() {}
	String(char c): std::string(1, c) {}
	String(const char *s): std::string(s) {}
	String(int i, int base = 10);
	String(size_t i, int base = 10);
	String(double d): std::string(std::to_string(d)) {}

	String(const std::string &str): std::string(str) {}
	String(std::string &&str): std::string(str) {}


	template<typename T>
	String operator+(const T& t) const {
		return static_cast<const std::string&>(*this) + String(t);
	}
	template<typename T>
	String& operator+=(const T& t) {
		static_cast<std::string&>(*this) += String(t);
		return *this;
	}
	template<typename T>
	String& operator=(const T& t) {
		static_cast<std::string&>(*this) = String(t);
		return *this;
	}

	operator bool() const { return !empty(); }
	int toInt(int base = 10) const;
	double toDouble() const;

	bool isNumber() const;
	bool isSimpleString() const;

	template <typename T>
	bool contains(const T &sub) const { return find(sub) != size_t(-1); }

	bool startsWith(const String &str, bool withSpaces = true) const;
	bool endsWith(const String &str) const;

	void deleteAll(const String &str);
	void replaceAll(const String &from, const String &to);

	std::vector<String> split(const String &separator) const;
	static String join(const std::vector<String> &strings, const String &separator);
};

#endif // STRING_H
