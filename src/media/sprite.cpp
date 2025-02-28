#include "sprite.h"

#include <map>

#include "media/py_utils.h"
#include "parser/node.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


static std::map<std::string, Node*> declAts;
static std::map<PyObject*, Node*> actionNodes;
static std::map<std::string, std::vector<std::string>> declAtVecs;


static Node* parseAction(PyObject *action, uint32_t childNum,
                         const std::string &parentFileName, uint32_t parentNumLine)
{
	auto it = actionNodes.find(action);
	if (it != actionNodes.end()) return it->second;

	Node *res = actionNodes[action] = nullptr;

	ScopeExit se([&] {
		if (!res) {
			std::string num = std::to_string(childNum);
			std::string place = '(' + parentFileName + ':' + std::to_string(parentNumLine) + ')';
			Utils::outMsg("Sprite, parseAction", "Failed parse child <" + num + "> of " + place);
		}
	});

	if (!PyTuple_CheckExact(action) && !PyList_CheckExact(action)) return res;
	if (Py_SIZE(action) != 3) return res;

	PyObject *pyCommand = PySequence_Fast_GET_ITEM(action, 0);
	if (!PyUnicode_CheckExact(pyCommand) && !PyTuple_CheckExact(pyCommand) && !PyList_CheckExact(pyCommand)) return res;

	PyObject *pyFileName = PySequence_Fast_GET_ITEM(action, 1);
	if (!PyUnicode_CheckExact(pyFileName)) return res;

	PyObject *pyNumLine = PySequence_Fast_GET_ITEM(action, 2);
	if (!PyLong_CheckExact(pyNumLine)) return res;

	int overflow;
	long numLine = PyLong_AsLongAndOverflow(pyNumLine, &overflow);
	if (overflow || numLine < 0) return res;

	Py_ssize_t fileNameSize;
	const char *fileNamePtr = PyUnicode_AsUTF8AndSize(pyFileName, &fileNameSize);
	std::string fileName = std::string(fileNamePtr, size_t(fileNameSize));

	res = actionNodes[action] = Node::getNewNode(fileName, uint32_t(numLine));

	if (PyUnicode_CheckExact(pyCommand)) {
		Py_ssize_t commandSize;
		const char *commandPtr = PyUnicode_AsUTF8AndSize(pyCommand, &commandSize);
		res->command = std::string(commandPtr, size_t(commandSize));
		return res;
	}

	uint32_t blockSize = uint32_t(Py_SIZE(pyCommand));
	for (uint32_t i = 0; i < blockSize; ++i) {
		PyObject *action = PySequence_Fast_GET_ITEM(pyCommand, i);
		Node *child = parseAction(action, i, fileName, uint32_t(numLine));
		if (!child) continue;

		if (res->command.empty() && res->getNumLine() == child->getNumLine()) {
			res->command = child->command;
		}else {
			child->parent = res;
			child->childNum = uint32_t(res->children.size());
			res->children.push_back(child);
		}
	}

	return res;
}


void Sprite::registerImage(Node *imageNode) {
	const std::string &desc = imageNode->params;
	std::string name;
	std::string path;

	size_t i = desc.find('=');
	if (i == size_t(-1)) {
		name = desc;
	}else {
		name = desc.substr(0, i - 1);
		path = String::strip(desc.substr(i + 1));
	}

	name = String::strip(name);
	if (name.empty()) {
		Utils::outMsg("Utils::registerImage",
		              "Empty name\n" +
		              imageNode->getPlace());
		return;
	}
	declAts[name] = imageNode;

	bool haveDeclAt = !imageNode->children.empty();
	if (!path.empty()) {
		Node *first = Node::getNewNode(imageNode->getFileName(), imageNode->getNumLine());
		first->params = path;
		first->parent = imageNode;
		first->childNum = uint32_t(imageNode->children.size());
		imageNode->children.insert(imageNode->children.begin(), first);
	}
	if (haveDeclAt) return;



	PyUtils::callInPythonThread([&]() {
		PyObject* actions = PyUtils::execRetObj(imageNode->getFileName(), imageNode->getNumLine(),
		                                        "get_default_transform_actions()");
		if (!actions) return;

		if (!PyTuple_CheckExact(actions) && !PyList_CheckExact(actions)) {
			std::string type = actions->ob_type->tp_name;
			Utils::outMsg("Sprite::registerImage",
			              "type(get_default_transform_actions()) expected tuple or list, got <" + type + ">");
			return;
		}

		uint32_t sizeDefaultDeclAt = uint32_t(Py_SIZE(actions));
		for (uint32_t i = 0; i < sizeDefaultDeclAt; ++i) {
			PyObject *elem = PySequence_Fast_GET_ITEM(actions, i);

			Node *node = parseAction(elem, i, imageNode->getFileName(), imageNode->getNumLine());
			if (node) {
				imageNode->children.push_back(node);
			}
		}
	});
}

bool Sprite::imageWasRegistered(const std::string &name) {
	return declAts.find(name) != declAts.end();
}
void Sprite::clearImages() {
	declAts.clear();
	actionNodes.clear();
	declAtVecs.clear();
}

PyObject* Sprite::getImageDeclAt(const std::string &name) {
	auto it = declAts.find(name);
	if (it != declAts.end()) {
		const Node *node = it->second;
		return node->getPyChildren();
	}

	Utils::outMsg("Sprite::getImageDeclAt", "Image <" + name + "> not registered");
	return PyTuple_New(0);
}

static void getChildrenImages(std::vector<std::string> &res, const Node* node, bool isRoot = true) {
	auto checkAndAdd = [&](std::string path) {
		if (!String::isSimpleString(path)) return;
		path.pop_back();
		path.erase(0, 1);

		if (!FileSystem::exists(path)) return;
		if (FileSystem::isDirectory(path)) return;

		if (Sprite::imageWasRegistered(path)) {
			getChildrenImages(res, declAts[path]);
		}else {
			res.push_back(path);
		}
	};

	if (!isRoot) {
		checkAndAdd(node->command);
		checkAndAdd(node->params);
	}

	for (const Node* child : node->children) {
		bool childIsBlock = Algo::in(std::string_view(child->command), Node::blockCommandsInImage);

		if (childIsBlock) {
			getChildrenImages(res, child, false);
		}else {
			checkAndAdd(Algo::clear(child->command + " " + child->params));
		}
	}
}

const std::vector<std::string>& Sprite::getChildrenImagesDeclAt(const std::string &name) {
	auto cacheIt = declAtVecs.find(name);
	if (cacheIt != declAtVecs.end()) return cacheIt->second;

	std::vector<std::string> &res = declAtVecs[name] = {};

	auto it = declAts.find(name);
	if (it != declAts.end()) {
		const Node *node = it->second;
		getChildrenImages(res, node);
	}else {
		Utils::outMsg("Sprite::getVectorImageDeclAt", "Image <" + name + "> not registered");
	}

	return res;
}
