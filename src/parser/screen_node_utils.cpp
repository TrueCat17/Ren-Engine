#include "screen_node_utils.h"

#include <map>
#include <set>

#include "gui/screen/screen.h"
#include "gui/screen/key.h"
#include "gui/screen/text.h"
#include "gui/screen/textbutton.h"
#include "gui/screen/hotspot.h"
#include "gui/screen/imagemap.h"

#include "media/py_utils.h"

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


std::string ScreenNodeUtils::getScreenCode(const Node *screenNode) {
	return screenCodes[screenNode];
}
const std::vector<ScreenUpdateFunc>* ScreenNodeUtils::getUpdateFuncs(const Node *node) {
	return screenUpdateFuncs[node];
}


void ScreenNodeUtils::init(Node *node) {
	if (screenCodes.find(node) != screenCodes.end()) return;

	initConsts(node);
	initIsEnd(node);
	initNums(node);

	std::string screenCode = initCode(node);
	screenCodes[node] = screenCode;

	initUpdateFuncs(node);
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
				ScreenNodeUtils::init(screenNode);
				node->isScreenConst = screenNode->isScreenConst;
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

		res += indent + "try:\n";
	}

	for (const std::string &line : code) {
		res += indent + "    " + line + '\n';
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
		res += indent + "except:\n";
		res += indent + "    _SL_error_processing()\n";

		if (childCommand != "$" && childCommand != "python") {
			res += indent + "finally:\n";
			res += indent + "    _SL_last = _SL_stack.pop()\n";
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
		while (t->command != "for" && t->command != "while") {
			t = t->parent;
		}
		std::string id = std::to_string(t->id);

		if (command == "break" &&
		    t->childNum + 1 < t->parent->children.size() &&
		    t->parent->children[t->childNum + 1]->command == "else")
		{
			res = "_SL_break" + id + " = True\n";
		}

		res += "_SL_counter" + id + " += " + std::to_string(maxScreenChild(t) + 1) + "\n" +
		       command + " # " + std::to_string(node->getNumLine()) + " " + node->getFileName() + "\n";
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


	if (command == "screen") {
		if (index.empty()) {
			res += "\n"
				   "_SL_check_events()\n";
		}
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
	if ((isFloat = PyFloat_CheckExact(prop))) { /*set & check*/ \
		var = PyFloat_AS_DOUBLE(prop); \
	}else \
	if (PyInt_CheckExact(prop)) { \
		var = PyInt_AS_LONG(prop); \
	}else \
	if (PyLong_CheckExact(prop)) { \
		var = PyLong_AsDouble(prop); \
	}


#define makeUpdateFuncType(Type, propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	[[maybe_unused]] bool tmpBool; \
	updateCondition(tmpBool, static_cast<Type*>(obj)->propName, prop) \
	else { \
	    std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, int or long, got " + type); \
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
	    std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type bool, got " + type); \
	} \
}

#define makeUpdateFuncWithIsDouble(propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	updateCondition(obj->propName##IsDouble, obj->propName, prop) \
	else { \
	    std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, int or long, got " + type); \
	} \
}

#define makeUpdateCommonFunc(propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	if (!PyList_CheckExact(prop) && !PyTuple_CheckExact(prop)) { \
	    std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types list or tuple, got " + type); \
		return; \
	} \
	if (Py_SIZE(prop) != 2) { \
	    std::string size = std::to_string(Py_SIZE(prop)); \
		outError(obj, #propName, propIndex, "Expected sequence with size == 2, got " + size); \
		return; \
	} \
	\
	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0); \
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1); \
	\
	updateCondition(obj->x##propName##IsDouble, obj->x##propName, x) \
	else { \
	    std::string type = x->ob_type->tp_name; \
		outError(obj, "x"#propName, propIndex, "At x" #propName "-prop expected types float, int or long, got " + type); \
	} \
	updateCondition(obj->y##propName##IsDouble, obj->y##propName, y) \
	else { \
	    std::string type = y->ob_type->tp_name; \
		outError(obj, "y"#propName, propIndex, "At y" #propName "-prop expected types float, int or long, got " + type); \
	} \
}


#define makeUpdateFuncWithAlign(x_or_y) \
static void update_##x_or_y##align(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	updateCondition(obj->x_or_y##posIsDouble = obj->x_or_y##anchorPreIsDouble, \
					obj->x_or_y##pos = obj->x_or_y##anchorPre, prop) \
	else { \
	    std::string type = prop->ob_type->tp_name; \
		outError(obj, #x_or_y"align", propIndex, "Expected types float, int or long, got " + type); \
	} \
}

static void update_align(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	if (!PyList_CheckExact(prop) && !PyTuple_CheckExact(prop)) {
		std::string type = prop->ob_type->tp_name;
		outError(obj, "align", propIndex, "Expected types list or tuple, got " + type);
		return;
	}
	if (Py_SIZE(prop) != 2) {
		std::string size = std::to_string(Py_SIZE(prop));
		outError(obj, "align", propIndex, "Expected sequence with size == 2, got " + size);
		return;
	}

	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0);
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1);

	updateCondition(obj->xposIsDouble = obj->xanchorPreIsDouble,
					obj->xpos = obj->xanchorPre, x)
	else {
		std::string type = x->ob_type->tp_name;
		outError(obj, "align", propIndex, "At xalign-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->yposIsDouble = obj->yanchorPreIsDouble,
					obj->ypos = obj->yanchorPre, y)
	else {
		std::string type = y->ob_type->tp_name;
		outError(obj, "align", propIndex, "At yalign-prop expected types float, int or long, got " + type);
	}
}

#define makeUpdateFuncWithStr(Type, propName, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	if (PyString_CheckExact(prop)) { \
		static_cast<Type*>(obj)->propName = PyString_AS_STRING(prop); \
	}else { \
	    std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type str, got " + type); \
	} \
}
#define makeUpdateFuncWithStrSimple(Type, propName) makeUpdateFuncWithStr(Type, propName, propName)

static void update_crop(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		std::string type = prop->ob_type->tp_name;
		outError(obj, "crop", propIndex, "Expected types tuple or list, got " + type);
		return;
	}
	if (Py_SIZE(prop) != 4) {
		std::string size = std::to_string(Py_SIZE(prop));
		outError(obj, "crop", propIndex, "Expected sequence with size == 4, got " + size);
		return;
	}

	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0);
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1);
	PyObject *w = PySequence_Fast_GET_ITEM(prop, 2);
	PyObject *h = PySequence_Fast_GET_ITEM(prop, 3);

	updateCondition(obj->xcropIsDouble, obj->xcrop, x)
	else {
		std::string type = x->ob_type->tp_name;
		outError(obj, "xcrop", propIndex, "At xcrop-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->ycropIsDouble, obj->ycrop, y)
	else {
		std::string type = y->ob_type->tp_name;
		outError(obj, "ycrop", propIndex, "At ycrop-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->wcropIsDouble, obj->wcrop, w)
	else {
		std::string type = w->ob_type->tp_name;
		outError(obj, "wcrop", propIndex, "At wcrop-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->hcropIsDouble, obj->hcrop, h)
	else {
		std::string type = h->ob_type->tp_name;
		outError(obj, "hcrop", propIndex, "At hcrop-prop expected types float, int or long, got " + type);
	}
}


makeUpdateFunc(alpha)
makeUpdateFunc(rotate)
makeUpdateFuncWithBool(Child, clip, clip)

makeUpdateFuncWithIsDouble(xpos)
makeUpdateFuncWithIsDouble(ypos)

makeUpdateFuncWithIsDouble(xanchorPre)
makeUpdateFuncWithIsDouble(yanchorPre)

makeUpdateFuncWithIsDouble(xsize)
makeUpdateFuncWithIsDouble(ysize)

makeUpdateCommonFunc(pos)
makeUpdateCommonFunc(anchorPre)
makeUpdateCommonFunc(size)

makeUpdateFuncWithAlign(x)
makeUpdateFuncWithAlign(y)

makeUpdateFuncWithStrSimple(Child, first_param)

makeUpdateFuncType(Key, keyDelay)
makeUpdateFuncType(Key, firstKeyDelay)

makeUpdateFuncType(Screen, zorder)
makeUpdateFuncWithBool(Screen, modal, modal)

makeUpdateFuncType(Container, spacing)

makeUpdateFuncType(Text, color)
makeUpdateFuncType(Text, text_size)

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
    {"clip",              update_clip},
	{"delay",             update_keyDelay},
	{"first_delay",       update_firstKeyDelay},
	{"xpos",              update_xpos},
	{"ypos",              update_ypos},
	{"xanchor",           update_xanchorPre},
	{"yanchor",           update_yanchorPre},
	{"xsize",             update_xsize},
	{"ysize",             update_ysize},
	{"pos",               update_pos},
	{"anchor",            update_anchorPre},
	{"size",              update_size},
	{"xalign",            update_xalign},
	{"yalign",            update_yalign},
	{"align",             update_align},
	{"first_param",       update_first_param},
	{"crop",              update_crop},
	{"zorder",            update_zorder},
	{"modal",             update_modal},
	{"spacing",           update_spacing},
	{"color",             update_color},
	{"text_size",         update_text_size},
	{"font",              update_font},
	{"text_align",        update_text_align},
	{"text_valign",       update_text_valign},
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
	ScreenUpdateFunc res = nullptr;

	if (propName == "ground" || propName == "hover" || propName == "mouse") {
		propName += "_" + (type == "button" ? "textbutton" : type);
	}

	auto it = mapScreenFuncs.find(propName);
	if (it != mapScreenFuncs.end()) {
		res = it->second;
	}else {
		Utils::outMsg("ScreenNodeUtils::getUpdateFunc", "Func <" + propName + "> not found");
	}

	return res;
}


