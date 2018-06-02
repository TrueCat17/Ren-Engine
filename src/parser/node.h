#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include "utils/string.h"


namespace boost {
	namespace python {
		class list;
	}
}
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
	static size_t countNodes;
	Node(const String &fileName, size_t numLine);
	~Node();


	static String jumpNextLabel;
	static bool jumpNextIsCall;

	static int preloadImages(const Node *parent, int start, int count);
	static void preloadImageAt(const std::vector<String> &children);

	static void jump(const String &label, bool isCall);


	String fileName;
	size_t numLine;

	String firstParam;
	std::map<String, NodeProp> props;
	bool alwaysNonePropCode = false;
public:
	static bool loading;
	static size_t stackDepth;
	static std::vector<std::pair<String, String>> stack;

	static void jumpNext(const std::string &label, bool isCall);

	static Node *getNewNode(const String &fileName, size_t numLine);
	static void destroyAll();


	int childNum;//index in parent Node
	int priority;

	String command;
	String params;//text to out, command to execute, condition to check...

	String loadPath;//for main, path to save
	String name;//for label, screen, main (mod name)

	Node *prevNode;//for blocks <elif> and <else>
	bool condIsTrue = false;

	std::vector<Node*> children;


	void execute();

	py::list getPyList() const;
	std::vector<String> getImageChildren() const;

	void initProp(const String &name, const String &value, size_t numLine);
	void updateAlwaysNonePropCode();
	NodeProp getPropCode(const String &name, const String &commonName = "", const String &indexStr = "") const;

	const String& getFirstParam() const { return firstParam; }
	void setFirstParam(const String &value) { firstParam = value; }

	const String& getFileName() const { return fileName; }
	size_t getNumLine() const { return numLine; }
	String getPlace() const;
};

#endif // NODE_H
