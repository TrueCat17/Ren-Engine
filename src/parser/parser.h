#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <map>

#include <boost/python.hpp>
namespace py = boost::python;

#include "utils/string.h"

class Node;


class Parser {
private:
	static void initScreenNode(Node *node);


	std::vector<String> code;

	String dir;
	String fileName = "NO_FILE";
	size_t startFile = 0;
	size_t pythonIndent = -1;

	size_t getNextStart(size_t start) const;

	Node* getNode(size_t start, size_t end, int superParent, bool isText);
	Node* getMainNode();
public:
	static py::dict getMods();

	Parser(const String &dir);
	Node* parse();
};

#endif // PARSER_H
