#include "node.h"

#include <set>
#include <fstream>
#include <filesystem>

#include "gv.h"
#include "config.h"
#include "logger.h"

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/image_manipulator.h"
#include "media/music.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/game.h"
#include "utils/mouse.h"
#include "utils/utils.h"

String Node::loadPath;

static const size_t NODES_IN_PART = 10000;
static size_t countNodes = 0;
static std::vector<Node*> nodes;

Node::Node(const String &fileName, size_t numLine, size_t id):
	fileName(fileName),
	numLine(numLine),
	id(id)
{}
Node::~Node() {}

Node* Node::getNewNode(const String &fileName, size_t numLine) {
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
		const size_t size = i == nodes.size() - 1 ? (countNodes % NODES_IN_PART) : NODES_IN_PART;

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

	static const std::set<String> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = highLevelCommands.count(command);

	if (!isHighLevelCommands) {
		if (command) {
			PyList_Append(res, PyString_FromString(command.c_str()));
		}
		if (params) {
			PyList_Append(res, PyString_FromString(params.c_str()));
		}
	}

	for (const Node* child : children) {
		PyObject *childPyList = child->getPyList();

		static const std::set<String> blockCommandsInImage = {"contains", "block", "parallel"};
		bool childIsBlock = blockCommandsInImage.count(child->command);

		if (childIsBlock) {
			PyList_Append(res, childPyList);
		}else {
			size_t len = size_t(Py_SIZE(childPyList));

			std::vector<String> toJoin;
			toJoin.reserve(len);

			for (size_t i = 0; i < len; ++i) {
				PyObject *elem = PyList_GET_ITEM(childPyList, i);
				const char *chars = PyString_AS_STRING(elem);
				toJoin.push_back(String(chars));
			}

			String joinedStr = String::join(toJoin, ' ');
			PyObject *joined = PyString_FromString(joinedStr.c_str());

			PyList_Append(res, joined);
		}
	}

	return res;
}
std::vector<String> Node::getImageChildren() const {
	std::vector<String> res;

	static const std::vector<String> highLevelCommands = {"image", "scene", "show", "hide"};
	bool isHighLevelCommands = Algo::in(command, highLevelCommands);

	if (!isHighLevelCommands) {
		if (command) {
			res.push_back(command);
		}
		if (params) {
			res.push_back(params);
		}
	}

	for (const Node* child : children) {
		static const std::vector<String> blockCommandsInImage = {"contains", "block", "parallel"};
		bool childIsBlock = Algo::in(child->command, blockCommandsInImage);

		if (!childIsBlock) {
			if (child->command || child->params) {
				const String str = Algo::clear(child->command + " " + child->params);
				res.push_back(str);
			}
		}else {
			const std::vector<String> childChildren = child->getImageChildren();
			for (const String &str : childChildren) {
				res.push_back(str);
			}
		}
	}

	return res;
}



static void preloadImageAt(const std::vector<String> &children) {
	auto fileExists = [](const String &path) -> bool {
		if (std::filesystem::exists(path.c_str())) {
			return !std::filesystem::is_directory(path.c_str());
		}
		return false;
	};

	for (const String &str : children) {
		if (!str.isSimpleString()) continue;

		const String image = str.substr(1, str.size() - 2);

		if (Utils::imageWasRegistered(image)) {
			const String &imageName = image;

			const std::pair<String, size_t> place = Utils::getImagePlace(imageName);

			const String code = Utils::getImageCode(imageName);
			const String imagePath = PyUtils::exec(place.first, place.second, code, true);
			if (imagePath) {
				if (fileExists(imagePath)) {
					ImageManipulator::loadImage(imagePath);
				}
			}

			const std::vector<String> declAt = Utils::getVectorImageDeclAt(imageName);
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
		const String &childCommand = child->command;
		const String &childParams = child->params;

		if (childCommand == "with") {
			count = preloadImages(child, 0, count);
		}else

		if (childCommand == "jump" || childCommand == "call") {
			const String &label = childParams;

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



		if ((childCommand == "show" && !childParams.startsWith("screen ")) ||
			childCommand == "scene")
		{
			static const std::set<String> words = {"at", "with", "behind", "as"};

			--count;

			std::vector<String> args = Algo::getArgs(childParams);
			while (args.size() > 2 && words.count(args[args.size() - 2])) {
				args.erase(args.end() - 2, args.end());
			}
			if (args.empty()) continue;

			const String imageName = String::join(args, ' ');
			const String code = Utils::getImageCode(imageName);
			const String imageCode = PyUtils::exec(parent->getFileName(), parent->getNumLine(), code, true);
			if (imageCode) {
				ImageManipulator::loadImage(imageCode);
			}

			std::vector<String> declAt = Utils::getVectorImageDeclAt(imageName);
			preloadImageAt(declAt);
		}
	}

	return count;
}

Node* Node::getProp(const String &name) {
	for (Node *child : children) {
		if (child->command == name) {
			return child;
		}
	}
	return nullptr;
}

String Node::getPlace() const {
	return "Место объявления объекта <" + command + ">:\n"
			"  Файл <" + fileName + ">\n" +
			"  Номер строки: " + String(numLine);
}

