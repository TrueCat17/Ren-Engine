#ifndef PARSER_H
#define PARSER_H

#include <vector>

#include <Python.h>

#include "utils/string.h"

class Node;


class Parser {
private:
	std::vector<String> code;

	String dir;
	String fileName = "NO_FILE";
	size_t startFile = 0;
	size_t pythonIndent = size_t(-1);

	size_t getNextStart(size_t start) const;

	Node* getNode(size_t start, size_t end, int superParent, bool isText);
	Node* getMainNode();
public:
	static bool isEvent(const String &type);
	static void getIsFakeOrIsProp(const String &type, bool &isFake, bool &isProp, bool &isEvent);
	static PyObject* getMods();

	Parser(const String &dir);
	Node* parse();
};

#endif // PARSER_H
