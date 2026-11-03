#ifndef PARSER_H
#define PARSER_H

#include <inttypes.h>
#include <vector>
#include <string>

class Node;
typedef struct _object PyObject;


class Parser {
private:
	std::vector<std::string> code;

	std::string dir;
	std::string fileName = "NO_FILE";
	uint32_t startFile = 0;

	uint32_t getNextStart(uint32_t start) const;

	Node* getNode(uint32_t start, uint32_t end, int superParent, bool isText);
	Node* getMainNode();
public:
	static void getIsFakeOrIsProp(const std::string &type, bool &isFake, bool &isProp, bool &isEvent);

	static PyObject* parseFile(const std::string &path);

	Parser(const std::string &dir, bool justOneFile = false);
	Node* parse();
};

#endif // PARSER_H
