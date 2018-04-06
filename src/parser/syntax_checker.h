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

struct SyntaxPart {
	std::vector<String> prevs;
	int superParent;

	inline bool check(const String &prevChild, const int superParent) const {
		if (!(this->superParent & superParent)) return false;

		if (prevs.empty()) return true;

		for (const String &prev : prevs) {
			if (prev == prevChild) return true;
		}
		return false;
	}
};


class SyntaxChecker {
private:
	static std::map<String, std::map<String, SyntaxPart>> mapSyntax;

	static void addBlockChildren(const String &parents, const String &childs);
	static void setSuperParents(const String &nodesStr, const int superParent);

public:
	static void init();
	static bool check(const String &parent, const String &child, const String &prevChild, const int superParent, bool &thereIsNot);
};

#endif // SYNTAXCHECKER_H
