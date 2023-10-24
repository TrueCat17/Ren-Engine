#ifndef SYNTAXCHECKER_H
#define SYNTAXCHECKER_H

#include <string>
#include <vector>

struct SuperParent {
	static const int NONE     = 0;
	static const int MAIN     = 1 << 0;
	static const int INIT     = 1 << 1;
	static const int LABEL    = 1 << 2;
	static const int SCREEN   = 1 << 3;
	static const int TL_STRS  = 1 << 4;
};


class SyntaxChecker {
public:
	static void init();
	static bool check(const std::string &parent, const std::string &child, const std::string &prevChild, const int superParent, bool &isText);

	static bool isKnownScreenLeaf(const std::string &name);//xpos, alpha, pass, python...
	static const std::vector<std::string>& getScreenProps(const std::string &type);
};

#endif // SYNTAXCHECKER_H
