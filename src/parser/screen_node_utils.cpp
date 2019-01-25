#include "screen_node_utils.h"

#include <iostream>
#include <set>

#include "logger.h"

#include "gui/screen/screen.h"
#include "gui/screen/key.h"
#include "gui/screen/text.h"
#include "gui/screen/textbutton.h"
#include "gui/screen/hotspot.h"
#include "gui/screen/imagemap.h"

#include "media/py_utils.h"
#include "utils/utils.h"



static void initConsts(Node *node);
static void initIsEnd(Node *node);
static void initNums(Node *node);
static String initCode(Node *node);

static void initUpdateFuncs(Node *node);



static std::map<Node*, String> screenCodes;
static std::map<Node*, std::vector<ScreenUpdateFunc>> screenUpdateFuncs;


String ScreenNodeUtils::getScreenCode(Node *screenNode) {
	return screenCodes[screenNode];
}
const std::vector<ScreenUpdateFunc>& ScreenNodeUtils::getUpdateFuncs(Node *node) {
	return screenUpdateFuncs[node];
}


void ScreenNodeUtils::init(Node *node) {
	if (screenCodes.find(node) != screenCodes.end()) return;

	initConsts(node);
	initIsEnd(node);
	initNums(node);

	String screenCode = initCode(node);
	screenCodes[node] = screenCode;

	initUpdateFuncs(node);
}

void ScreenNodeUtils::clear() {
	screenCodes.clear();
	screenUpdateFuncs.clear();
}


static void initConsts(Node *node) {
	node->countPropsToCalc = 0;

	if (node->isScreenProp) {
		node->isScreenConst = !node->isScreenEvent && PyUtils::isConstExpr(node->params);
		return;
	}
	if (node->command == "$" || node->command == "python") return;

	for (Node *child : node->children) {
		initConsts(child);

		if (child->isScreenProp && !child->isScreenConst && !child->isScreenEvent) {
			++node->countPropsToCalc;
		}
	}

	if (node->command == "if" || node->command == "elif" || node->command == "else") return;

	for (const Node *child : node->children) {
		if (!child->isScreenConst) return;
	}
	node->isScreenConst = true;
}

static void initIsEnd(Node *node) {
	if (node->isScreenProp) return;
	if (node->command == "$" || node->command == "python") return;

	for (Node *child : node->children) {
		initIsEnd(child);
	}

	if (node->command == "if" || node->command == "elif" || node->command == "else") return;

	for (const Node *child : node->children) {
		if (!child->isScreenProp && !child->isScreenConst) return;
	}
	node->isScreenEnd = true;
}

static void initNums(Node *node) {
	size_t i = 0;
	for (Node *child : node->children) {
		if (child->isScreenConst || child->command == "$" || child->command == "python") continue;

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


static void initChildCode(Node *child, String &res, const String &indent, const String &index) {
	if (child->isScreenConst || child->isScreenEvent) return;

	String childRes = initCode(child);

	if (child->isScreenProp || child->isScreenEnd) {
		res += indent + "SL_last[" + index + "] = " + childRes + '\n';
		return;
	}


	const String &childCommand = child->command;
	if (childCommand != "elif" && childCommand != "else") {
		if (childCommand != "$" && childCommand != "python") {
			res += indent + "SL_stack.append(SL_last)\n";
		}

		res += indent + "try:\n";
	}

	std::vector<String> code = childRes.split('\n');
	for (const String &line : code) {
		res += indent + "    " + line + '\n';
	}

	bool hasEnd = true;
	if (childCommand == "if" || childCommand == "elif") {                       // child is not-end-condition
		for (size_t i = child->childNum + 1; i < child->parent->children.size(); ++i) {
			Node *nextChild = child->parent->children[i];                       // and nextChild
			if (nextChild->command == "elif" || nextChild->command == "else") { // is may be end-condition
				hasEnd = false;
				break;
			}
		}
	}
	if (hasEnd) {
		res += indent + "except:\n";
		res += indent + "    error_processing()\n";

		if (childCommand != "$" && childCommand != "python") {
			res += indent + "finally:\n";
			res += indent + "    SL_last = SL_stack.pop()\n";
		}
	}
}


static String initCycleCode(Node *node) {
	String res;
	String id = "SL_counter" + String(node->id);
	String indent = "    ";

	const String &command = node->command;
	const String &params = node->params;

	size_t count = maxScreenChild(node) + 1;

	PyUtils::exec(node->getFileName(), node->getNumLine(), "SL_counter" + String(node->id) + " = 0");

	res +=
			"SL_last[" + String(node->screenNum) + "] = SL_last = [None] * " + id + "\n"
			"SL_last_len = " + id + "\n" +
			id + " = 0\n"
			"\n" +
			command + ' ' + params + ":\n" +
			indent + "\n";

	res +=
			indent + "if " + id + " == SL_last_len:\n" +
			indent + "    SL_last += [None] * " + (100 * count) + "\n" +
			indent + "    SL_last_len += " + (100 * count) + "\n" +
			indent + "\n";


	for (Node *child : node->children) {
		if (child->isScreenConst) continue;

		String index = id;
		if (child->screenNum) {
			index += " + " + String(child->screenNum);
		}
		initChildCode(child, res, indent, index);

		if (child->screenNum != count - 1) {
			res += indent + '\n';
		}
	}

	res += indent + id + " += " + count + "\n";

	return res;
}

static String initCode(Node *node) {
	if (node->isScreenConst || node->isScreenEvent) return "";

	const String &command = node->command;
	const String &params = node->params;

	if (node->isScreenProp) return params;
	if (command == "continue" || command == "break") {
		Node *t = node->parent;
		while (t->command != "for" && t->command != "while") {
			t = t->parent;
		}

		return "SL_counter" + String(t->id) + " += " + String(maxScreenChild(t)) + "\n" +
				command + "\n";
	}

	if (command == "$" || command == "python") return params;

	if (command == "for" || command == "while") return initCycleCode(node);


	String res;

	if (node->isScreenEnd) {
		res = "(\n";
		for (Node *child : node->children) {
			if (child->isScreenConst || child->isScreenEvent) continue;

			String childRes = initCode(child);
			res += "    " + childRes + ",\n";
		}
		if (res.size() != 2) {
			res += ")";
		}else {
			res[1] = ')';
		}

		for (Node *child : node->children) {
			if (child->isScreenEvent) {
				res += "\nSL_check_events()";
				break;
			}
		}

		return res;
	}


	String indent;

	if (command == "screen") {
		res = "SL_stack = []\n"
			  "SL_last = None\n\n";
	}else
	if (command == "if" || command == "elif" || command == "else") {
		res = command + " " + params + ":\n";
		indent += "    ";
	}

	res += indent;
	if (command != "screen") {
		res += "SL_last[" + String(node->screenNum) + "] = ";
	}
	res += "SL_last = [\n";
	size_t bracketBegin = res.size() - 2;

	size_t lastPropNum = size_t(-1);
	size_t maxChildNum = maxScreenChild(node);

	//props before first obj
	for (size_t i = 0; i < node->children.size(); ++i) {
		Node *child = node->children[i];
		if (child->isScreenConst) continue;
		if (!child->isScreenProp) break;

		String childRes = initCode(child);
		res += indent + "    " + childRes + ",\n";
		lastPropNum = child->screenNum;
	}

	if (maxChildNum != size_t(-1)) {
		res += (indent + "    None,\n").repeat(maxChildNum - lastPropNum);
		res += indent + "]";
	}else {
		res.back() = ']';
	}

	if (lastPropNum == maxChildNum) {
		res[bracketBegin] = '(';
		res.back() = ')';
	}else {
		res += "\n\n";

		for (Node *child : node->children) {
			if (child->isScreenConst) continue;
			if (child->screenNum <= lastPropNum && lastPropNum != size_t(-1)) continue;

			initChildCode(child, res, indent, String(child->screenNum));

			if (child->screenNum != maxChildNum) {
				res += indent + '\n';
			}
		}
	}


	if (command == "screen") {
		Logger::log("\n\n\n" + res);
//		abort();
	}

	return res;
}






static void outError(Child *obj, const String &propName, size_t propIndex, const String &expected) {
	for (Node *child : obj->node->children) {
		if (child->isScreenProp && child->screenNum == propIndex) {
			String desc = expected + '\n';

			if (child->command == propName) {
				desc += child->getPlace();
			}else {
				String style = obj->node->command;
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

static bool tmpBool;

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
	updateCondition(tmpBool, static_cast<Type*>(obj)->propName, prop) \
	else { \
		String type = prop->ob_type->tp_name; \
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
		String type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type bool, got " + type); \
	} \
}

#define makeUpdateFuncWithIsDouble(propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	updateCondition(obj->propName##IsDouble, obj->propName, prop) \
	else { \
		String type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, int or long, got " + type); \
	} \
}

#define makeUpdateCommonFunc(propName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	\
	if (!PyList_CheckExact(prop) && !PyTuple_CheckExact(prop)) { \
		String type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types list or tuple, got " + type); \
		return; \
	} \
	if (Py_SIZE(prop) != 2) { \
		String size = int(Py_SIZE(prop)); \
		outError(obj, #propName, propIndex, "Expected sequence with size == 2, got " + size); \
		return; \
	} \
	\
	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0); \
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1); \
	\
	updateCondition(obj->x##propName##IsDouble, obj->x##propName, x) \
	else { \
		String type = x->ob_type->tp_name; \
		outError(obj, "x"#propName, propIndex, "At x" #propName "-prop expected types float, int or long, got " + type); \
	} \
	updateCondition(obj->y##propName##IsDouble, obj->y##propName, y) \
	else { \
		String type = y->ob_type->tp_name; \
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
		String type = prop->ob_type->tp_name; \
		outError(obj, #x_or_y"align", propIndex, "Expected types float, int or long, got " + type); \
	} \
}

static void update_align(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	if (!PyList_CheckExact(prop) && !PyTuple_CheckExact(prop)) {
		String type = prop->ob_type->tp_name;
		outError(obj, "align", propIndex, "Expected types list or tuple, got " + type);
		return;
	}
	if (Py_SIZE(prop) != 2) {
		String size = int(Py_SIZE(prop));
		outError(obj, "align", propIndex, "Expected sequence with size == 2, got " + size);
		return;
	}

	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0);
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1);

	updateCondition(obj->xposIsDouble = obj->xanchorPreIsDouble,
					obj->xpos = obj->xanchorPre, x)
	else {
		String type = x->ob_type->tp_name;
		outError(obj, "align", propIndex, "At xalign-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->yposIsDouble = obj->yanchorPreIsDouble,
					obj->ypos = obj->yanchorPre, y)
	else {
		String type = y->ob_type->tp_name;
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
		String type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type str, got " + type); \
	} \
}
#define makeUpdateFuncWithStrSimple(Type, propName) makeUpdateFuncWithStr(Type, propName, propName)

static void update_crop(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		String type = prop->ob_type->tp_name;
		outError(obj, "crop", propIndex, "Expected types tuple or list, got " + type);
		return;
	}
	if (Py_SIZE(prop) != 4) {
		String size = int(Py_SIZE(prop));
		outError(obj, "crop", propIndex, "Expected sequence with size == 4, got " + size);
		return;
	}

	PyObject *x = PySequence_Fast_GET_ITEM(prop, 0);
	PyObject *y = PySequence_Fast_GET_ITEM(prop, 1);
	PyObject *w = PySequence_Fast_GET_ITEM(prop, 2);
	PyObject *h = PySequence_Fast_GET_ITEM(prop, 3);

	updateCondition(obj->xcropIsDouble, obj->xcrop, x)
	else {
		String type = x->ob_type->tp_name;
		outError(obj, "xcrop", propIndex, "At xcrop-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->ycropIsDouble, obj->ycrop, y)
	else {
		String type = y->ob_type->tp_name;
		outError(obj, "ycrop", propIndex, "At ycrop-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->wcropIsDouble, obj->wcrop, w)
	else {
		String type = w->ob_type->tp_name;
		outError(obj, "wcrop", propIndex, "At wcrop-prop expected types float, int or long, got " + type);
	}

	updateCondition(obj->hcropIsDouble, obj->hcrop, h)
	else {
		String type = h->ob_type->tp_name;
		outError(obj, "hcrop", propIndex, "At hcrop-prop expected types float, int or long, got " + type);
	}
}


makeUpdateFunc(alpha)
makeUpdateFunc(rotate)

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


static std::map<String, ScreenUpdateFunc> mapScreenFuncs = {
	{"alpha",             update_alpha},
	{"rotate",            update_rotate},
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
	std::vector<ScreenUpdateFunc> vec;

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

	vec.resize(maxScreenNum + 1, nullptr);

	for (Node *child : node->children) {
		if (!child->isScreenProp || child->isScreenConst || child->isScreenEvent) continue;

		ScreenUpdateFunc func = ScreenNodeUtils::getUpdateFunc(node->command, child->command);
		vec[child->screenNum] = func;
	}

	screenUpdateFuncs[node] = vec;
}

ScreenUpdateFunc ScreenNodeUtils::getUpdateFunc(String type, String propName) {
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


