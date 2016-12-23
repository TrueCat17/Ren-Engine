#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <map>

#include "parser/node.h"

class Parser {
private:
	static void initScreenNode(Node *node);


	std::vector<String> code;

	size_t getNextStart(size_t start) const;
	Node* getNode(size_t start, size_t end, int superParent, bool isText);
	Node* getMainNode();
public:
	static std::map<String, String> getModNamesAndLabels();

	Parser(String dir);
	Node* parse();
};

#endif // PARSER_H
