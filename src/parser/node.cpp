#include "node.h"

#include <set>

#include <Python.h>

#include "gv.h"
#include "media/image_manipulator.h"
#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/string.h"
#include "utils/utils.h"

static const size_t NODES_IN_PART = 5000;
static size_t countNodes = 0;
static std::vector<Node*> nodes;

Node::Node(const std::string &fileName, size_t numLine, size_t id):
	fileName(fileName),
	numLine(numLine),
	id(id)
{}

Node* Node::getNewNode(const std::string &fileName, size_t numLine) {
	if ((countNodes % NODES_IN_PART) == 0) {
		nodes.push_back((Node*)::malloc(NODES_IN_PART * sizeof(Node)));
	}

	Node *node = nodes.back() + (countNodes % NODES_IN_PART);
	new(node) Node(fileName, numLine, countNodes);
	++countNodes;

	return node;
}
void Node::destroyAll() {
	for (size_t i = 0; i < nodes.size(); ++i) {
		const size_t size = (i == nodes.size() - 1) ? (countNodes % NODES_IN_PART) : NODES_IN_PART;

		Node *array = nodes[i];
		for (size_t j = 0; j < size; ++j) {
			array[j].~Node();//destructor
		}
		::free(array);
	}
	countNodes = 0;
	nodes.clear();
}



PyObject* Node::getPyList() const {
	PyObject *res = PyList_New(0);

	static const std::set<std::string> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = highLevelCommands.count(command);

	if (!isHighLevelCommands) {
		if (!command.empty()) {
			PyList_Append(res, PyString_FromString(command.c_str()));
		}
		if (!params.empty()) {
			PyList_Append(res, PyString_FromString(params.c_str()));
		}
	}

	for (const Node* child : children) {
		PyObject *childPyList = child->getPyList();

		static const std::set<std::string> blockCommandsInImage = {"contains", "block", "parallel"};
		bool childIsBlock = blockCommandsInImage.count(child->command);

		if (childIsBlock) {
			PyList_Append(res, childPyList);
		}else {
			size_t len = size_t(Py_SIZE(childPyList));

			std::vector<std::string> toJoin;
			toJoin.reserve(len);

			for (size_t i = 0; i < len; ++i) {
				PyObject *elem = PyList_GET_ITEM(childPyList, i);
				const char *chars = PyString_AS_STRING(elem);
				toJoin.push_back(std::string(chars));
			}

			std::string joinedStr = String::join(toJoin, " ");
			PyObject *joined = PyString_FromString(joinedStr.c_str());

			PyList_Append(res, joined);
		}
	}

	return res;
}
std::vector<std::string> Node::getImageChildren() const {
	std::vector<std::string> res;

	static const std::vector<std::string> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = Algo::in(command, highLevelCommands);

	if (!isHighLevelCommands) {
		if (!command.empty()) {
			res.push_back(command);
		}
		if (!params.empty()) {
			res.push_back(params);
		}
	}

	for (const Node* child : children) {
		static const std::vector<std::string> blockCommandsInImage = {"contains", "block", "parallel"};
		bool childIsBlock = Algo::in(child->command, blockCommandsInImage);

		if (childIsBlock) {
			const std::vector<std::string> childChildren = child->getImageChildren();
			for (const std::string &str : childChildren) {
				res.push_back(str);
			}
		}else {
			if (!child->command.empty() || !child->params.empty()) {
				const std::string str = Algo::clear(child->command + " " + child->params);
				res.push_back(str);
			}
		}
	}

	return res;
}



static void preloadImageAt(const std::vector<std::string> &children) {
	auto fileExists = [](const std::string &path) -> bool {
		return FileSystem::exists(path) && !FileSystem::isDirectory(path);
	};

	for (const std::string &str : children) {
		if (!String::isSimpleString(str)) continue;

		const std::string image = str.substr(1, str.size() - 2);

		if (Utils::imageWasRegistered(image)) {
			const std::vector<std::string> declAt = Utils::getVectorImageDeclAt(image);
			preloadImageAt(declAt);
		}else {
			if (fileExists(image)) {
				ImageManipulator::loadImage(image);
			}
		}
	}
}
size_t Node::preloadImages(const Node *parent, size_t start, size_t count) {
	for (size_t i = start; i < parent->children.size() && count; ++i) {
		const Node *child = parent->children[i];
		const std::string &childCommand = child->command;
		const std::string &childParams = child->params;

		if (childCommand == "with") {
			count = preloadImages(child, 0, count);
		}else

		if (childCommand == "jump" || childCommand == "call") {
			const std::string &label = childParams;

			for (size_t j = 0; j < GV::mainExecNode->children.size(); ++j) {
				Node *t = GV::mainExecNode->children[j];
				if (t->command == "label" && t->params == label) {
					if (childCommand == "jump") {
						return preloadImages(child, 0, count - 1);
					}else {
						--count;
						preloadImages(child, 0, 1);
					}
				}
			}
		}else

		if (childCommand == "if" || childCommand == "elif" || childCommand == "else") {
			preloadImages(child, 0, 1);
		}else



		if ((childCommand == "show" && !String::startsWith(childParams, "screen ")) ||
			childCommand == "scene")
		{
			static const std::set<std::string> words = {"at", "with", "behind", "as"};

			--count;

			std::vector<std::string> args = Algo::getArgs(childParams);
			while (args.size() > 2 && words.count(args[args.size() - 2])) {
				args.erase(args.end() - 2, args.end());
			}
			if (args.empty()) continue;

			const std::string imageName = String::join(args, " ");

			std::vector<std::string> declAt = Utils::getVectorImageDeclAt(imageName);
			preloadImageAt(declAt);
		}
	}

	return count;
}

Node* Node::getProp(const std::string &name) const {
	for (Node *child : children) {
		if (child->command == name) {
			return child;
		}
	}
	return nullptr;
}

std::string Node::getPlace() const {
	return "Object <" + command + "> declared in:\n"
	        "  File <" + fileName + ">\n" +
	        "  Line: " + std::to_string(numLine);
}

