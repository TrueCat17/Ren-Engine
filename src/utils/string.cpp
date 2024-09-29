#include "string.h"

#include "utils/utils.h"


std::string String::repeat(std::string_view str, size_t count) {
	std::string res;
	res.reserve(str.size() * count);

	for (size_t i = 0; i < count; ++i) {
		res += str;
	}
	return res;
}

std::vector<std::string> String::split(const std::string &str, std::string_view separator) {
	size_t prev = 0;
	size_t n;
	
	size_t count = 1;
	while ((n = str.find(separator, prev)) != size_t(-1)) {
		++count;
		prev = n + separator.size();
	}

	std::vector<std::string> res;
	res.reserve(count);

	prev = 0;
	while ((n = str.find(separator, prev)) != size_t(-1)) {
		res.push_back(str.substr(prev, n - prev));
		prev = n + separator.size();
	}
	res.push_back(str.substr(prev));
	
	return res;
}

int String::toInt(std::string_view str, int base) {
	if (str.empty()) return 0;

	int res = 0;
	bool neg = str[0] == '-';
	for (size_t i = neg; i < str.size(); ++i) {
		char c = str[i];
		
		if (c >= '0' && c <= '9') c -= '0';
		else if (c >= 'a' && c <= 'z' && base > c - 'a' + 10) c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'Z' && base > c - 'A' + 10) c = c - 'A' + 10;
		else {
			c = 0;
			std::string s(str);
			std::string baseStr = std::to_string(base);
			Utils::outMsg("String::toInt",
			              "String <" + s + "> is invalid number in numeral system with base " + baseStr);
			return 0;
		}
		
		res = res * base + c;
	}
	return neg ? -res : res;
}
double String::toDouble(std::string_view str) {
	char *end;
	double res = strtod(str.data(), &end);
	if (end == str.data() + str.size()) return res;

	Utils::outMsg("String::toDouble",
	              "Failed to convert <" + std::string(str) + "> to double");
	return 0;
}

bool String::isNumber(std::string_view str) {
	if (str.empty()) return false;
	
	if (str[0] == '-' || str[0] == '+') {
		str = str.substr(1);
		if (str.empty()) return false;
	}
	
	bool wasDot = false;
	bool wasE = false;
	for (size_t i = 0; i < str.size(); ++i) {
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
bool String::isSimpleString(std::string_view str) {
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

size_t String::firstNotInQuotes(std::string_view str, char c) {
	bool q1 = false;
	bool q2 = false;
	
	char prev = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		char t = str[i];
		
		if (prev != '\\') {
			if (t == '\'' && !q2) q1 = !q1;
			if (t == '"'  && !q1) q2 = !q2;
		}

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

bool String::startsWith(std::string_view str, std::string_view substr) {
	if (str.size() < substr.size()) return false;
	
	if (str.size() > substr.size()) {
		str = str.substr(0, substr.size());
	}
	return str == substr;
}
bool String::endsWith(std::string_view str, std::string_view substr) {
	if (str.size() < substr.size()) return false;

	if (str.size() > substr.size()) {
		str = str.substr(str.size() - substr.size());
	}
	return str == substr;
}

std::string_view String::stripView(std::string_view s, char space) {
	size_t start = s.find_first_not_of(space);
	if (start == size_t(-1)) return {};

	size_t end = s.find_last_not_of(space) + 1;
	return s.substr(start, end - start);
}
std::string String::strip(std::string_view s, char space) {
	s = stripView(s, space);
	return std::string(s);
}

void String::deleteAll(std::string &str, std::string_view toRemove) {
	size_t prev = 0;
	size_t i;
	while ((i = str.find(toRemove, prev)) != size_t(-1)) {
		str.erase(i, toRemove.size());
		prev = i;
	}
}
void String::replaceAll(std::string &str, std::string_view from, std::string_view to) {
	size_t i = 0;
	while ((i = str.find(from, i)) != size_t(-1)) {
		str.erase(i, from.size());
		str.insert(i, to);
		i += to.size();
	}
}


std::string String::join(const std::vector<std::string> &strings, std::string_view separator) {
	size_t size = separator.size() * (strings.size() - 1);
	for (const std::string &s : strings) {
		size += s.size();
	}

	std::string res;
	res.reserve(size);

	for (size_t i = 0; i < strings.size() - 1; ++i) {
		res += strings[i];
		res += separator;
	}
	if (!strings.empty()) {
		res += strings.back();
	}

	return res;
}

size_t String::getCountBytes(const char first) {
	bool isAscii = !(first & (1 << 7));
	if (isAscii) return 1;

	bool b2 = first & (1 << 6);
	bool b3 = first & (1 << 5);
	bool b4 = first & (1 << 4);
	if (b2) {
		if (b3) {
			if (b4) return 4;
			return 3;
		}
		return 2;
	}
	return 4;//error, <first> is not first byte of symbol
}
size_t String::getCorrectCropIndex(std::string_view s, size_t i) {
	size_t t = i;
	while (!String::isFirstByte(s[t])) {
		--t;
	}
	size_t last = String::getCountBytes(s[t]);
	if (t + last > i) {
		last = 0;
	}
	return t + last;
}


using StringArray = std::vector<std::string>;
const size_t STRING_ARRAY_SIZE = 128;

std::vector<StringArray*> stringArrays;

const std::string* String::getConstPtr(const std::string &str) {
	for (const StringArray *array : stringArrays) {
		for (const std::string &elem : *array) {
			if (elem == str) return &elem;
		}
	}

	if (stringArrays.empty() || stringArrays.back()->size() == STRING_ARRAY_SIZE) {
		StringArray *array = new StringArray();
		array->reserve(STRING_ARRAY_SIZE);
		stringArrays.push_back(array);
	}
	StringArray *array = stringArrays.back();
	array->push_back(str);

	return &array->back();
}
