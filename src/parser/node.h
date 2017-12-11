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

	String styleName;
	String propName;


	static NodeProp initPyExpr(const String &pyExpr, size_t numLine) {
		NodeProp res;
		res.pyExpr = pyExpr;
		res.numLine = numLine;
		return res;
	}
	static NodeProp initStyleProp(const String &styleName, const String &propName) {
		NodeProp res;
		res.styleName = styleName;
		res.propName = propName;
		return res;
	}
};


class Node {
private:
	static std::vector<Node*> nodes;

	static int preloadImages(const Node *parent, int start, int count);
	static void preloadImageAt(const std::vector<String> &children);

	static void jump(const String &label, bool isCall);


	String fileName;
	size_t numLine;

	String firstParam;
	std::map<String, NodeProp> props;
public:
	static bool loading;
	static size_t stackDepth;
	static std::vector<std::pair<String, String>> stack;

	static const String mainLabel;

	static void destroyAll();


	size_t childNum;

	String command;
	int priority;

	//for main
	String loadPath;

	String params;//text to out, command to execute, condition to check...
	String name;//for label

	Node *prevNode;//for blocks <elif> and <else>
	bool condIsTrue = false;

	std::vector<Node*> children;


	Node(const String &fileName, size_t numLine):
		fileName(fileName),
		numLine(numLine),
		childNum(0)
	{ nodes.push_back(this); }

	void execute();

	py::list getPyList() const;
	std::vector<String> getImageChildren() const;

	void initProp(const String &name, const String &value, size_t numLine);
	NodeProp getPropCode(const String &name, const String &commonName = "", const String &indexStr = "") const;

	const String& getFirstParam() const { return firstParam; }
	void setFirstParam(const String &value) { firstParam = value; }

	const String& getFileName() const { return fileName; }
	size_t getNumLine() const { return numLine; }
	String getPlace() const;
};

#endif // NODE_H
