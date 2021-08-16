#include "screen_node_utils.h"

#include <map>
#include <set>

#include <SDL2/SDL_ttf.h>

#include "gui/text_field.h"
#include "gui/screen/screen.h"
#include "gui/screen/key.h"
#include "gui/screen/text.h"
#include "gui/screen/textbutton.h"
#include "gui/screen/hotspot.h"
#include "gui/screen/imagemap.h"

#include "media/py_utils.h"
#include "media/py_utils/absolute.h"

#include <utils/algo.h>
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"



static void initConsts(Node *node);
static void initIsEnd(Node *node);
static void initNums(Node *node);
static std::string initCode(Node *node, const std::string& index = "");

static void initUpdateFuncs(Node *node);



static std::map<const Node*, std::string> screenCodes;
static std::map<const Node*, const std::vector<ScreenUpdateFunc>*> screenUpdateFuncs;


void initScreen(Node *node) {
	if (screenCodes.find(node) != screenCodes.end()) return;

	initConsts(node);
	initIsEnd(node);
	initNums(node);

	std::string screenCode = initCode(node);
	screenCodes[node] = screenCode;

	initUpdateFuncs(node);
}


std::string ScreenNodeUtils::getScreenCode(Node *screenNode) {
	auto it = screenCodes.find(screenNode);
	if (it != screenCodes.end()) return it->second;

	initScreen(screenNode);
	return screenCodes[screenNode];
}
const std::vector<ScreenUpdateFunc>* ScreenNodeUtils::getUpdateFuncs(const Node *node) {
	return screenUpdateFuncs[node];
}

void ScreenNodeUtils::clear() {
	screenCodes.clear();

	for (auto &p : screenUpdateFuncs) {
		delete p.second;
	}
	screenUpdateFuncs.clear();
}


static std::vector<std::string> screensInInit;

static void initConsts(Node *node) {
	const std::string &command = node->command;
	const std::string &params = node->params;

	node->countPropsToCalc = 0;

	if (node->isScreenProp) {
		if (node->isScreenEvent) {
			node->parent->withScreenEvent = true;
		}else {
			node->isScreenConst =
					command == "style" ||
					command == "has" ||
					PyUtils::isConstExpr(params);
		}
		return;
	}
	if (command == "$" || command == "python") return;

	ScopeExit se(nullptr);
	if (command == "screen") {
		screensInInit.push_back(params);
		se.func = [&]() {
			screensInInit.pop_back();
		};
	}

	if (command == "use") {
		Node *screenNode = Screen::getDeclared(node->params);

		if (screenNode) {
			if (Algo::in(params, screensInInit)) {
				Utils::outMsg("ScreenNodeUtils::initConsts",
							  "Using screens with recursion: " + String::join(screensInInit, " -> ") + " -> " + params);
				node->command = "pass";
			}else {
				initScreen(screenNode);
				node->isScreenConst = screenNode->isScreenConst;
				node->countPropsToCalc = screenNode->countPropsToCalc;
			}
		}else {
			Utils::outMsg("ScreenNodeUtils::initConsts",
						  "Screen with name <" + node->params + "> not found\n" +
						  node->getPlace());
			node->command = "pass";
		}
		return;
	}

	for (Node *child : node->children) {
		initConsts(child);

		if (child->isScreenProp && !child->isScreenConst && !child->isScreenEvent) {
			++node->countPropsToCalc;
		}
	}


	if (command == "button" || command == "textbutton") {
		node->withScreenEvent = true;
		return;
	}

	if (command == "if" || command == "elif" || command == "else" ||
	    command == "for" || command == "while") return;

	for (const Node *child : node->children) {
		if (!child->isScreenConst) return;
	}
	node->isScreenConst = true;
}

static void initIsEnd(Node *node) {
	if (node->isScreenProp) return;

	const std::string &command = node->command;

	if (command == "$" || command == "python") return;

	if (command == "use") {
		Node *screenNode = Screen::getDeclared(node->params);
		if (screenNode) {
			node->isScreenEnd = screenNode->isScreenEnd;
		}
		return;
	}

	for (Node *child : node->children) {
		initIsEnd(child);
	}

	if (command == "if" || command == "elif" || command == "else" ||
	    command == "for" || command == "while") return;

	for (const Node *child : node->children) {
		if (!child->isScreenProp && !child->isScreenConst) return;
	}
	node->isScreenEnd = true;
}

static void initNums(Node *node) {
	size_t i = 0;
	for (Node *child : node->children) {
		if (child->isScreenConst || child->isScreenEvent ||
			child->command == "$" || child->command == "python") continue;

		child->screenNum = i++;
		initNums(child);
	}
}



static size_t maxScreenChild(Node *node) {
	for (size_t i = node->children.size() - 1; i != size_t(-1); --i) {
		Node *child = node->children[i];
		if (child->screenNum != size_t(-1)) {
			return child->screenNum;
		}
	}

	return size_t(-1);
}


static void initChildCode(Node *child, std::string &res, const std::string &indent, const std::string &index) {
	const std::string &childCommand = child->command;

	std::string childRes = initCode(child, index);
	if (childRes.empty()) return;

	std::vector<std::string> code = String::split(childRes, "\n");

	if (child->isScreenProp || child->isScreenEnd) {
		if (childCommand != "continue" && childCommand != "break") {
			res += indent + "_SL_last[" + index + "] = ";
		}else {
			res += indent;
		}

		for (size_t i = 0; i < code.size(); ++i) {
			if (i) {
				res += indent;
			}
			res += code[i];
			if (i != code.size() - 1) {
				res += '\n';
			}
		}

		if (child->withScreenEvent) {
			res += "\n" +
				   indent + "_SL_check_events()";
		}
		res += '\n';

		return;
	}


	if (childCommand != "elif" && childCommand != "else") {
		if (childCommand != "$" && childCommand != "python") {
			res += indent + "_SL_stack.append(_SL_last)\n";
		}
	}

	for (const std::string &line : code) {
		res += indent + line + '\n';
	}

	bool hasEnd = true;
	if (childCommand == "if" || childCommand == "elif" ||
	    childCommand == "for" || childCommand == "while")
	{                                                                           // child is not-end-[condition/cycle]
		if (child->childNum + 1 < child->parent->children.size()) {             // and nextChild
			Node *nextChild = child->parent->children[child->childNum + 1];
			if (nextChild->command == "elif" || nextChild->command == "else") { // is may be end-[condition/cycle]
				hasEnd = false;
			}
		}
	}
	if (hasEnd) {
		if (childCommand != "$" && childCommand != "python") {
			res += indent + "_SL_last = _SL_stack.pop()\n";
		}
	}
}


static std::string initCycleCode(Node *node) {
	std::string res;
	std::string id = std::to_string(node->id);
	std::string name = "_SL_counter" + id;
	std::string idLen = "_SL_len" + id;
	std::string indent = "    ";

	const std::string &command = node->command;
	const std::string &params = node->params;

	size_t count = maxScreenChild(node) + 1;

	PyUtils::exec(node->getFileName(), node->getNumLine(), name + " = 0");

	if (node->childNum + 1 < node->parent->children.size()) {
		Node *next = node->parent->children[node->childNum + 1];
		if (next->command == "else") {
			res += "_SL_break" + id + " = False\n";
		}
	}

	res +=
	    "_SL_last[" + std::to_string(node->screenNum) + "] = _SL_last = [None] * " + name + "\n" +
	    idLen + " = " + name + "\n" +
	    name + " = 0\n"
	    "\n" +
	    command + ' ' + params + ": # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n" +
	    indent + "\n";

	if (count) {
		res +=
		    indent + "if " + name + " == " + idLen + ":\n" +
		    indent + "    _SL_last += [None] * " + std::to_string(50 * count) + "\n" +
		    indent + "    " + idLen + " += " + std::to_string(50 * count) + "\n" +
		    indent + "\n";
	}


	bool empty = true;
	for (Node *child : node->children) {
		if (child->isScreenConst && child->command != "continue" && child->command != "break") continue;
		if (child->isScreenEvent) continue;

		std::string index = name;
		if (child->screenNum) {
			index += " + " + std::to_string(child->screenNum);
		}
		initChildCode(child, res, indent, index);

		res += indent + '\n';
		empty = false;
	}
	if (empty) {
		res += indent + "pass\n";
	}

	if (count) {
		res += indent + name + " += " + std::to_string(count) + "\n";
	}

	return res;
}

static std::string initCode(Node *node, const std::string& index) {
	const std::string &command = node->command;
	const std::string &params = node->params;

	std::string res;

	if (command == "use") {
		Node *screenNode = Screen::getDeclared(params);
		if (!screenNode) return "";

		res = initCode(screenNode, index);
		return res;
	}

	if (command == "continue" || command == "break") {
		Node *t = node->parent;
		int extraDepth = 0;
		while (t->command != "for" && t->command != "while") {
			t = t->parent;
			++extraDepth;
		}
		std::string id = std::to_string(t->id);

		res = "# " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";

		if (command == "break" &&
		    t->childNum + 1 < t->parent->children.size() &&
		    t->parent->children[t->childNum + 1]->command == "else")
		{
			res += "_SL_break" + id + " = True\n";
		}
		std::string removeStackTop;
		if (extraDepth) {
			for (int i = 0; i < extraDepth - 1; ++i) {
				removeStackTop += "_SL_stack.pop()\n";
			}
			removeStackTop += "_SL_last = _SL_stack.pop()\n";
		}

		res += "_SL_counter" + id + " += " + std::to_string(maxScreenChild(t) + 1) + "\n" +
		       removeStackTop +
		       command + "\n";
		return res;
	}

	bool isMainScreen = command == "screen" && index.empty();
	if ((node->isScreenConst && !isMainScreen) || node->isScreenEvent) return "";

	if (node->isScreenProp) {
		res = params;
		res = res.substr(res.find_first_not_of(' '));
		res = res.substr(res.find_first_not_of(' '));
		res += "# " + std::to_string(node->getNumLine()) + " " + node->getFileName();
		return res;
	}

	if (command == "$" || command == "python") {
		size_t startLine = node->getNumLine() + (command == "python");

		const std::vector<std::string> code = String::split(params, "\n");
		for (size_t i = 0; i < code.size(); ++i) {
			if (code[i].empty()) continue;

			res += code[i] + " # " + std::to_string(startLine + i) + " " + node->getFileName();
			if (i != code.size() - 1) {
				res += "\n";
			}
		}

		return res;
	}

	if (command == "for" || command == "while") return initCycleCode(node);



	if (node->isScreenEnd && !isMainScreen) {
		res = "(\n";
		for (Node *child : node->children) {
			if (child->isScreenConst || child->isScreenEvent) continue;

			std::string childRes = initCode(child);
			childRes.insert(String::firstNotInQuotes(childRes, '#'), ", ");

			res += "    " + childRes + "\n";
		}

		if (res.size() != 2) {
			res += ")";
		}else {
			res[1] = ')';
		}

		return res;
	}


	std::string indent;

	if (command == "screen") {
		if (index.empty()) {
			res = "_SL_stack = []\n\n";
		}
	}else
	if (command == "if" || command == "elif" || command == "else") {
		indent = "    ";

		if (command == "else" && !String::endsWith(node->parent->children[node->childNum - 1]->command, "if")) {//prev is for/while
			res +=
			    "if not _SL_break" + std::to_string(node->parent->children[node->childNum - 1]->id) + ":"
			        " # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";
			res += indent + "_SL_last = _SL_stack[-1]\n";
		}else {
			res = command + " " + params + ": # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";
		}
	}

	res += indent;
	if (command == "screen" && index.empty()) {
		res += "_SL_" + params + " = ";
	}else {
		res += "_SL_last[" + index + "] = ";
	}
	res += "_SL_last = [\n";
	size_t bracketBegin = res.size() - 2;

	size_t lastPropNum = size_t(-1);
	size_t maxChildNum = maxScreenChild(node);

	//props before first obj
	for (size_t i = 0; i < node->children.size(); ++i) {
		Node *child = node->children[i];
		if (child->isScreenConst) continue;
		if (!child->isScreenProp) break;

		std::string childRes = initCode(child);
		childRes.insert(String::firstNotInQuotes(childRes, '#'), ", ");

		res += indent + "    " + childRes + "\n";
		lastPropNum = child->screenNum;
	}

	if (maxChildNum != size_t(-1)) {
		res += String::repeat(indent + "    None,\n", maxChildNum - lastPropNum);
		res += indent + "]";
	}else {
		res.back() = ']';
	}

	bool empty = false;
	if (lastPropNum == maxChildNum) {
		res[bracketBegin] = '(';
		res.back() = ')';

		empty = true;
		for (Node *child : node->children) {
			if (child->command == "$" || child->command == "python" ||
				child->command == "continue" || child->command == "break")
			{
				empty = false;
				break;
			}
		}
	}

	if (!empty) {
		res += "\n" +
			   indent + "\n";

		for (Node *child : node->children) {
			if (child->isScreenConst && child->command != "continue" && child->command != "break") continue;
			if (child->isScreenEvent) continue;

			if (child->screenNum <= lastPropNum && lastPropNum != size_t(-1)) continue;

			initChildCode(child, res, indent, std::to_string(child->screenNum));

			if (child->screenNum != maxChildNum) {
				res += indent + '\n';
			}
		}
	}


	if (isMainScreen) {
		res += "\n"
		       "_SL_check_events()\n";
	}

	return res;
}






static void outError(Child *obj, const std::string &propName, size_t propIndex, const std::string &expected) {
	if (!obj->wasInited()) {
		for (Node *child : obj->node->children) {
			if (child->isScreenProp && child->command == propName) {
				std::string desc = expected + '\n' +
				                   child->getPlace();
				Utils::outMsg(propName, desc);
				return;
			}
		}
	}

	for (Node *child : obj->node->children) {
		if (child->isScreenProp && child->screenNum == propIndex) {
			std::string desc = expected + '\n';

			if (child->command == propName) {
				desc += child->getPlace();
			}else {
				std::string style = obj->node->command;
				for (Node *child : obj->node->children) {
					if (child->command == "style") {
						style = child->params;
						break;
					}
				}
				desc += "style <" + style + ">";
			}

			Utils::outMsg(propName, desc);
			return;
		}
	}
}

#define updateCondition(isFloat, var, prop) \
	isFloat = PyFloat_CheckExact(prop); \
	typedef decltype(var) DT; \
	if (isFloat || PyAbsolute_CheckExact(prop)) { \
	    var = DT(PyFloat_AS_DOUBLE(prop)); \
	}else \
	if (PyInt_CheckExact(prop)) { \
	    var = DT(PyInt_AS_LONG(prop)); \
	}else \
	if (PyLong_CheckExact(prop)) { \
	    var = DT(PyLong_AsDouble(prop)); \
	}


#define makeUpdateFuncType(Type, propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	[[maybe_unused]] bool tmpBool; \
	updateCondition(tmpBool, static_cast<Type*>(obj)->propName, prop) \
	else { \
	    static_cast<Type*>(obj)->propName = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, absolute, int or long, got " + type); \
	} \
}

#define makeUpdateFunc(propName) makeUpdateFuncType(Child, propName)

#define makeUpdateFuncWithBool(Type, propName, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	if (PyBool_Check(prop)) { \
		static_cast<Type*>(obj)->propName = prop == Py_True; \
	}else { \
	    static_cast<Type*>(obj)->propName = false; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type bool, got " + type); \
	} \
}

#define makeUpdateFuncWithIsFloat(propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	updateCondition(obj->propName##IsFloat, obj->propName, prop) \
	else { \
	    obj->propName = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, absolute, int or long, got " + type); \
	} \
}

#define makeUpdateCommonFuncWithOptionalIsFloat(propName, xIsFloat, yIsFloat) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	bool tmpBool; \
	bool isNumber = true; \
	[[maybe_unused]] bool unusedBoolX, unusedBoolY; \
	float value = 0; \
	updateCondition(tmpBool, value, prop) \
	else { \
		isNumber = false; \
	} \
	\
	if (isNumber) { \
	    xIsFloat = yIsFloat = tmpBool; \
		obj->x##propName = obj->y##propName = value; \
		return; \
	} \
	\
	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) { \
	    xIsFloat = yIsFloat = false; \
	    obj->x##propName = obj->y##propName = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, absolute, int, long, tuple or list, got " + type); \
		return; \
	} \
	if (Py_SIZE(prop) != 2) { \
	    xIsFloat = yIsFloat = false; \
	    obj->x##propName = obj->y##propName = 0; \
		std::string size = std::to_string(Py_SIZE(prop)); \
		outError(obj, #propName, propIndex, "Expected sequence with size == 2, got " + size); \
		return; \
	} \
	\
	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0); \
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1); \
	\
	updateCondition(xIsFloat, obj->x##propName, x) \
	else { \
	    xIsFloat = false; \
	    obj->x##propName = 0; \
		std::string type = x->ob_type->tp_name; \
		outError(obj, "x"#propName, propIndex, "At x" #propName "-prop expected types float, absolute, int or long, got " + type); \
	} \
	updateCondition(yIsFloat, obj->y##propName, y) \
	else { \
	    yIsFloat = false; \
	    obj->y##propName = 0; \
		std::string type = y->ob_type->tp_name; \
		outError(obj, "y"#propName, propIndex, "At y" #propName "-prop expected types float, absolute, int or long, got " + type); \
	} \
}

#define makeUpdateCommonFunc(propName) makeUpdateCommonFuncWithOptionalIsFloat(propName, obj->x##propName##IsFloat, obj->y##propName##IsFloat)
#define makeUpdateCommonFuncWithoutIsFloat(propName) makeUpdateCommonFuncWithOptionalIsFloat(propName, unusedBoolX, unusedBoolY)


#define makeUpdateFuncWithAlign(x_or_y) \
static void update_##x_or_y##align(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	bool isFloat; \
	float value = 0; \
	updateCondition(isFloat, value, prop) \
	else { \
	    isFloat = false; \
	    value = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #x_or_y"align", propIndex, "Expected types float, absolute, int or long, got " + type); \
	} \
	obj->x_or_y##posIsFloat = obj->x_or_y##anchorPreIsFloat = isFloat; \
	obj->x_or_y##pos = obj->x_or_y##anchorPre = value; \
}

static void update_align(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	bool isFloat;
	bool isNumber = true;
	float value = 0;
	updateCondition(isFloat, value, prop)
	else {
		isNumber = false;
	}

	if (isNumber) {
		obj->xposIsFloat = obj->xanchorPreIsFloat = obj->yposIsFloat = obj->yanchorPreIsFloat = isFloat;
		obj->xpos = obj->xanchorPre = obj->ypos = obj->yanchorPre = value;
		return;
	}

	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		obj->xposIsFloat = obj->xanchorPreIsFloat = obj->yposIsFloat = obj->yanchorPreIsFloat = false;
		obj->xpos = obj->xanchorPre = obj->ypos = obj->yanchorPre = 0;
		std::string type = prop->ob_type->tp_name;
		outError(obj, "align", propIndex, "Expected types float, absolute, int, long, tuple or list, got " + type);
		return;
	}
	if (Py_SIZE(prop) != 2) {
		obj->xposIsFloat = obj->xanchorPreIsFloat = obj->yposIsFloat = obj->yanchorPreIsFloat = false;
		obj->xpos = obj->xanchorPre = obj->ypos = obj->yanchorPre = 0;
		std::string size = std::to_string(Py_SIZE(prop));
		outError(obj, "align", propIndex, "Expected sequence with size == 2, got " + size);
		return;
	}

	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0);
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1);

	updateCondition(isFloat, value, x)
	else {
		isFloat = false;
		value = 0;
		std::string type = x->ob_type->tp_name;
		outError(obj, "align", propIndex, "At xalign-prop expected types float, absolute, int or long, got " + type);
	}
	obj->xposIsFloat = obj->xanchorPreIsFloat = isFloat;
	obj->xpos = obj->xanchorPre = value;

	updateCondition(isFloat, value, y)
	else {
		isFloat = false;
		value = 0;
		std::string type = y->ob_type->tp_name;
		outError(obj, "align", propIndex, "At yalign-prop expected types float, absolute, int or long, got " + type);
	}
	obj->yposIsFloat = obj->yanchorPreIsFloat = isFloat;
	obj->ypos = obj->yanchorPre = value;
}

#define makeUpdateFuncWithStr(Type, propName, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	if (PyString_CheckExact(prop)) { \
		static_cast<Type*>(obj)->propName = PyString_AS_STRING(prop); \
	}else { \
	    static_cast<Type*>(obj)->propName = ""; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type str, got " + type); \
	} \
}
#define makeUpdateFuncWithStrSimple(Type, propName) makeUpdateFuncWithStr(Type, propName, propName)

static void update_crop(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		obj->xcropIsFloat = obj->ycropIsFloat = obj->wcropIsFloat = obj->hcropIsFloat = false;
		obj->xcrop = obj->ycrop = obj->wcrop = obj->hcrop = 0;
		std::string type = prop->ob_type->tp_name;
		outError(obj, "crop", propIndex, "Expected types tuple or list, got " + type);
		return;
	}
	if (Py_SIZE(prop) != 4) {
		obj->xcropIsFloat = obj->ycropIsFloat = obj->wcropIsFloat = obj->hcropIsFloat = false;
		obj->xcrop = obj->ycrop = obj->wcrop = obj->hcrop = 0;
		std::string size = std::to_string(Py_SIZE(prop));
		outError(obj, "crop", propIndex, "Expected sequence with size == 4, got " + size);
		return;
	}

	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0);
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1);
	PyObject *w = PySequence_Fast_GET_ITEM(prop, 2);
	PyObject *h = PySequence_Fast_GET_ITEM(prop, 3);

	updateCondition(obj->xcropIsFloat, obj->xcrop, x)
	else {
		obj->xcropIsFloat = false;
		obj->xcrop = 0;
		std::string type = x->ob_type->tp_name;
		outError(obj, "xcrop", propIndex, "At xcrop-prop expected types float, absolute, int or long, got " + type);
	}

	updateCondition(obj->ycropIsFloat, obj->ycrop, y)
	else {
		obj->ycropIsFloat = false;
		obj->ycrop = 0;
		std::string type = y->ob_type->tp_name;
		outError(obj, "ycrop", propIndex, "At ycrop-prop expected types float, absolute, int or long, got " + type);
	}

	updateCondition(obj->wcropIsFloat, obj->wcrop, w)
	else {
		obj->wcropIsFloat = false;
		obj->wcrop = 0;
		std::string type = w->ob_type->tp_name;
		outError(obj, "wcrop", propIndex, "At wcrop-prop expected types float, absolute, int or long, got " + type);
	}

	updateCondition(obj->hcropIsFloat, obj->hcrop, h)
	else {
		obj->hcropIsFloat = false;
		obj->hcrop = 0;
		std::string type = h->ob_type->tp_name;
		outError(obj, "hcrop", propIndex, "At hcrop-prop expected types float, absolute, int or long, got " + type);
	}
}

#define makeUpdateTextFunc(propName, mask) \
static void update_text_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &fontStyle = text->tf->mainStyle.fontStyle; \
	auto oldFontStyle = fontStyle; \
	\
	if (prop == Py_True) { \
		fontStyle |= mask; \
		if (fontStyle != oldFontStyle) { \
			text->needUpdate = true; \
		} \
	}else if (prop == Py_False) { \
		fontStyle &= ~mask; \
		if (fontStyle != oldFontStyle) { \
			text->needUpdate = true; \
		} \
	}else { \
	    fontStyle &= ~mask; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected bool, got " + type); \
	} \
}
#define makeUpdateTextColor(propName, isOutline) \
static void update_text_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &style = text->tf->mainStyle; \
	\
	Uint32 v = Uint32(-1); \
	if (PyInt_CheckExact(prop)) { \
		long l = PyInt_AS_LONG(prop); \
		if (l < 0 || l > 0xFFFFFF) { \
			outError(obj, #propName, propIndex, "Expected value between 0 and 0xFFFFFF, got <" + std::to_string(l) + ">"); \
		}else { \
			v = Uint32(l); \
		} \
	}else \
	if (PyLong_CheckExact(prop)) { \
	    double d = PyLong_AsDouble(prop); \
		if (d < 0 || d > 0xFFFFFF) { \
			outError(obj, #propName, propIndex, "Expected value between 0 and 0xFFFFFF, got <" + std::to_string(d) + ">"); \
		}else { \
			v = Uint32(d); \
		} \
	}else \
	if (prop == Py_None || prop == Py_False) { \
		if (style.enableOutline) { \
			text->needUpdate = true; \
			style.enableOutline = false; \
		} \
	} \
	else { \
		std::string type = prop == Py_True ? "True" : prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types int, long, None or False, got " + type); \
	} \
	\
	if (v != Uint32(-1)) { \
		if constexpr (isOutline) { \
			if (style.outlineColor != v || !style.enableOutline) { \
				text->needUpdate = true; \
				style.outlineColor = v; \
				style.enableOutline = true; \
			} \
		}else { \
			if (style.color != v) { \
				text->needUpdate = true; \
				style.color = v; \
			} \
		} \
	}else { \
	    if constexpr (isOutline) { \
	        if (style.enableOutline) { \
	            style.enableOutline = false; \
	            text->needUpdate = true; \
	        } \
	    }else { \
	        if (style.color) { \
	            style.color = 0; \
	            text->needUpdate = true; \
	        } \
	    } \
	}\
}

makeUpdateFunc(alpha)
makeUpdateFunc(rotate)
makeUpdateFuncWithBool(Child, clipping, clipping)

makeUpdateFuncWithIsFloat(xpos)
makeUpdateFuncWithIsFloat(ypos)

makeUpdateFuncWithIsFloat(xanchorPre)
makeUpdateFuncWithIsFloat(yanchorPre)

makeUpdateFuncWithIsFloat(xsize)
makeUpdateFuncWithIsFloat(ysize)

makeUpdateFuncType(Child, xzoom)
makeUpdateFuncType(Child, yzoom)

makeUpdateCommonFunc(pos)
makeUpdateCommonFunc(anchorPre)
makeUpdateCommonFunc(size)
makeUpdateCommonFuncWithoutIsFloat(zoom)

makeUpdateFuncWithAlign(x)
makeUpdateFuncWithAlign(y)

makeUpdateFuncWithStrSimple(Child, first_param)

makeUpdateFuncType(Key, keyDelay)
makeUpdateFuncType(Key, firstKeyDelay)

makeUpdateFuncType(Screen, zorder)
makeUpdateFuncWithBool(Screen, modal, modal)

makeUpdateFuncType(Container, spacing)

makeUpdateFuncType(Text, text_size)
makeUpdateTextColor(color, false)
makeUpdateTextColor(outlinecolor, true)
makeUpdateTextFunc(bold, TTF_STYLE_BOLD)
makeUpdateTextFunc(italic, TTF_STYLE_ITALIC)
makeUpdateTextFunc(underline, TTF_STYLE_UNDERLINE)
makeUpdateTextFunc(strikethrough, TTF_STYLE_STRIKETHROUGH)

makeUpdateFuncWithStrSimple(Text, font)
makeUpdateFuncWithStr(Text, textHAlign, text_align)
makeUpdateFuncWithStr(Text, textVAlign, text_valign)

makeUpdateFuncWithStr(TextButton, ground, ground_textbutton)
makeUpdateFuncWithStr(TextButton, hover, hover_textbutton)
makeUpdateFuncWithBool(TextButton, btnRect.buttonMode, mouse_textbutton)

makeUpdateFuncWithBool(Hotspot, btnRect.buttonMode, mouse_hotspot)

makeUpdateFuncWithStr(Imagemap, groundPath, ground_imagemap)
makeUpdateFuncWithStr(Imagemap, hoverPath, hover_imagemap)


static std::map<std::string, ScreenUpdateFunc> mapScreenFuncs = {
	{"alpha",             update_alpha},
	{"rotate",            update_rotate},
	{"clipping",          update_clipping},
	{"delay",             update_keyDelay},
	{"first_delay",       update_firstKeyDelay},
	{"xpos",              update_xpos},
	{"ypos",              update_ypos},
	{"xanchor",           update_xanchorPre},
	{"yanchor",           update_yanchorPre},
	{"xsize",             update_xsize},
	{"ysize",             update_ysize},
	{"xzoom",             update_xzoom},
	{"yzoom",             update_yzoom},
	{"pos",               update_pos},
	{"anchor",            update_anchorPre},
	{"size",              update_size},
	{"zoom",              update_zoom},
	{"xalign",            update_xalign},
	{"yalign",            update_yalign},
	{"align",             update_align},
	{"first_param",       update_first_param},
	{"crop",              update_crop},
	{"zorder",            update_zorder},
	{"modal",             update_modal},
	{"spacing",           update_spacing},
	{"color",             update_text_color},
	{"outlinecolor",      update_text_outlinecolor},
	{"font",              update_font},
	{"text_align",        update_text_align},
	{"text_valign",       update_text_valign},
	{"text_size",         update_text_size},
	{"bold",              update_text_bold},
	{"italic",            update_text_italic},
	{"underline",         update_text_underline},
	{"strikethrough",     update_text_strikethrough},
	{"ground_textbutton", update_ground_textbutton},
	{"hover_textbutton",  update_hover_textbutton},
	{"mouse_textbutton",  update_mouse_textbutton},
	{"mouse_hotspot",     update_mouse_hotspot},
	{"ground_imagemap",   update_ground_imagemap},
	{"hover_imagemap",    update_hover_imagemap},
};

static void initUpdateFuncs(Node *node) {
	if (screenUpdateFuncs.find(node) != screenUpdateFuncs.end()) return;

	size_t maxScreenNum = size_t(-1);
	for (Node *child : node->children) {
		if (child->isScreenProp) {
			if (!child->isScreenConst && !child->isScreenEvent &&
				(child->screenNum > maxScreenNum || maxScreenNum == size_t(-1)))
			{
				maxScreenNum = child->screenNum;
			}
		}else {
			initUpdateFuncs(child);
		}
	}

	std::vector<ScreenUpdateFunc> *vec = nullptr;

	if (maxScreenNum != size_t(-1)) {
		vec = new std::vector<ScreenUpdateFunc>(maxScreenNum + 1, nullptr);

		for (Node *child : node->children) {
			if (!child->isScreenProp || child->isScreenConst || child->isScreenEvent) continue;

			ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, child->command);
			(*vec)[child->screenNum] = func;
		}
	}

	screenUpdateFuncs[node] = vec;
}

ScreenUpdateFunc ScreenNodeUtils::getUpdateFunc(const std::string &type, std::string propName) {
	if (propName == "ground" || propName == "hover" || propName == "mouse") {
		propName += "_" + (type == "button" ? "textbutton" : type);
	}

	auto it = mapScreenFuncs.find(propName);
	if (it != mapScreenFuncs.end()) {
		return it->second;
	}

	Utils::outMsg("ScreenNodeUtils::getUpdateFunc", "Func <" + propName + "> not found");
	return nullptr;
}


