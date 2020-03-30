#include "string.h"

#include "utils/utils.h"


std::string String::repeat(const std::string &str, size_t count) {
	size_t size = str.size() * count;
	if (!size) return "";
	
	std::string res;
	res.resize(size);
	
	char *dst = &res[0];
	const char *srcEnd = str.data() + str.size();
	
	while (count--) {
		const char *src = str.data();
		while (src != srcEnd) {
			*dst++ = *src++;
		}
	}
	
	return res;
}

std::vector<std::string> String::split(const std::string &str, const std::string &separator) {
	size_t prev = -separator.size();
	size_t n;
	size_t start;
	
	size_t count = 1;
	while ((n = str.find(separator, prev + separator.size())) != size_t(-1)) {
		start = prev + separator.size();
		prev = n;

		++count;
	}

	std::vector<std::string> res;
	res.reserve(count);

	prev = -separator.size();
	while ((n = str.find(separator, prev + separator.size())) != size_t(-1)) {
		start = prev + separator.size();
		res.push_back(str.substr(start, n - start));

		prev = n;
	}
	
	start = prev + separator.size();
	res.push_back(str.substr(start));
	
	return res;
}

int String::toInt(const std::string &str, int base) {
	if (str.empty()) return 0;
	
	int res = 0;
	bool neg = str[0] == '-';
	for (size_t i = neg; i < str.size(); ++i) {
		char c = str[i];
		
		if (c >= '0' && c <= '9') c -= '0';
		else if (c >= 'a' && c <= 'z' && base > c - 'a') c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'Z' && base > c - 'A') c = c - 'A' + 10;
		else {
			c = 0;
			Utils::outMsg("String::toInt",
			              "String <" + str + "> is invalid number in numeral system with base " + std::to_string(base));
			return 0;
		}
		
		res = res * base + c;
	}
	if (neg) {
		res *= -1;
	}
	
	return res;
}
double String::toDouble(const std::string &str) {
	char *end;
	double res = strtod(str.c_str(), &end);
	if (end != str.c_str() + str.size()) {
		Utils::outMsg("String::toDouble", "Could not to convert <" + str + "> to double");
	}

	return res;
}

bool String::isNumber(const std::string &str) {
	size_t start = 0;
	char f = str[start];
	while (start < str.size() && (f == ' ' || f == '\t')) {
		++start;
		f = str[start];
	}
	if (start == str.size()) return false;
	
	if (f == '-' || f == '+') {
		++start;
		if (start == str.size()) return false;
	}
	
	bool wasDot = false;
	bool wasE = false;
	for (size_t i = start; i < str.size(); ++i) {
		const char c = str[i];
		if (c == '.') {
			if (wasDot) return false;
			wasDot = true;
		}else
		if (c == 'e') {
			if (wasE) return false;
			wasE = true;

			if (i + 1 == str.size()) return false;
			const char n = str[i + 1];
			if (n == '-' || n == '+') {
				if (i + 2 == str.size()) return false;
				++i;
			}
		}else
		if (c < '0' || c > '9') {
			return false;
		}
	}
	
	return true;
}
bool String::isSimpleString(const std::string &str) {
	if (str.size() < 2) return false;
	
	char f = str.front();
	if (f != '\'' && f != '"') return false;
	char b = str.back();
	if (f != b) return false;

	for (size_t i = 1; i < str.size() - 1; ++i) {
		char c = str[i];
		if (c == f || c == '\\') return false;
	}
	return true;
}

size_t String::firstNotInQuotes(const std::string &str, char c) {
	bool q1 = false;
	bool q2 = false;
	
	char prev = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		char t = str[i];
		
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

bool String::startsWith(const std::string &str, const std::string &substr, bool skipSpaces) {
	if (str.size() < substr.size()) return false;
	
	size_t k = 0;
	if (skipSpaces) {
		while (k < str.size() && (str[k] == ' ' || str[k] == '\t')) {
			++k;
		}
		if (k == str.size()) {
			return str.empty();
		}
	}
	
	for (size_t i = 0; i < substr.size(); ++i) {
		if (str[i + k] != substr[i]) {
			return false;
		}
	}
	return true;
}
bool String::endsWith(const std::string &str, const std::string &substr) {
	if (str.size() < substr.size()) return false;
	
	for (size_t i = 0; i < substr.size(); ++i) {
		if (str[str.size() - i - 1] != substr[substr.size() - i - 1]) {
			return false;
		}
	}
	return true;
}

void String::deleteAll(std::string &str, const std::string &toRemove) {
	size_t i;
	while((i = str.find(toRemove)) != size_t(-1)) {
		str.erase(i, toRemove.size());
	}
}
void String::replaceAll(std::string &str, const std::string &from, const std::string &to) {
	size_t i = -to.size();
	while((i = str.find(from, i + to.size())) != size_t(-1)) {
		str.erase(i, from.size());
		str.insert(i, to);
	}
}


std::string String::join(const std::vector<std::string> &strings, const std::string &separator) {
	std::string res;
	
	size_t size = separator.size() * (strings.size() - 1);
	for (const std::string &s : strings) {
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
