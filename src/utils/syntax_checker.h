#ifndef SYNTAXCHECKER_H
#define SYNTAXCHECKER_H

#include <algorithm>
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
	String name;
	std::vector<String> prevs;
	int superParent;

	bool check(const String &prevChild, const int superParent) const {
		return (this->superParent & superParent)
				&&
			   (prevs.empty() || std::find(prevs.begin(), prevs.end(), prevChild) != prevs.end());
	}
};


class SyntaxChecker {
private:
	static std::map<String, std::vector<SyntaxPart>> mapSyntax;
	static void addBlockChildren(const String &parents, const String &childs, const bool clear = false);
	static void setSuperParents(const String &nodesStr, const int superParent);

public:
	static void init();
	static bool check(const String &parent, const String &child, const String &prevChild, const int superParent, bool &thereIsNot);
};

#endif // SYNTAXCHECKER_H
