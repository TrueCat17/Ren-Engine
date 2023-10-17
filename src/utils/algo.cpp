#include "algo.h"

#include "utils/string.h"
#include "utils/utils.h"

static size_t getStartArg(std::string_view args, size_t end, const char separator) {
	return args.find_first_not_of(separator, end + 1);
}
static size_t getEndArg(std::string_view args, size_t start, const char separator, bool *isValid) {
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
					std::string cStr(1, c);
					std::string pos = std::to_string(i);
					Utils::outMsg("Algo::getEndArg",
					              "Invalid bracket " + cStr + " at position " + pos + " in string:\n" +
					              std::string(args));
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
			            ) + " in string:\n" +
			            std::string(args);
			Utils::outMsg("Algo::getEndArg", error);
		}
	}
	return i;
}

std::vector<std::string> Algo::getArgs(std::string_view args, const char separator) {
	args = String::stripView(args);
	if (args.empty()) return {};

	std::vector<std::string> res;

	size_t start = 0;
	while (start != size_t(-1)) {
		size_t end = getEndArg(args, start, separator, nullptr);

		res.push_back(std::string(args.substr(start, end - start)));

		start = getStartArg(args, end, separator);
	}
	return res;
}

std::string Algo::clear(std::string_view s) {
	s = String::stripView(s);
	if (s.empty()) return {};

	std::string res;

	size_t start = s.find_first_not_of('(');
	if (start == size_t(-1)) {
		start = 0;
	}
	size_t end = s.find_last_not_of(')') + 1;
	if (!end) {
		end = s.size();
	}

	start = std::min(start, s.size() - end);
	end = s.size() - start;

	res = s.substr(start, end - start);
	return res;
}

bool Algo::isLexicalValid(std::string_view s) {
	bool res;
	getEndArg(s, 0, 0, &res);
	return res;
}

bool Algo::isValidPyName(std::string_view s) {
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
