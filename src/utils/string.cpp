#include "string.h"

#include <iomanip>
#include <sstream>

#include "utils/utils.h"


String::String(int i, int base) {
	if (base == 10) {
		*this = std::to_string(i);
	}else {
		std::stringstream ss;
		ss << std::setbase(base) << i;
		ss >> *this;
	}
}
String::String(size_t i, int base) {
	if (base == 10) {
		*this = std::to_string(i);
	}else {
		std::stringstream ss;
		ss << std::setbase(base) << i;
		ss >> *this;
	}
}


String String::repeat(size_t count) const {
	String res;
	res.resize(size() * count);

	char *dst = res.data();
	const char *srcEnd = data() + size();

	while (count--) {
		const char *src = data();
		while (src != srcEnd) {
			*dst++ = *src++;
		}
	}

	return res;
}

std::vector<String> String::split(const String &separator) const {
	std::vector<String> res;

	size_t prev = -separator.size();
	size_t n;

	size_t start;
	size_t end;

	while ((n = find(separator, prev + separator.size())) != size_t(-1)) {
		start = prev + separator.size();
		end = n;

		res.push_back(substr(start, end - start));

		prev = n;
	}

	start = prev + separator.size();
	end = n;
	res.push_back(substr(start, end - start));

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
	return std::atof(c_str());
}

bool String::isNumber() const {
	size_t start = 0;
	char f = (*this)[start];
	while (start < size() && (f == ' ' || f == '\t')) {
		++start;
		f = (*this)[start];
	}
	if (start == size()) return false;

	if (f == '-' || f == '+') {
		++start;
		if (start == size()) return false;
	}

	bool wasDot = false;
	bool wasE = false;
	for (size_t i = start; i < size(); ++i) {
		const char c = (*this)[i];
		if (c == '.') {
			if (wasDot) return false;
			wasDot = true;
		}else
		if (c == 'e') {
			if (wasE) return false;
			wasE = true;

			if (i + 1 == size()) return false;
			const char n = (*this)[i + 1];
			if (n == '-' || n == '+') {
				if (i + 2 == size()) return false;
				++i;
			}
		}else
		if (c < '0' || c > '9') {
			return false;
		}
	}

	return true;
}
bool String::isSimpleString() const {
	if (size() < 2) return false;

	char f = front();
	if (f != '\'' && f != '"') return false;
	char b = back();
	if (f != b) return false;

	for (size_t i = 1; i < size() - 1; ++i) {
		char c = (*this)[i];
		if (c == f || c == '\\') return false;
	}
	return true;
}

size_t String::firstNotInQuotes(char c) const {
	bool q1 = false;
	bool q2 = false;

	char prev = 0;
	for (size_t i = 0; i < size(); ++i) {
		char t = (*this)[i];

		if (t == '\'' && !q2 && prev != '\\') q1 = !q1;
		if (t == '"'  && !q1 && prev != '\\') q2 = !q2;

		if (!q1 && !q2 && t == c) {
			return i;
		}

		if (prev == '\\' && t == '\\') {
			prev = 0;
		}else {
			prev = t;
		}
	}

	return size_t(-1);
}

bool String::startsWith(const String &str, bool withSpaces) const {
	if (size() < str.size()) return false;

	size_t k = 0;
	if (!withSpaces) {
		while (k < size() && ((*this)[k] == ' ' || (*this)[k] == '\t')) {
			++k;
		}
		if (k == size()) {
			return str.empty();
		}
	}

	for (size_t i = 0; i < str.size(); ++i) {
		if ((*this)[i + k] != str[i]) {
			return false;
		}
	}
	return true;
}
bool String::endsWith(const String &str) const {
	if (size() < str.size()) return false;

	for (size_t i = 0; i < str.size(); ++i) {
		if ((*this)[size() - i - 1] != str[str.size() - i - 1]) {
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
	size_t i = -to.size();
	while((i = find(from, i + to.size())) != size_t(-1)) {
		erase(i, from.size());
		insert(i, to);
	}
}


String String::join(const std::vector<String> &strings, const String &separator) {
	String res;

	size_t size = separator.size() * (strings.size() - 1);
	for (const String &s : strings) {
		size += s.size();
	}
	res.reserve(size);

	for (size_t i = 0; i < strings.size(); ++i) {
		res += strings[i];

		if (i != strings.size() - 1) {
			res += separator;
		}
	}
	return res;
}
