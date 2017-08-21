#ifndef STRING_H
#define STRING_H

#include <string>
#include <vector>
#include <map>

class String: public std::string {
public:
	String() {}
	String(char c) { char a[2] = {c, 0}; *this = a; }
	String(const char *s): std::string(s) {}
	String(const std::string &str): std::string(str) {}
	String(double d);
	String(int i, int base = 10);
	String(size_t i): String(double(i)) {}

	template<typename T>
	String operator+(const T& t) const {
		String res = *this;
		return res += String(t);
	}
	template<typename T>
	String& operator+=(const T& t) {
		const String tStr = String(t);
		*(dynamic_cast<std::string*>(this)) += tStr.c_str();

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
