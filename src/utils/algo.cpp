#include "algo.h"

#include "utils/utils.h"

size_t Algo::getStartArg(const std::string &args, size_t end, const char separator) {
	size_t res = end + 1;
	while (res < args.size() && args[res] == separator) {
		++res;
	}
	if (res < args.size()) return res;

	return size_t(-1);
}
size_t Algo::getEndArg(const std::string &args, size_t start, const char separator) {
	bool wasSeparator = false;
	int b1 = 0;//open (
	int b2 = 0;//open [
	bool q1 = false;//open '
	bool q2 = false;//open "

	size_t i = start;
	for (; i < args.size(); ++i) {
		const char c = args[i];
		wasSeparator = wasSeparator || (c == separator);

		if (wasSeparator && !b1 && !b2 && !q1 && !q2) break;

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
		}
	}
	if (i == args.size() && (b1 || b2 || q1 || q2)) {
		std::string error = "Not closed " +
		                    ((b1 || b2)
		                     ? std::string(b1 ? "round" : "square") + " bracket"
		                     : std::string(q1 ? "single" : "double") + " quote"
		                    );
		Utils::outMsg("Algo::getEndArg", error);
	}
	return i;
}

std::string Algo::clear(std::string s) {
	size_t start = s.find_first_not_of(' ');
	size_t end = s.find_last_not_of(' ') + 1;

	if (start == size_t(-1)) start = 0;
	if (!end) end = s.size();
	if (start || end != s.size()) {
		s = s.substr(start, end - start);
	}

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
		start = n;
		end = s.size() - n;
		s = s.substr(start, end - start);
	}

	return s;
}

std::vector<std::string> Algo::getArgs(std::string args, const char separator) {
	size_t start = args.find_first_not_of(' ');
	size_t end = args.find_last_not_of(' ') + 1;

	if (start == size_t(-1)) start = 0;
	if (!end) end = args.size();
	if (start || end != args.size()) {
		args = args.substr(start, end - start);
	}

	std::vector<std::string> res;

	start = 0;
	while (start != size_t(-1)) {
		end = Algo::getEndArg(args, start, separator);

		std::string t = args.substr(start, end - start);
		if (!t.empty()) {
			res.push_back(t);
		}

		start = Algo::getStartArg(args, end, separator);
	}
	return res;
}
