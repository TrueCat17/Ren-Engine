#ifndef NODE_H
#define NODE_H

#include <inttypes.h>
#include <string>
#include <vector>

typedef struct _object PyObject;


class Node {
private:
	Node(const std::string &fileName, uint32_t numLine, uint32_t id);

	const std::string* fileName;
	uint32_t numLine;

public:
	static Node* getNewNode(const std::string &fileName, uint32_t numLine);
	static void destroyAll();

	static size_t preloadImages(const Node *parent, size_t start, size_t count);


	uint32_t id;//unique id for each Node
	float priority;//for init-block

	uint32_t childNum;//index in parent Node
	Node *parent;

	uint8_t countPropsToCalc;

	bool isScreenProp:1;     //xpos, ypos, pos, ...
	bool isScreenEvent:1;    //action, alternate, hover_sound...
	bool withScreenEvent:1;  //some child is screen event
	bool isScreenConst:1;    //have only const {children and props}?
	bool isScreenEnd:1;      //all children (not props) are const
	uint32_t screenNum = uint32_t(-1); //childNum, but skip screenConst children

	std::string command;//$, play, image, show, ...
	std::string params; //text to out, command to execute, condition to check, name for main/label/screen, ...

	//for screen vars (args)
	std::vector<std::pair<std::string, std::string>> vars;//elem = {varName, pyCodeForValue}

	std::vector<Node*> children;

	PyObject* getPyList() const;
	std::vector<std::string> getImageChildren() const;

	const Node *getProp(const std::string &name) const;

	const std::string& getFileName() const { return *fileName; }
	uint32_t getNumLine() const { return numLine; }
	std::string getPlace() const;
};

#endif // NODE_H
