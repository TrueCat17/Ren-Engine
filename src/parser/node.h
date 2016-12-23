#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include "utils/string.h"


class Node {
private:
	static std::vector<Node*> nodes;

	static bool initing;

	static void jump(const String &label);

public:
	static bool jumped;
	static void destroyAll();


	String firstParam;
	std::map<String, String> props;

	String command;
	int priority;

	String params;//text to out, command to execute, condition to check...
	String name;//for label

	Node *prevNode;//for blocks `elif` and `else`
	bool condIsTrue = false;

	String mainLabel;//for type == main

	std::vector<Node*> children;

	Node() { nodes.push_back(this); }
	void execute();

	String getProp(const String& name) const;
	String getPropCode(const String& name) const;
	String getFirstParam() const { return firstParam; }
};

#endif // NODE_H
