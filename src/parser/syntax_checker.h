#ifndef SYNTAXCHECKER_H
#define SYNTAXCHECKER_H

#include <vector>
#include <map>

#include <utils/string.h>

struct SuperParent {
	static const int NONE     = 0;
	static const int MAIN     = 1 << 0;
	static const int INIT     = 1 << 1;
	static const int LABEL    = 1 << 2;
	static const int SCREEN   = 1 << 3;
};


class SyntaxChecker {
public:
	static void init();
	static bool check(const String &parent, const String &child, const String &prevChild, const int superParent, bool &thereIsNot);

	static const std::vector<String>& getScreenProps(const String &type);
};

#endif // SYNTAXCHECKER_H
