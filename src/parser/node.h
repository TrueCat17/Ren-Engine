#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include <Python.h>

#include "utils/string.h"


class Node {
private:
	static std::vector<Node*> nodes;
	static size_t countNodes;
	Node(const String &fileName, size_t numLine, size_t id);
	~Node();


	static String jumpNextLabel;
	static bool jumpNextIsCall;

	static int preloadImages(const Node *parent, int start, int count);
	static void preloadImageAt(const std::vector<String> &children);

	static void jump(const String &label, bool isCall);


	String fileName;
	size_t numLine;

public:
	static String loadPath;
	static bool loading;

	static size_t stackDepth;
	static std::vector<std::pair<String, String>> stack;

	static void jumpNext(const std::string &label, bool isCall);

	static Node *getNewNode(const String &fileName, size_t numLine);
	static void destroyAll();



	size_t id;//unique id for each Node
	int priority;//for init-block

	Node *parent;
	size_t childNum;//index in parent Node

	Node *prevNode;//for blocks <elif> and <else>
	bool condIsTrue = false;

	uint8_t countPropsToCalc;

	bool isScreenProp = false;       //xpos, ypos, pos, ...
	bool isScreenEvent = false;      //action, alternate, hover_sound...
	bool isScreenConst = false;      //have only const {children and props}?
	bool isScreenEnd = false;        //all children (not props) are const
	size_t screenNum   = size_t(-1); //childNum, but skip screenConst children

	String command;//$, play, image, show, ...
	String params; //text to out, command to execute, condition to check, name for main/label/screen, ...

	std::vector<Node*> children;


	void execute();

	PyObject* getPyList() const;
	std::vector<String> getImageChildren() const;

	Node* getProp(const String &name);

	const String& getFileName() const { return fileName; }
	size_t getNumLine() const { return numLine; }
	String getPlace() const;
};

#endif // NODE_H
