#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

class Node;


class Parser {
private:
	std::vector<std::string> code;

	std::string dir;
	std::string fileName = "NO_FILE";
	size_t startFile = 0;

	size_t getNextStart(size_t start) const;

	Node* getNode(size_t start, size_t end, int superParent, bool isText);
	Node* getMainNode();
public:
	static bool isEvent(const std::string &type);
	static void getIsFakeOrIsProp(const std::string &type, bool &isFake, bool &isProp, bool &isEvent);

	Parser(const std::string &dir);
	Node* parse();
};

#endif // PARSER_H
