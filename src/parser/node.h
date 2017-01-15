#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include "utils/string.h"


class Node {
private:
	static std::vector<Node*> nodes;

	static bool initing;

	static void preloadImages(Node *node, int start, int count);
	static void jump(const String &label);

	std::map<String, String> props;
public:
	static bool jumped;
	static void destroyAll();


	String firstParam;

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

	void initProp(const String &name, const String &value);
	String getProp(const String& name, const String &commonName = "", const String &indexStr = "") const;
	String getPropCode(const String& name) const;
	const String& getFirstParam() const { return firstParam; }
};

#endif // NODE_H
