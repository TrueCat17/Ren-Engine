#include "screen_node_utils.h"

#include <map>

#include <SDL2/SDL_ttf.h>

#include "gui/screen/screen.h"
#include "gui/screen/key.h"
#include "gui/screen/text.h"
#include "gui/screen/textbutton.h"
#include "gui/screen/hotspot.h"
#include "gui/screen/imagemap.h"

#include "media/py_utils.h"
#include "media/py_utils/absolute.h"

#include "utils/algo.h"
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
			node->isScreenConst = command == "has" || PyUtils::isConstExpr(params);
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
	if (command == "use") return;

	for (Node *child : node->children) {
		initIsEnd(child);
	}
	if (command == "screen") return;

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

static std::string initUseCode(Node *node, const std::string &index) {
	Node *screenNode = Screen::getDeclared(node->params);
	if (!screenNode) return "";
	if (screenNode->vars.size() < node->vars.size()) {
		Utils::outMsg("ScreenNodeUtils::initUseCode",
		              "Maximum " + std::to_string(screenNode->vars.size()) + " args expected"
		              "for screen <" + node->params + ">, got " + std::to_string(node->vars.size()) + "\n\n" +
		              node->getPlace());
	}

	std::string inner = initCode(screenNode, index);
	std::string res =
	        "_SL_screen_vars_stack.append(screen)\n"
	        "screen = Object()\n";


	std::vector<std::string> argsWasSet;
	argsWasSet.reserve(node->vars.size());

	for (size_t i = 0; i < node->vars.size(); ++i) {
		auto &[varName, varCode] = node->vars[i];

		const std::string *namePtr = &varName;
		if (namePtr->empty() && i < screenNode->vars.size()) {
			namePtr = &screenNode->vars[i].first;
		}
		const std::string &name = *namePtr;

		if (!name.empty()) {
			if (std::find_if(
			        screenNode->vars.cbegin(),
			        screenNode->vars.cend(),
			        [&name] (const std::pair<std::string, std::string> &p) -> bool { return p.first == name; }
			) == screenNode->vars.cend())
			{
				Utils::outMsg("ScreenNodeUtils::initUseCode",
				              "No param <" + name + "> in screen <" + node->params + ">\n\n" +
				              node->getPlace());
			}else {
				argsWasSet.push_back(name);
				res += "screen." + name + " = " + varCode + "\n";
			}
		}
	}

	for (size_t i = 0; i < screenNode->vars.size(); ++i) {
		auto &[varName, varCode] = screenNode->vars[i];
		if (std::find(argsWasSet.cbegin(), argsWasSet.cend(), varName) != argsWasSet.cend()) continue;

		if (varCode.empty()) {
			Utils::outMsg("ScreenNodeUtils::initUseCode",
			              "No value for param <" + varName + "> in screen <" + node->params + ">\n\n" +
			              node->getPlace());
		}else {
			res += "screen." + varName + " = " + varCode + "\n";
		}
	}

	res += "\n" +
	       inner + "\n\n" +
	       "screen = _SL_screen_vars_stack.pop()";
	return res;
}

static std::string initCode(Node *node, const std::string& index) {
	const std::string &command = node->command;
	const std::string &params = node->params;

	std::string res;

	if (command == "use") {
		res = initUseCode(node, index);
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



	if (node->isScreenEnd) {
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
			res = "_SL_stack = []\n"
			      "_SL_screen_vars_stack = []\n"
			      "screen = screen_vars['" + params + "']\n\n";
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
	for (Node *child : node->children) {
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
		for (const Node *child : node->children) {
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
	Node *node = obj->node;
	if (node->command == "use") {
		node = Screen::getDeclared(node->params);
		if (!node) {
			Utils::outMsg("ScreenNodeUtils::outError", "Error on out error: screen <" + obj->node->params + "> not found (?)");
			Utils::outMsg(propName, expected);
			return;
		}
	}

	if (!obj->wasInited()) {
		for (const Node *child : node->children) {
			if (!child->isScreenProp) continue;

			if (child->command == propName || child->command + "Pre" == propName) {
				std::string desc = expected + '\n' +
				                   child->getPlace();
				Utils::outMsg(propName, desc);
				return;
			}
		}
	}

	for (const Node *child : node->children) {
		if (child->isScreenProp && child->screenNum == propIndex) {
			std::string desc = expected + '\n';

			if (child->command == propName || child->command + "Pre" == propName) {
				desc += child->getPlace();
			}else {
				std::string style = node->command;
				for (const Node *child : node->children) {
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

#define updateCondition(isFloat, var, prop, propName) \
	isFloat = PyFloat_CheckExact(prop); \
	typedef decltype(var) DT; \
	if (isFloat || PyAbsolute_CheckExact(prop)) { \
		var = DT(PyFloat_AS_DOUBLE(prop)); \
	}else \
	if (PyLong_CheckExact(prop)) { \
		int overflow; \
		var = DT(PyLong_AsLongAndOverflow(prop, &overflow)); \
		if (overflow) { \
			var = 0; \
			outError(obj, propName, propIndex, "int too large (" + PyUtils::objToStr(prop) + ")"); \
		} \
	}


#define makeUpdateFuncType(Type, propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	bool tmpBool; \
	updateCondition(tmpBool, static_cast<Type*>(obj)->propName, prop, #propName) \
	else { \
		static_cast<Type*>(obj)->propName = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, absolute or int, got " + type); \
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
	updateCondition(obj->propName##IsFloat, obj->propName, prop, #propName) \
	else { \
		obj->propName = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, absolute or int, got " + type); \
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
	updateCondition(tmpBool, value, prop, #propName) \
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
	std::string error; \
	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) { \
		std::string type = prop->ob_type->tp_name; \
		error = "Expected types float, absolute, int, tuple or list, got " + type; \
	}else \
	if (Py_SIZE(prop) != 2) { \
		std::string size = std::to_string(Py_SIZE(prop)); \
		error = "Expected sequence with size == 2, got " + size; \
	} \
	if (!error.empty()) { \
		xIsFloat = yIsFloat = false; \
		obj->x##propName = obj->y##propName = 0; \
		outError(obj, #propName, propIndex, error); \
		return; \
	} \
	\
	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0); \
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1); \
	\
	updateCondition(xIsFloat, obj->x##propName, x, #propName) \
	else { \
		xIsFloat = false; \
		obj->x##propName = 0; \
		std::string type = x->ob_type->tp_name; \
		outError(obj, "x"#propName, propIndex, "At x" #propName "-prop expected types float, absolute or int, got " + type); \
	} \
	updateCondition(yIsFloat, obj->y##propName, y, #propName) \
	else { \
		yIsFloat = false; \
		obj->y##propName = 0; \
		std::string type = y->ob_type->tp_name; \
		outError(obj, "y"#propName, propIndex, "At y" #propName "-prop expected types float, absolute or int, got " + type); \
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
	updateCondition(isFloat, value, prop, #x_or_y"align") \
	else { \
		isFloat = false; \
		value = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #x_or_y"align", propIndex, "Expected types float, absolute or int, got " + type); \
	} \
	obj->x_or_y##posIsFloat = obj->x_or_y##anchorPreIsFloat = isFloat; \
	obj->x_or_y##pos = obj->x_or_y##anchorPre = value; \
}

static void update_align(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	bool isFloat;
	bool isNumber = true;
	float value = 0;
	updateCondition(isFloat, value, prop, "align")
	else {
		isNumber = false;
	}

	if (isNumber) {
		obj->xposIsFloat = obj->xanchorPreIsFloat = obj->yposIsFloat = obj->yanchorPreIsFloat = isFloat;
		obj->xpos = obj->xanchorPre = obj->ypos = obj->yanchorPre = value;
		return;
	}

	std::string error;
	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		std::string type = prop->ob_type->tp_name;
		error = "Expected types float, absolute, int, tuple or list, got " + type;
	}else
	if (Py_SIZE(prop) != 2) {
		std::string size = std::to_string(Py_SIZE(prop));
		error = "Expected sequence with size == 2, got " + size;
	}
	if (!error.empty()) {
		obj->xposIsFloat = obj->xanchorPreIsFloat = obj->yposIsFloat = obj->yanchorPreIsFloat = false;
		obj->xpos = obj->xanchorPre = obj->ypos = obj->yanchorPre = 0;
		outError(obj, "align", propIndex, error);
		return;
	}

#define partProc(part, index) \
	PyObject *part = PySequence_Fast_GET_ITEM(prop, index); \
	updateCondition(isFloat, value, part, #part "align") \
	else { \
		isFloat = false; \
		value = 0; \
		std::string type = part->ob_type->tp_name; \
		outError(obj, "align", propIndex, "At " #part "align-prop expected types float, absolute or int, got " + type); \
	} \
	obj->part##posIsFloat = obj->part##anchorPreIsFloat = isFloat; \
	obj->part##pos = obj->part##anchorPre = value;

	partProc(x, 0);
	partProc(y, 1);
#undef partProc
}

#define makeUpdateFuncWithStr(Type, propName, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	if (PyUnicode_CheckExact(prop)) { \
		static_cast<Type*>(obj)->propName = PyUnicode_AsUTF8(prop); \
	}else { \
		static_cast<Type*>(obj)->propName.clear(); \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type str, got " + type); \
	} \
}

static void update_crop(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	std::string error;
	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		std::string type = prop->ob_type->tp_name;
		error = "Expected types tuple or list, got " + type;
	}else
	if (Py_SIZE(prop) != 4) {
		std::string size = std::to_string(Py_SIZE(prop));
		error = "Expected sequence with size == 4, got " + size;
	}
	if (!error.empty()) {
		obj->xcropIsFloat = obj->ycropIsFloat = obj->wcropIsFloat = obj->hcropIsFloat = false;
		obj->xcrop = obj->ycrop = obj->wcrop = obj->hcrop = 0;
		outError(obj, "crop", propIndex, error);
		return;
	}

#define partProc(part, index) \
	PyObject *part = PySequence_Fast_GET_ITEM(prop, index); \
	updateCondition(obj->part##cropIsFloat, obj->part##crop, part, #part "crop") \
	else { \
		obj->part##cropIsFloat = false; \
		obj->part##crop = 0; \
		std::string type = part->ob_type->tp_name; \
		outError(obj, #part "crop", propIndex, "At " #part "crop-prop expected types float, absolute or int, got " + type); \
	}

	partProc(x, 0)
	partProc(y, 1)
	partProc(w, 2)
	partProc(h, 3)
#undef partProc
}


#define makeUpdateTextSize(main_or_hover, propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	bool tmpBool; \
	updateCondition(tmpBool, params.size, prop, #propName) \
	else { \
		params.size = 0; \
		\
		constexpr bool isHoverParam = std::string_view(#main_or_hover) == "hover"; \
		bool ok = isHoverParam && prop == Py_None; \
		if (!ok) { \
			std::string type = prop->ob_type->tp_name; \
			outError(obj, #propName, propIndex, "Expected types float, absolute or int, got " + type); \
		} \
	} \
}

#define makeUpdateTextFunc(main_or_hover, propName, mask) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	if (prop == Py_None) { \
		params.set_font_style &= Uint8(~mask); \
		params.font_style &= Uint8(~mask); \
	}else { \
		params.set_font_style |= Uint8(mask); \
		\
		if (prop == Py_True) { \
			params.font_style |= Uint8(mask); \
		}else { \
			params.font_style &= Uint8(~mask); \
			if (prop != Py_False) { \
				std::string type = prop->ob_type->tp_name; \
				outError(obj, #propName, propIndex, "Expected bool, got " + type); \
			} \
		} \
	} \
}

#define makeUpdateFont(main_or_hover, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	if (PyUnicode_CheckExact(prop)) { \
		params.font = PyUnicode_AsUTF8(prop); \
		params.set_font = true; \
	}else { \
		params.font.clear(); \
		params.set_font = false; \
		\
		constexpr bool isHoverParam = std::string_view(#main_or_hover) == "hover"; \
		bool ok = isHoverParam && prop == Py_None; \
		if (!ok) { \
			std::string type = prop->ob_type->tp_name; \
			outError(obj, #funcPostfix, propIndex, "Expected type str, got " + type); \
		} \
	} \
}

#define makeUpdateTextColor(main_or_hover, propName, simpleName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	constexpr bool isOutline = std::string_view(#simpleName) == "outlinecolor"; \
	\
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	Uint32 v = Uint32(-1); \
	if (PyLong_CheckExact(prop)) { \
		int overflow; \
		long d = PyLong_AsLongAndOverflow(prop, &overflow); \
		if (overflow || d < 0 || d > 0xFFFFFF) { \
			std::string s = PyUtils::objToStr(prop); \
			outError(obj, #propName, propIndex, "Expected value between 0 and 0xFFFFFF, got <" + s + ">"); \
		}else { \
			v = Uint32(d); \
		} \
	}else { \
		constexpr bool isHoverParam = std::string_view(#main_or_hover) == "hover"; \
		bool ok = (isHoverParam || isOutline) && (prop == Py_None || prop == Py_False); \
		if (!ok) { \
			if constexpr (isOutline) { \
				std::string type = prop == Py_True ? "True" : prop->ob_type->tp_name; \
				outError(obj, #propName, propIndex, "Expected type int, None or False, got " + type); \
			}else { \
				std::string type = prop->ob_type->tp_name; \
				outError(obj, #propName, propIndex, "Expected type int, got " + type); \
			} \
		} \
	} \
	\
	if (v != Uint32(-1)) { \
		params.simpleName = v; \
		params.set_##simpleName = true; \
	}else { \
		params.simpleName = 0; \
		params.set_##simpleName = false; \
	} \
}
#define makeUpdateTextAlignFunc(main_or_hover, propName, funcPostfix, zero, one) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	auto text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	if (PyUnicode_CheckExact(prop)) { \
		std::string valueStr = PyUnicode_AsUTF8(prop); \
		float value = 0; \
		if (valueStr == zero) { \
			value = 0; \
		}else \
		if (valueStr == "center") { \
			value = 0.5; \
		}else \
		if (valueStr == one) { \
			value = 1; \
		}else { \
			outError(obj, #propName, propIndex, "Expected str " zero ", center or " one ", got <" + valueStr + ">"); \
		} \
		params.propName = value; \
		params.set_##propName = true; \
	}else \
	if (PyFloat_CheckExact(prop)) { \
		params.propName = float(PyFloat_AS_DOUBLE(prop)); \
		params.set_##propName = true; \
	}else { \
		params.propName = 0; \
		params.set_##propName = false; \
		\
		if constexpr (std::string_view(#main_or_hover) == "main") { \
			std::string type = prop->ob_type->tp_name; \
			outError(obj, #propName, propIndex, "Expected types str or float, got " + type); \
		}else { \
			if (prop != Py_None) { \
				std::string type = prop->ob_type->tp_name; \
				outError(obj, #propName, propIndex, "Expected types str, float or None, got " + type); \
			} \
		} \
	} \
}

#include "gui/screen/style.h"
static void update_style(Child *obj, size_t propIndex) {
	PyObject *style = PySequence_Fast_GET_ITEM(obj->props, propIndex);
	obj->style = StyleManager::getByNode(obj->node, style);
}


makeUpdateFunc(alpha)
makeUpdateFunc(rotate)
makeUpdateFuncWithBool(Child, clipping, clipping)
makeUpdateFuncWithBool(Child, skip_mouse, skip_mouse)

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

makeUpdateFuncWithStr(Child, first_param, first_param)

makeUpdateFuncType(Key, delay)
makeUpdateFuncType(Key, first_delay)

makeUpdateFuncType(Screen, zorder)
makeUpdateFuncWithBool(Screen, ignore_modal, ignore_modal)
makeUpdateFuncWithBool(Screen, modal, modal)
makeUpdateFuncWithBool(Screen, save, save)

makeUpdateFuncType(Container, spacing)

makeUpdateFuncWithStr(TextButton, ground, ground_textbutton)
makeUpdateFuncWithStr(TextButton, hover, hover_textbutton)
makeUpdateFuncWithBool(TextButton, btnRect.buttonMode, mouse_textbutton)

makeUpdateFuncWithBool(Hotspot, btnRect.buttonMode, mouse_hotspot)

makeUpdateFuncWithStr(Imagemap, groundPath, ground_imagemap)
makeUpdateFuncWithStr(Imagemap, hoverPath, hover_imagemap)


#define makeTextFuncs(main_or_hover, infix) \
makeUpdateFont(main_or_hover, infix##font) \
makeUpdateTextSize(main_or_hover, infix##text_size) \
makeUpdateTextColor(main_or_hover, infix##color, color) \
makeUpdateTextColor(main_or_hover, infix##outlinecolor, outlinecolor) \
makeUpdateTextFunc(main_or_hover, infix##bold, TTF_STYLE_BOLD) \
makeUpdateTextFunc(main_or_hover, infix##italic, TTF_STYLE_ITALIC) \
makeUpdateTextFunc(main_or_hover, infix##underline, TTF_STYLE_UNDERLINE) \
makeUpdateTextFunc(main_or_hover, infix##strikethrough, TTF_STYLE_STRIKETHROUGH) \
makeUpdateTextAlignFunc(main_or_hover, halign, infix##text_align, "left", "right") \
makeUpdateTextAlignFunc(main_or_hover, valign, infix##text_valign, "top", "bottom")

makeTextFuncs(main, )
makeTextFuncs(hover, hover_)


static std::map<std::string, ScreenUpdateFunc> mapScreenFuncs = {
	{ "xanchor", update_xanchorPre },
	{ "yanchor", update_yanchorPre },
	{ "anchor", update_anchorPre },

//addProp(style) -> { "style", update_style },
#define addProp(name) { #name, update_##name },
	addProp(style)
	addProp(alpha)
	addProp(rotate)
	addProp(clipping)
	addProp(skip_mouse)
	addProp(delay)
	addProp(first_delay)
	addProp(first_param)
	addProp(crop)
	addProp(zorder)
	addProp(ignore_modal)
	addProp(modal)
	addProp(save)
	addProp(spacing)
	addProp(ground_textbutton)
	addProp(hover_textbutton)
	addProp(mouse_textbutton)
	addProp(mouse_hotspot)
	addProp(ground_imagemap)
	addProp(hover_imagemap)

#define addXYProp(name) addProp(x##name) addProp(y##name) addProp(name)
	addXYProp(pos)
	addXYProp(size)
	addXYProp(zoom)
	addXYProp(align)
#undef addXYProp

#define addTextProp(name) addProp(name) addProp(hover_##name)
	addTextProp(color)
	addTextProp(outlinecolor)
	addTextProp(font)
	addTextProp(text_align)
	addTextProp(text_valign)
	addTextProp(text_size)
	addTextProp(bold)
	addTextProp(italic)
	addTextProp(underline)
	addTextProp(strikethrough)

#undef addTextProp
#undef addProp
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

		for (const Node *child : node->children) {
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
