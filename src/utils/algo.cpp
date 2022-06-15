#include "algo.h"

#include "utils/string.h"
#include "utils/utils.h"

size_t Algo::getStartArg(const std::string &args, size_t end, const char separator) {
	size_t res = end + 1;
	while (res < args.size() && args[res] == separator) {
		++res;
	}
	if (res < args.size()) return res;

	return size_t(-1);
}
size_t Algo::getEndArg(const std::string &args, size_t start, const char separator, bool *isValid) {
	if (isValid) {
		*isValid = true;
	}

	int b1 = 0;//open (
	int b2 = 0;//open [
	int b3 = 0;//open {
	bool q1 = false;//open '
	bool q2 = false;//open "

	size_t i = start;
	for (; i < args.size(); ++i) {
		const char c = args[i];
		if (!b1 && !b2 && !b3 && !q1 && !q2 && c == separator) {
			break;
		}

		if (c == '\'' && !q2) {
			q1 = !q1;
		}else

		if (c == '"' && !q1) {
			q2 = !q2;
		}else

		if (!q1 && !q2) {
			     if (c == '(') ++b1;
			else if (c == ')') --b1;
			else if (c == '[') ++b2;
			else if (c == ']') --b2;
			else if (c == '{') ++b3;
			else if (c == '}') --b3;

			if (b1 < 0 || b2 < 0 || b3 < 0) {
				if (b1 < 0) b1 = 0;
				if (b2 < 0) b2 = 0;
				if (b3 < 0) b3 = 0;

				if (isValid) {
					*isValid = false;
				}else {
					Utils::outMsg("Algo::getEndArg",
					              std::string("Invalid bracket ") + c + " in position " + std::to_string(i) + " for string:\n" + args);
				}
			}
		}
	}
	if (i == args.size() && (b1 || b2 || b3 || q1 || q2)) {
		if (isValid) {
			*isValid = false;
		}else {
			std::string error = "Not closed " +
			            ((b1 || b2 || b3)
			             ? std::string(b1 ? "(" :  b2 ? "[" : "{") + " bracket"
			             : std::string(q1 ? "single" : "double") + " quote"
			            );
			Utils::outMsg("Algo::getEndArg", error);
		}
	}
	return i;
}

std::string Algo::clear(std::string s) {
	s = String::strip(s);

	size_t n = 0;
	while (n < s.size() && s[n] == '(') {
		if (s[s.size() - n - 1] == ')') {
			++n;
		}else{
			Utils::outMsg("Algo::clear", "Invalid brackets:\n<" + s + ">");
			return nullptr;
		}
	}

	if (n) {
		size_t start = n;
		size_t end = s.size() - n;
		s = s.substr(start, end - start);
	}

	return s;
}

std::vector<std::string> Algo::getArgs(std::string args, const char separator) {
	args = String::strip(args);

	std::vector<std::string> res;

	size_t start = 0;
	while (start != size_t(-1)) {
		size_t end = Algo::getEndArg(args, start, separator);

		std::string t = args.substr(start, end - start);
		if (!t.empty()) {
			res.push_back(t);
		}

		start = Algo::getStartArg(args, end, separator);
	}
	return res;
}

bool Algo::isLexicalValid(std::string s) {
	char separator = '\n';
	if (s.find(separator) != size_t(-1)) {
		return false;
	}

	s += separator;
	bool res;
	getEndArg(s, 0, separator, &res);

	return res;
}

bool Algo::isValidPyName(const std::string &s) {
	if (s.empty()) return false;
	if (s[0] >= '0' && s[0] <= '9') return false;

	for (size_t i = 0; i < s.size(); ++i) {
		const char c = s[i];

		if (c >= '0' && c <= '9') continue;
		if (c >= 'a' && c <= 'z') continue;
		if (c >= 'A' && c <= 'Z') continue;
		if (c == '_') continue;

		return false;
	}

	return true;
}
