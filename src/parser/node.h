#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>

#include <Python.h>

#include "utils/string.h"


class Node {
private:
	Node(const String &fileName, size_t numLine, size_t id);
	~Node();

	String fileName;
	size_t numLine;

public:
	static String loadPath;

	static Node *getNewNode(const String &fileName, size_t numLine);
	static void destroyAll();

	static size_t preloadImages(const Node *parent, size_t start, size_t count);


	size_t id;//unique id for each Node
	int priority;//for init-block

	Node *parent;
	size_t childNum;//index in parent Node

	uint8_t countPropsToCalc;

	bool isScreenProp = false;     //xpos, ypos, pos, ...
	bool isScreenEvent = false;    //action, alternate, hover_sound...
	bool withScreenEvent = false;  //some child is screen event
	bool isScreenConst = false;    //have only const {children and props}?
	bool isScreenEnd = false;      //all children (not props) are const
	size_t screenNum = size_t(-1); //childNum, but skip screenConst children

	String command;//$, play, image, show, ...
	String params; //text to out, command to execute, condition to check, name for main/label/screen, ...

	std::vector<Node*> children;

	PyObject* getPyList() const;
	std::vector<String> getImageChildren() const;

	Node* getProp(const String &name);

	const String& getFileName() const { return fileName; }
	size_t getNumLine() const { return numLine; }
	String getPlace() const;
};

#endif // NODE_H
