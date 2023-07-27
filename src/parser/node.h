#ifndef NODE_H
#define NODE_H

#include <inttypes.h>
#include <string>
#include <vector>

typedef struct _object PyObject;


class Node {
private:
	Node(const std::string &fileName, size_t numLine, size_t id);

	std::string fileName;
	size_t numLine;

public:
	static Node* getNewNode(const std::string &fileName, size_t numLine);
	static void destroyAll();

	static size_t preloadImages(const Node *parent, size_t start, size_t count);


	size_t id;//unique id for each Node
	double priority;//for init-block

	Node *parent;
	size_t childNum;//index in parent Node

	uint8_t countPropsToCalc;

	bool isScreenProp = false;     //xpos, ypos, pos, ...
	bool isScreenEvent = false;    //action, alternate, hover_sound...
	bool withScreenEvent = false;  //some child is screen event
	bool isScreenConst = false;    //have only const {children and props}?
	bool isScreenEnd = false;      //all children (not props) are const
	size_t screenNum = size_t(-1); //childNum, but skip screenConst children

	std::string command;//$, play, image, show, ...
	std::string params; //text to out, command to execute, condition to check, name for main/label/screen, ...

	//for screen vars (args)
	std::vector<std::pair<std::string, std::string>> vars;//elem = {varName, pyCodeForValue}

	std::vector<Node*> children;

	PyObject* getPyList() const;
	std::vector<std::string> getImageChildren() const;

	const Node *getProp(const std::string &name) const;

	const std::string& getFileName() const { return fileName; }
	size_t getNumLine() const { return numLine; }
	std::string getPlace() const;
};

#endif // NODE_H
