#include "string.h"

#include <iomanip>
#include <sstream>

#include "utils/utils.h"


String::String(int i, int base) {
	std::stringstream ss;
	ss << std::setbase(base) << i;
	ss >> *this;
}
String::String(double d) {
	std::stringstream ss;
	ss << d;
	ss >> *this;
}

std::vector<String> String::split(const String &separator) const {
	std::vector<String> res;

	int prev = -separator.size();
	int n;

	int start;
	int end;
	String t;

	while ((n = find(separator, prev + separator.size())) != -1) {
		start = prev + separator.size();
		end = n;

		t = substr(start, end - start);
		res.push_back(t);

		prev = n;
	}

	start = prev + separator.size();
	end = n;

	t = substr(start, end - start);
	res.push_back(t);

	return res;
}

int String::toInt(int base) const {
	if (empty()) return 0;

	int res = 0;
	bool neg = at(0) == '-';
	for (size_t i = neg; i < size(); ++i) {
		char c = at(i);

		if (c >= '0' && c <= '9') c -= '0';
		else if (c >= 'a' && c <= 'z' && base > c - 'a') c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'Z' && base > c - 'A') c = c - 'A' + 10;
		else {
			c = 0;
			Utils::outMsg("String::toInt", "Строка '" + *this + "' не является корректным числом в " + String(base) + "-ричной системе счисления");
			return 0;
		}

		res = res * base + c;
	}
	if (neg) {
		res *= -1;
	}

	return res;
}
double String::toDouble() const {
	if (empty()) return 0;


	static std::map<String, double> cache;

	auto i = cache.find(*this);
	if (i != cache.end()) {
		return i->second;
	}

	std::stringstream ss;
	ss << *this;

	double res;
	ss >> res;

	if (cache.size() < 100000) {
		cache[*this] = res;
	}
	return res;
}

bool String::isNumber() const {
	if (empty()) return false;

	size_t start = 0;
	char f = at(start);
	while (f == ' ' || f == '\t') {
		++start;
		f = at(start);
	}

	if (f == '-' || f == '+') {
		++start;
		if (start == size()) return false;
	}

	bool res = true;

	bool wasDot = false;
	int wasE = -1;
	for (size_t i = start; i < size(); ++i) {
		char c = at(i);
		if (c == '.') {
			if (wasDot) {
				res = false;
				break;
			}
			wasDot = true;
			continue;
		}
		if (c == 'e') {
			if (wasE != -1) {
				res = false;
				break;
			}
			wasE = i;
			continue;
		}
		if (c == '-' || c == '+') {
			if (wasE != -1 && wasE == int(i) - 1) {
				continue;
			}
			res = false;
			break;
		}

		if (c < '0' || c > '9') {
			res = false;
			break;
		}
	}

	return res;
}
bool String::isSimpleString() const {
	if (size() < 2) return false;

	char f = front();
	if (f != '\'' && f != '"') return false;
	char b = back();
	if (f != b) return false;

	for (size_t i = 1; i < size() - 1; ++i) {
		char c = at(i);
		if (c == f || c == '\\') return false;
	}
	return true;
}

bool String::startsWith(const String &str, bool withSpaces) const {
	if (size() < str.size()) return false;

	size_t k = 0;
	if (!withSpaces) {
		while (k < size() && (at(k) == ' ' || at(k) == '\t')) {
			++k;
		}
		if (k == size()) {
			return str.empty();
		}
	}

	for (size_t i = 0; i < str.size(); ++i) {
		char c = at(i + k);
		if (c != str.at(i)) {
			return false;
		}
	}
	return true;
}
bool String::endsWith(const String &str) const {
	if (size() < str.size()) return false;

	for (size_t i = 0; i < str.size(); ++i) {
		if (at(size() - i - 1) != str.at(str.size() - i - 1)) {
			return false;
		}
	}
	return true;
}

void String::deleteAll(const String &str) {
	size_t i;
	while((i = find(str)) != size_t(-1)) {
		erase(i, str.size());
	}
}
void String::replaceAll(const String &from, const String &to) {
	size_t i;
	while((i = find(from)) != size_t(-1)) {
		erase(i, from.size());
		insert(i, to);
	}
}


String String::join(const std::vector<String> &strings, const String &separator) {
	String res;

	for (size_t i = 0; i < strings.size(); ++i) {
		res += strings[i];

		if (i != strings.size() - 1) {
			res += separator;
		}
	}
	return res;
}
