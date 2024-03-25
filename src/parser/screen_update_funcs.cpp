#include "screen_update_funcs.h"

#include <map>

#include <SDL2/SDL_ttf.h>
#undef main

#include "gui/screen/hotspot.h"
#include "gui/screen/imagemap.h"
#include "gui/screen/key.h"
#include "gui/screen/screen.h"
#include "gui/screen/text.h"
#include "gui/screen/textbutton.h"

#include "media/py_utils.h"
#include "media/py_utils/absolute.h"

#include "utils/algo.h"
#include "utils/string.h"
#include "utils/utils.h"


#undef PyFloat_AS_DOUBLE
//copypaste, but without assert(type(op) is float) in debug-mode
#define PyFloat_AS_DOUBLE(op) _Py_CAST(PyFloatObject*, op)->ob_fval


static void outError(Child *obj, const std::string &propName, size_t propIndex, const std::string &expected) {
	Node *node = obj->node;
	if (node->command == "use") {
		node = Screen::getDeclared(node->params);
		if (!node) {
			Utils::outMsg("ScreenUpdateFuncs::outError", "Error on out error: screen <" + obj->node->params + "> not found (?)");
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



#define makeUpdateFuncWithBool(Type, propName, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	Type *typedObj = static_cast<Type*>(obj); \
	\
	if (PyBool_Check(prop)) { \
		typedObj->propName = prop == Py_True; \
	}else { \
		typedObj->propName = false; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected type bool, got " + type); \
	} \
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


#define makeUpdateFuncTemplate(Type, propName, isFloat) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	Type *typedObj = static_cast<Type*>(obj); \
	\
	[[maybe_unused]] \
	bool tmpBool; \
	updateCondition(isFloat, typedObj->propName, prop, #propName) \
	else { \
		typedObj->propName = 0; \
		std::string type = prop->ob_type->tp_name; \
		outError(obj, #propName, propIndex, "Expected types float, absolute or int, got " + type); \
	} \
}

#define makeUpdateFuncType(Type, propName) makeUpdateFuncTemplate(Type, propName, tmpBool)
#define makeUpdateFunc(propName) makeUpdateFuncType(Child, propName)
#define makeUpdateFuncTypeWithIsFloat(Type, propName) makeUpdateFuncTemplate(Type, propName, typedObj->propName##_is_float)
#define makeUpdateFuncWithIsFloat(propName) makeUpdateFuncTypeWithIsFloat(Child, propName)

#define makeUpdateCommonFuncWithIsFloat(propName, xIsFloat, yIsFloat) \
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

#define makeUpdateCommonFunc(propName) makeUpdateCommonFuncWithIsFloat(propName, obj->x##propName##_is_float, obj->y##propName##_is_float)
#define makeUpdateCommonFuncWithoutIsFloat(propName) makeUpdateCommonFuncWithIsFloat(propName, unusedBoolX, unusedBoolY)


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
	obj->x_or_y##pos_is_float = obj->x_or_y##anchor_is_float = isFloat; \
	obj->x_or_y##pos = obj->x_or_y##anchor = value; \
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
		obj->xpos_is_float = obj->xanchor_is_float = obj->ypos_is_float = obj->yanchor_is_float = isFloat;
		obj->xpos = obj->xanchor = obj->ypos = obj->yanchor = value;
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
		obj->xpos_is_float = obj->xanchor_is_float = obj->ypos_is_float = obj->yanchor_is_float = false;
		obj->xpos = obj->xanchor = obj->ypos = obj->yanchor = 0;
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
	obj->part##pos_is_float = obj->part##anchor_is_float = isFloat; \
	obj->part##pos = obj->part##anchor = value;

	partProc(x, 0);
	partProc(y, 1);
#undef partProc
}

#define makeUpdateFuncWithStr(Type, propName, funcPostfix) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	Type *typedObj = static_cast<Type*>(obj); \
	\
	if (PyUnicode_CheckExact(prop)) { \
		typedObj->propName = PyUtils::objToStr(prop); \
	}else { \
		typedObj->propName.clear(); \
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
		obj->xcrop_is_float = obj->ycrop_is_float = obj->wcrop_is_float = obj->hcrop_is_float = false;
		obj->xcrop = obj->ycrop = obj->wcrop = obj->hcrop = 0;
		outError(obj, "crop", propIndex, error);
		return;
	}

#define partProc(part, index) \
	PyObject *part = PySequence_Fast_GET_ITEM(prop, index); \
	updateCondition(obj->part##crop_is_float, obj->part##crop, part, #part "crop") \
	else { \
		obj->part##crop_is_float = false; \
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


#define makeUpdateTextSize(main_or_hover, propName, postfix) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	Text *text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	updateCondition(params.size##postfix##_is_float, params.size##postfix, prop, #propName) \
	else { \
		params.size##postfix##_is_float = false; \
		params.size##postfix = 0; \
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
	Text *text = static_cast<Text*>(obj); \
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
	Text *text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	if (PyUnicode_CheckExact(prop)) { \
		params.font = PyUtils::objToStr(prop); \
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

static Uint32 getColor(PyObject *prop, std::string &error, bool canBeDisabled) {
	Uint32 fail = Uint32(-1);
	int overflow;

	if (PyLong_CheckExact(prop)) {
		long color = PyLong_AsLongAndOverflow(prop, &overflow);
		if (overflow || color < 0 || color > 0xFFFFFF) {
			std::string s = PyUtils::objToStr(prop);
			error = "Expected value between 0 and 0xFFFFFF, got " + s;
			return fail;
		}
		return Uint32(color);
	}

	if (PyUnicode_CheckExact(prop)) {
		std::string str = PyUtils::objToStr(prop);
		if (String::startsWith(str, "#")) {
			str.erase(0, 1);
		}else
		if (String::startsWith(str, "0x")) {
			str.erase(0, 2);
		}

		size_t size = str.size();
		if (size != 3 && size != 6) {
			error = "Expected str with len 3 or 6 symbols, got " + std::to_string(size);
			return fail;
		}

		int ints[6];
		for (size_t i = 0; i < size; ++i) {
			char c = str[i];
			bool isNum        = c >= '0' && c <= '9';
			bool isSmallAlpha = c >= 'a' && c <= 'f';
			bool isBigAlpha   = c >= 'A' && c <= 'F';
			if (!isNum && !isSmallAlpha && !isBigAlpha) {
				std::string s = PyUtils::objToStr(prop);
				error = "Expected color symbols in [0-9], [a-f] and [A-F], got <" + s + ">";
				return fail;
			}

			if (isNum) {
				ints[i] = c - '0';
			}else
			if (isSmallAlpha) {
				ints[i] = c - 'a' + 10;
			}else {
				ints[i] = c - 'A' + 10;
			}
		}

		int r, g, b;
		if (size == 3) {
			r = ints[0] * 17;
			g = ints[1] * 17;
			b = ints[2] * 17;
		}else {
			r = ints[0] * 16 + ints[1];
			g = ints[2] * 16 + ints[3];
			b = ints[4] * 16 + ints[5];
		}
		return Uint32((r << 16) + (g << 8) + b);
	}

	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		std::string type = prop->ob_type->tp_name;
		if (canBeDisabled) {
			error = "Expected types int, str, tuple, list, None or False, got ";
		}else {
			error = "Expected types int, str, tuple or list, got ";
		}
		error += type;
		return fail;
	}
	if (Py_SIZE(prop) != 3) {
		std::string size = std::to_string(Py_SIZE(prop));
		error = "Expected sequence with size == 3, got " + size;
		return fail;
	}

#define partProc(ch, index) \
	PyObject *ch##Py = PySequence_Fast_GET_ITEM(prop, index); \
	if (!PyLong_CheckExact(ch##Py)) { \
		std::string type = ch##Py->ob_type->tp_name; \
		error = "Expected sequence with ints, got " + type; \
		return fail; \
	} \
	long ch = PyLong_AsLongAndOverflow(ch##Py, &overflow); \
	if (overflow || ch < 0 || ch > 255) { \
		std::string s = PyUtils::objToStr(ch##Py); \
		error = "Expected sequence with ints between 0 and 255, got " + s; \
		return fail; \
	}

	partProc(r, 0);
	partProc(g, 1);
	partProc(b, 2);
#undef partProc

	return Uint32((r << 16) + (g << 8) + b);
}

#define makeUpdateTextColor(main_or_hover, propName, simpleName) \
static void update_##propName(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	Text *text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	constexpr bool isHoverParam = std::string_view(#main_or_hover) == "hover"; \
	constexpr bool isOutline = std::string_view(#simpleName) == "outlinecolor"; \
	constexpr bool canBeDisabled = isHoverParam || isOutline; \
	bool disabled = (prop == Py_None || prop == Py_False); \
	\
	Uint32 v = Uint32(-1); \
	std::string error; \
	if (!canBeDisabled || !disabled) { \
		v = getColor(prop, error, canBeDisabled); \
	} \
	\
	if (v != Uint32(-1)) { \
		params.simpleName = v; \
		params.set_##simpleName = true; \
	}else { \
		params.simpleName = 0; \
		params.set_##simpleName = false; \
		\
		if (!error.empty()) { \
			outError(obj, #propName, propIndex, error); \
		} \
	} \
}

#define makeUpdateTextAlignFunc(main_or_hover, propName, funcPostfix, zero, one) \
static void update_##funcPostfix(Child *obj, size_t propIndex) { \
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex); \
	Text *text = static_cast<Text*>(obj); \
	auto &params = text->main_or_hover##Params; \
	\
	if (PyUnicode_CheckExact(prop)) { \
		std::string valueStr = PyUtils::objToStr(prop); \
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


static void update_corner_sizes(Child *obj, size_t propIndex) {
	PyObject *prop = PySequence_Fast_GET_ITEM(obj->props, propIndex);

	bool isFloat;
	bool isNumber = true;
	float value = 0;
	updateCondition(isFloat, value, prop, "corner_sizes")
	else {
		isNumber = false;
	}

#define setCornerSize(side) \
	obj->corner_sizes_##side##_is_float = isFloat; \
	obj->corner_sizes_##side = value;
#define setAllCornerSizes() \
	setCornerSize(left) \
	setCornerSize(top) \
	setCornerSize(right) \
	setCornerSize(bottom)

	if (isNumber) {
		setAllCornerSizes()
		return;
	}

	std::string error;
	size_t len = 0;
	if (!PyTuple_CheckExact(prop) && !PyList_CheckExact(prop)) {
		std::string type = prop->ob_type->tp_name;
		error = "Expected types float, absolute, int, tuple or list, got " + type;
	}else {
		len = size_t(Py_SIZE(prop));
		if (len != 2 && len != 4) {
			std::string size = std::to_string(len);
			error = "Expected sequence with size == 2 or size == 4, got " + size;
		}
	}
	if (!error.empty()) {
		isFloat = false;
		value = 0;
		setAllCornerSizes()
		outError(obj, "corner_sizes", propIndex, error);
		return;
	}

#define partProc(side, i) \
	PyObject *side##Py = PySequence_Fast_GET_ITEM(prop, i); \
	updateCondition(isFloat, value, side##Py, "corner_sizes[" #i "]") \
	else { \
		isFloat = false; \
		value = 0; \
		std::string type = side##Py->ob_type->tp_name; \
		std::string msg = "At corner_sizes[" #i "] expected types float, absolute or int, got " + type; \
		outError(obj, "corner_sizes", propIndex, msg); \
	} \
	setCornerSize(side)

	{
		partProc(left, 0);
		partProc(top, 1);
	}

	if (len == 2) {
		obj->corner_sizes_right_is_float = obj->corner_sizes_left_is_float;
		obj->corner_sizes_bottom_is_float = obj->corner_sizes_top_is_float;
		obj->corner_sizes_right = obj->corner_sizes_left;
		obj->corner_sizes_bottom = obj->corner_sizes_top;
	}else {
		partProc(right, 2);
		partProc(bottom, 3);
	}

#undef setCornerSize
#undef setAllCornerSizes
#undef partProc
}


#include "gui/screen/style.h"
static void update_style(Child *obj, size_t propIndex) {
	PyObject *style = PySequence_Fast_GET_ITEM(obj->props, propIndex);
	obj->style = StyleManager::getByObject(obj, style);
}



makeUpdateFunc(alpha)
makeUpdateFunc(rotate)
makeUpdateFuncWithBool(Child, clipping, clipping)
makeUpdateFuncWithBool(Child, skip_mouse, skip_mouse)

makeUpdateFuncTypeWithIsFloat(Container, spacing)
makeUpdateFuncTypeWithIsFloat(Container, spacing_min)
makeUpdateFuncTypeWithIsFloat(Container, spacing_max)

makeUpdateFuncWithIsFloat(xpos)
makeUpdateFuncWithIsFloat(ypos)

makeUpdateFuncWithIsFloat(xanchor)
makeUpdateFuncWithIsFloat(yanchor)

makeUpdateFuncWithIsFloat(xsize)
makeUpdateFuncWithIsFloat(ysize)
makeUpdateFuncWithIsFloat(xsize_min)
makeUpdateFuncWithIsFloat(ysize_min)
makeUpdateFuncWithIsFloat(xsize_max)
makeUpdateFuncWithIsFloat(ysize_max)

makeUpdateFunc(xzoom)
makeUpdateFunc(yzoom)

makeUpdateCommonFunc(pos)
makeUpdateCommonFunc(anchor)
makeUpdateCommonFunc(size)
makeUpdateCommonFunc(size_min)
makeUpdateCommonFunc(size_max)
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

makeUpdateFuncWithStr(TextButton, ground, ground_textbutton)
makeUpdateFuncWithStr(TextButton, hover, hover_textbutton)
makeUpdateFuncWithBool(TextButton, btnRect.buttonMode, mouse_textbutton)

makeUpdateFuncWithBool(Hotspot, btnRect.buttonMode, mouse_hotspot)

makeUpdateFuncWithBool(Child, selected, selected)

makeUpdateFuncWithStr(Imagemap, groundPath, ground_imagemap)
makeUpdateFuncWithStr(Imagemap, hoverPath, hover_imagemap)


#define makeTextFuncs(main_or_hover, infix) \
makeUpdateFont(main_or_hover, infix##font) \
makeUpdateTextSize(main_or_hover, infix##text_size, ) \
makeUpdateTextSize(main_or_hover, infix##text_size_min, _min) \
makeUpdateTextSize(main_or_hover, infix##text_size_max, _max) \
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
	addProp(ground_textbutton)
	addProp(hover_textbutton)
	addProp(mouse_textbutton)
	addProp(mouse_hotspot)
	addProp(selected)
	addProp(ground_imagemap)
	addProp(hover_imagemap)
	addProp(corner_sizes)

	addProp(spacing)
	addProp(spacing_min)
	addProp(spacing_max)

#define addXYProp(name) addProp(x##name) addProp(y##name) addProp(name)
	addXYProp(pos)
	addXYProp(anchor)
	addXYProp(size)
	addXYProp(size_min)
	addXYProp(size_max)
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
	addTextProp(text_size_min)
	addTextProp(text_size_max)
	addTextProp(bold)
	addTextProp(italic)
	addTextProp(underline)
	addTextProp(strikethrough)

#undef addTextProp
#undef addProp
};




static std::map<const Node*, const std::vector<ScreenUpdateFunc>*> updateFuncs;

const std::vector<ScreenUpdateFunc>* ScreenUpdateFuncs::getFuncs(const Node *node) {
	return updateFuncs[node];
}

void ScreenUpdateFuncs::initNodeFuncs(Node *node) {
	if (updateFuncs.find(node) != updateFuncs.end()) return;

	size_t maxScreenNum = size_t(-1);
	for (Node *child : node->children) {
		if (child->isScreenProp) {
			if (!child->isScreenConst && !child->isScreenEvent &&
			    (child->screenNum > maxScreenNum || maxScreenNum == size_t(-1)))
			{
				maxScreenNum = child->screenNum;
			}
		}else {
			initNodeFuncs(child);
		}
	}

	std::vector<ScreenUpdateFunc> *vec = nullptr;

	if (maxScreenNum != size_t(-1)) {
		vec = new std::vector<ScreenUpdateFunc>(maxScreenNum + 1, nullptr);

		for (const Node *child : node->children) {
			if (!child->isScreenProp || child->isScreenConst || child->isScreenEvent) continue;

			ScreenUpdateFunc func = ScreenUpdateFuncs::getFunc(node->command, child->command);
			(*vec)[child->screenNum] = func;
		}
	}

	updateFuncs[node] = vec;
}

using Strings = std::initializer_list<std::string>;
static const Strings buttonProps = { "ground", "hover", "mouse" };

ScreenUpdateFunc ScreenUpdateFuncs::getFunc(const std::string &type, std::string propName) {
	if (Algo::in(propName, buttonProps)) {
	    propName += "_" + (type == "button" ? "textbutton" : type);
	}

	auto it = mapScreenFuncs.find(propName);
	if (it != mapScreenFuncs.end()) {
		return it->second;
	}

	Utils::outMsg("ScreenUpdateFuncs::getFunc", "Func <" + propName + "> not found");
	return nullptr;
}


void ScreenUpdateFuncs::clear() {
	for (auto &p : updateFuncs) {
		delete p.second;
	}
	updateFuncs.clear();
}
