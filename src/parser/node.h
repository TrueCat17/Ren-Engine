#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include "utils/string.h"


struct NodeProp {
	String pyExpr;
	String numLine;
};


class Node {
private:
	static std::vector<Node*> nodes;

	static bool initing;

	static void preloadImages(Node *node, int start, int count);
	static void jump(const String &label, bool isCall);


	String fileName;
	size_t numLine;

	String firstParam;
	std::map<String, NodeProp> props;
public:
	static void destroyAll();


	String command;
	int priority;

	String params;//text to out, command to execute, condition to check...
	String name;//for label

	Node *prevNode;//for blocks <elif> and <else>
	bool condIsTrue = false;

	String mainLabel;//for type == main

	std::vector<Node*> children;


	Node(String fileName, size_t numLine):
		fileName(fileName),
		numLine(numLine)
	{ nodes.push_back(this); }

	void execute();

	void initProp(const String &name, const String &value, size_t numLine);
	String getProp(const String &name, const String &commonName = "", const String &indexStr = "") const;
	String getPropCode(const String &name) const;

	const String& getFirstParam() const { return firstParam; }
	void setFirstParam(const String &value) { firstParam = value; }

	const String& getFileName() const { return fileName; }
	size_t getNumLine() const { return numLine; }
	String getPlace() const;
};

#endif // NODE_H
