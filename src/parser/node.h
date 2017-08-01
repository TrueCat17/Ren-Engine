#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include <boost/python.hpp>

#include "utils/string.h"

namespace py = boost::python;

struct NodeProp {
	String pyExpr;
	size_t numLine;
};


class Node {
private:
	static std::vector<Node*> nodes;

	static void preloadImages(Node *node, int start, int count);
	static void jump(const String &label, bool isCall);


	String fileName;
	size_t numLine;

	String firstParam;
	std::map<String, NodeProp> props;
public:
	static bool loading;
	static size_t stackDepth;
	static std::vector<std::pair<String, String>> stack;

	static void destroyAll();


	size_t childNum;

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
		numLine(numLine),
		childNum(0)
	{ nodes.push_back(this); }

	void execute();

	py::list getPyList() const;

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
