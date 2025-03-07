#include "node.h"

#include <Python.h>

#include "gv.h"

#include "media/image_manipulator.h"
#include "media/py_utils/convert_to_py.h"
#include "media/sprite.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/string.h"

static const uint32_t NODES_IN_PART = 5000;
static uint32_t countNodes = 0;
static std::vector<Node*> nodes;

Node::Node(const std::string &fileName, uint32_t numLine, uint32_t id):
    fileName(String::getConstPtr(fileName)),
    numLine(numLine),
    id(id),
    isScreenProp(false),
    isScreenEvent(false),
    withScreenEvent(false),
    isScreenConst(false),
    isScreenEnd(false)
{}

Node* Node::getNewNode(const std::string &fileName, uint32_t numLine) {
	if ((countNodes % NODES_IN_PART) == 0) {
		void *ptr = ::malloc(NODES_IN_PART * sizeof(Node));
		nodes.push_back(reinterpret_cast<Node*>(ptr));
	}

	Node *node = nodes.back() + (countNodes % NODES_IN_PART);
	new(node) Node(fileName, numLine, countNodes);
	++countNodes;

	return node;
}
void Node::destroyAll() {
	for (size_t i = 0; i < nodes.size(); ++i) {
		const uint32_t size = (i == nodes.size() - 1) ? (countNodes % NODES_IN_PART) : NODES_IN_PART;

		Node *array = nodes[i];
		for (uint32_t j = 0; j < size; ++j) {
			array[j].~Node();//destructor
		}
		::free(array);
	}
	countNodes = 0;
	nodes.clear();
}


static void setElem(PyObject *tuple, Py_ssize_t &index, PyObject *obj, const Node *node) {
	PyObject *line = PyTuple_New(3);
	PyTuple_SET_ITEM(line, 0, obj);
	PyTuple_SET_ITEM(line, 1, convertToPy(node->getFileName()));
	PyTuple_SET_ITEM(line, 2, convertToPy(node->getNumLine()));

	PyTuple_SET_ITEM(tuple, index++, line);
}
PyObject* Node::getPyChildren(bool isRoot) const {
	Py_ssize_t count = Py_ssize_t(children.size());
	if (!isRoot) {
		++count;
		if (!params.empty()) {
			++count;
		}
	}

	PyObject *res = PyTuple_New(count);
	Py_ssize_t index = 0;

	if (!isRoot) {
		setElem(res, index, convertToPy(command), this);
		if (!params.empty()) {
			setElem(res, index, convertToPy(params), this);
		}
	}

	for (const Node* child : children) {
		PyObject *obj;

		bool childIsBlock = Algo::in(std::string_view(child->command), blockCommandsInImage);
		if (childIsBlock) {
			obj = child->getPyChildren(false);
		}else {
			obj = convertToPy(Algo::clear(child->command + ' ' + child->params));
		}

		setElem(res, index, obj, child);
	}

	return res;
}



size_t Node::preloadImages(const Node *parent, size_t start, size_t count) {
	for (size_t i = start; i < parent->children.size() && count; ++i) {
		const Node *child = parent->children[i];
		const std::string &childCommand = child->command;
		const std::string &childParams = child->params;

		if (childCommand == "with") {
			count = preloadImages(child, 0, count);
			continue;
		}

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
			continue;
		}

		if (childCommand == "if" || childCommand == "elif" || childCommand == "else") {
			preloadImages(child, 0, 1);
			continue;
		}



		if ((childCommand == "show" && !String::startsWith(childParams, "screen ")) || childCommand == "scene") {
			--count;

			std::vector<std::string> args = Algo::getArgs(childParams);
			while (args.size() >= 2 && Algo::in(std::string_view(args[args.size() - 2]), spriteParams)) {
				args.erase(args.end() - 2, args.end());
			}

			const std::string imageName = String::join(args, " ");
			if (!Sprite::imageWasRegistered(imageName)) continue;

			const std::vector<std::string> &declAt = Sprite::getChildrenImagesDeclAt(imageName);
			for (const std::string &path : declAt) {
				ImageManipulator::loadImage(path);
			}
		}
	}

	return count;
}

const Node* Node::getProp(const std::string &name) const {
	for (const Node *child : children) {
		if (child->command == name) {
			return child;
		}
	}
	return nullptr;
}

std::string Node::getPlace() const {
	return "Object <" + command + "> declared in:\n"
	        "  File <" + *fileName + ">\n" +
	        "  Line: " + std::to_string(numLine);
}

