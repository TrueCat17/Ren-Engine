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


int String::toInt(std::string_view str) {
	if (str.empty()) return 0;

	bool neg = str[0] == '-';
	if (neg || str[0] == '+') {
		str = str.substr(1);
	}

	int res = 0;
	for (char c : str) {
		if (c >= '0' && c <= '9') {
			res = res * 10 + c - '0';
			continue;
		}

		Utils::outError("String::toInt", "String <%> is an invalid number", str);
		return 0;
	}
	return neg ? -res : res;
}


static double strToDoubleWithError(std::string_view str, bool &error) {
	error = true;

	if (str.empty()) return 0;

	bool wasSign = false;
	bool wasNumber = false;

	bool negRes = false;
	double res = 0;

	bool wasDot = false;
	int pow = 0;

	bool wasE = false;
	bool negE = false;
	int e = 0;

	for (char c : str) {
		if (!wasE && (c == 'e' || c == 'E')) {
			wasE = true;
			wasSign = false;
			wasNumber = false;
			continue;
		}

		if (!wasSign) {
			wasSign = true;

			if (c == '+') {
				continue;
			}
			if (c == '-') {
				if (!wasE) {
					negRes = true;
				}else {
					negE = true;
				}
				continue;
			}
		}

		if (!wasDot) {
			if (c == '.') {
				wasDot = true;
				continue;
			}
		}else {
			if (!wasE) {
				--pow;
			}
		}

		if (c >= '0' && c <= '9') {
			wasNumber = true;
			c -= '0';

			if (!wasE) {
				res = res * 10 + c;
			}else {
				e = e * 10 + c;
			}
			continue;
		}

		return 0;
	}
	if (!wasNumber) return 0;

	error = false;

	res = negRes ? -res : res;
	pow = pow + (negE ? -e : e);

	return res * std::pow(10, pow);
}

double String::toDouble(std::string_view str) {
	bool error;
	double res = strToDoubleWithError(str, error);

	if (error) {
		Utils::outError("String::toDouble", "String <%> is an invalid number", str);
	}
	return res;
}

bool String::isNumber(std::string_view str) {
	bool error;
	strToDoubleWithError(str, error);
	return !error;
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
	std::string res;
	if (strings.empty()) return res;

	size_t size = separator.size() * (strings.size() - 1);
	for (const std::string &s : strings) {
		size += s.size();
	}
	res.reserve(size);

	for (size_t i = 0; i < strings.size() - 1; ++i) {
		res += strings[i];
		res += separator;
	}
	res += strings.back();

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
