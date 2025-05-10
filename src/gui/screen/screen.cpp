#include "screen.h"

#include <algorithm>
#include <map>

#include "logger.h"
#include "media/py_utils.h"
#include "parser/screen_code_generator.h"
#include "utils/stage.h"
#include "utils/utils.h"


bool Screen::destroyedScreenIsModal = false;


static std::map<std::string, Node*> declared;
static std::map<std::string, std::string> screenMap;
static std::mutex screenMutex;

void Screen::declare(Node *screenNode) {
	declared[screenNode->params] = screenNode;
}
Node* Screen::getDeclared(const std::string &name) {
	auto it = declared.find(name);
	return it != declared.end() ? it->second : nullptr;
}
Screen* Screen::getMain(const std::string &name) {
	if (Stage::screens) {
		for (DisplayObject *d : Stage::screens->children) {
			Screen *scr = static_cast<Screen*>(d);
			if (scr->name == name) {
				return scr;
			}
		}
	}
	return nullptr;
}


static std::vector<std::string> getSameNames(const std::string &name) {
	std::vector<std::string> res = { name };
	for (const auto& [from, to] : screenMap) {
		if (to == name) {
			res.push_back(from);
		}
	}
	return res;
}

static void show(const std::string &name) {
	Screen *scr = Screen::getMain(name);
	if (!scr) {
		Node *node = Screen::getDeclared(name);
		if (!node) {
			Utils::outMsg("Screen::show", "Screen <" + name + "> is not defined");
			return;
		}

		scr = new Screen(node, nullptr);
	}

	for (const auto &sameName : getSameNames(name)) {
		PyUtils::execWithSetTmp("CPP_EMBED: screen.cpp", __LINE__,
		                        "signals.send('show_screen', tmp)", sameName);
	}
	Stage::screens->addChildAt(scr, Stage::screens->children.size());
}
static void hide(const std::string &name) {
	if (!Stage::screens) return;

	Screen *scr = Screen::getMain(name);
	if (!scr) {
		Utils::outMsg("Screen::hide", "Screen <" + name + "> is not shown");
		return;
	}

	delete scr;
	PyUtils::execWithSetTmp("CPP_EMBED: screen.cpp", __LINE__,
	                        "del screen_vars[tmp]", name);

	for (const auto &sameName : getSameNames(name)) {
		PyUtils::execWithSetTmp("CPP_EMBED: screen.cpp", __LINE__,
		                        "signals.send('hide_screen', tmp)", sameName);
	}
}

static std::vector<std::string> toShowList;
static std::vector<std::string> toHideList;
void Screen::updateLists() {
	std::vector<std::string> toHideCopy, toShowCopy;
	{
		std::lock_guard g(screenMutex);
		toHideCopy.swap(toHideList);
		toShowCopy.swap(toShowList);
	}

	for (const std::string &name : toHideCopy) {
		hide(name);
	}
	for (const std::string &name : toShowCopy) {
		show(name);
	}
}

void Screen::replace(const std::string &fromName, const std::string &toName) {
	std::lock_guard g(screenMutex);

	if (toName.empty()) {
		auto it = screenMap.find(fromName);
		if (it != screenMap.end()) {
			screenMap.erase(it);
		}
	}else {
		screenMap[fromName] = toName;
	}
}

void Screen::clear() {
	std::lock_guard g(screenMutex);

	declared.clear();
	screenMap.clear();

	toShowList.clear();
	toHideList.clear();
}


static void makeScreenVars(const std::string &name, PyObject *args, PyObject *kwargs) {
	Node *node = Screen::getDeclared(name);
	auto &vars = node->vars;
	size_t countVars = vars.size();

	std::string code =
	        "if 'screen_vars' not in globals():\n"
	        "    screen_vars = {}\n"
	        "_SL_created_vars = False\n"
	        "_SL_got_args = " + std::string((args && kwargs) ? "True" : "False") + "\n"
	        "if ('" + name + "' not in screen_vars) or _SL_got_args:\n"
	        "    screen_vars['" + name + "'] = Object()\n"
	        "    _SL_created_vars = True\n"
	        "screen = screen_vars['" + name + "']";
	PyUtils::exec("CPP_EMBED: screen.cpp", __LINE__, code);

	PyObject *created_vars = PyDict_GetItemString(PyUtils::global, "_SL_created_vars");
	if (created_vars == Py_False && (!args || !kwargs)) return; //create screen on loading

	PyObject *screenVars = PyDict_GetItemString(PyUtils::global, "screen");

	if (!args) args = Py_None;
	if (!kwargs) kwargs = Py_None;

	if (args != Py_None && !PyTuple_CheckExact(args) && !PyList_CheckExact(args)) {
		Utils::outMsg("Screen::addToShow",
		              "Expected type(args) is tuple or list, got <" + std::string(args->ob_type->tp_name) + ">");
		return;
	}
	size_t argsSize = args == Py_None ? 0 : size_t(Py_SIZE(args));

	if (kwargs != Py_None && !PyDict_CheckExact(kwargs)) {
		Utils::outMsg("Screen::addToShow",
		              "Expected type(kwargs) is dict, got <" + std::string(kwargs->ob_type->tp_name) + ">");
		return;
	}
	size_t kwargsSize = kwargs == Py_None ? 0 : size_t(PyDict_Size(kwargs));

	if (argsSize + kwargsSize > countVars) {
		Utils::outMsg("Screen::addToShow",
		              "Screen <" + name + "> takes only " + std::to_string(countVars) + " args\n"
		              "Got " + std::to_string(argsSize) + " args and " + std::to_string(kwargsSize) + " kwargs");
		return;
	}

	std::vector<std::string> argsWasSet;
	argsWasSet.reserve(countVars);

	for (size_t i = 0; i < argsSize; ++i) {
		PyObject *elem = PySequence_Fast_GET_ITEM(args, i);

		const std::string &varName = vars[i].first;
		argsWasSet.push_back(varName);
		PyObject_SetAttrString(screenVars, varName.c_str(), elem);
	}

	Py_ssize_t i = 0;
	PyObject *key, *value;
	while (PyDict_Next(kwargs, &i, &key, &value)) {
		std::string varName = PyUtils::objToStr(key);
		if (std::find(argsWasSet.cbegin(), argsWasSet.cend(), varName) == argsWasSet.cend()) {
			argsWasSet.push_back(varName);
		}else {
			Utils::outMsg("Screen::addToShow",
			              "Keyworg <" + varName + "> argument repeated for screen <" + name + ">");
		}

		PyObject_SetAttrString(screenVars, varName.c_str(), value);
	}

	for (size_t i = 0; i < countVars; ++i) {
		const auto &[varName, varDefaultCode] = vars[i];
		if (std::find(argsWasSet.cbegin(), argsWasSet.cend(), varName) != argsWasSet.cend()) continue;

		if (varDefaultCode.empty()) {
			Utils::outMsg("Screen::addToShow",
			              "Arg <" + varName + "> was not set for screen <" + name + ">");
		}else {
			PyUtils::exec("CPP_EMBED: screen.cpp", __LINE__,
			              "screen." + varName + " = " + varDefaultCode);
		}
	}
}

void Screen::addToShow(std::string name, PyObject *args, PyObject *kwargs) {
	//python thread need for func makeScreenVars
	//this order of locks need for no deadlocks

	PyUtils::callInPythonThread([&]() {
		std::lock_guard g(screenMutex);

		auto it = screenMap.find(name);
		if (it != screenMap.end()) {
			name = screenMap[name];
		}

		Node *node = Screen::getDeclared(name);
		if (!node) {
			Utils::outMsg("Screen::addToShow", "No screen <" + name + ">");
			return;
		}

		makeScreenVars(name, args, kwargs);

		for (size_t i = 0; i < toHideList.size(); ++i) {
			if (toHideList[i] == name) {
				toHideList.erase(toHideList.cbegin() + long(i));
				return;
			}
		}

		toShowList.push_back(name);
	});
}
void Screen::addToHide(std::string name) {
	std::lock_guard g(screenMutex);

	auto it = screenMap.find(name);
	if (it != screenMap.end()) {
		name = screenMap[name];
	}

	for (size_t i = 0; i < toShowList.size(); ++i) {
		if (toShowList[i] == name) {
			toShowList.erase(toShowList.cbegin() + long(i));
			return;
		}
	}

	toHideList.push_back(name);
}
bool Screen::hasScreen(std::string name) {
	std::lock_guard g(screenMutex);

	auto it = screenMap.find(name);
	if (it != screenMap.end()) {
		name = screenMap[name];
	}

	for (const std::string &screenName : toShowList) {
		if (screenName == name) return true;
	}

	return getMain(name);
}

void Screen::logScreenCode(std::string name) {
	std::lock_guard g(screenMutex);

	auto it = screenMap.find(name);
	if (it != screenMap.end()) {
		name = screenMap[name];
	}

	Node* node = getDeclared(name);
	if (!node) {
		Utils::outMsg("Screen::logScreenCode", "Screen <" + name + "> is not defined");
		return;
	}

	std::string code = ScreenCodeGenerator::get(node);
	Logger::log("\n\n\nCode of screen <" + name + ">:\n\n" + code + "\n\n\n");
}



static bool _hasModal = false;
bool Screen::hasModal() {
	return _hasModal;
}

void Screen::updateScreens() {
	if (!Stage::screens) return;

	_hasModal = false;
	for (const DisplayObject *d : Stage::screens->children) {
		const Screen *s = static_cast<const Screen*>(d);
		if (s->modal) {
			_hasModal = true;
			break;
		}
	}

	auto zOrderCmp = [](DisplayObject *a, DisplayObject *b) -> bool {
		const Screen *sA = static_cast<Screen*>(a);
		const Screen *sB = static_cast<Screen*>(b);
		return sA->zorder < sB->zorder;
	};
	std::stable_sort(Stage::screens->children.begin(), Stage::screens->children.end(), zOrderCmp);

	uint32_t index = 0;
	for (DisplayObject *screen : Stage::screens->children) {
		screen->index = index++;
	}
}


struct StackElem {
	Container *obj;
	size_t curChildNum;
	size_t curChildIndex;
};

static std::vector<StackElem> screenStack;
static Screen* calcedScreen;

void Screen::checkScreenEvents() {
	if (screenStack.empty()) {
		std::string rootVar = "_SL_" + calcedScreen->name;
		calcedScreen->props = PyDict_GetItemString(PyUtils::global, rootVar.c_str());
		if (!calcedScreen->props) {
			Utils::outMsg("Screen::checkEvents", rootVar + " not found");
			return;
		}
		if (!PyList_CheckExact(calcedScreen->props) && !PyTuple_CheckExact(calcedScreen->props)) {
			Utils::outMsg("Screen::checkEvents", "_SL_stack is not list or tuple");
			return;
		}

		screenStack.push_back({calcedScreen, 0, 0});
	}

	StackElem elem = screenStack.back();
	Container *obj = elem.obj;
	size_t num = elem.curChildNum;
	size_t index = elem.curChildIndex;

	while (true) {
		const size_t countCalcs = size_t(Py_SIZE(obj->props));
		const size_t countChildren = countCalcs - obj->node->countPropsToCalc;
		while (countChildren > obj->screenChildren.size()) {
			obj->addChildrenFromNode();
		}

		if (num == obj->screenChildren.size() || index == countCalcs) {
			obj->updateProps();

			screenStack.pop_back();
			if (screenStack.empty()) return;

			if (obj->node->withScreenEvent) {
				obj->checkEvents();
				return;
			}

			elem = screenStack.back();
			obj = elem.obj;
			num = elem.curChildNum;
			index = elem.curChildIndex;
			continue;
		}


		Child *child = obj->screenChildren[num];
		++num;
		++screenStack.back().curChildNum;

		if (child->node->screenNum == uint32_t(-1)) {
			child->enable = true;
			continue;
		}

		if (obj->node->countPropsToCalc) {
			child->props = PySequence_Fast_GET_ITEM(obj->props, child->node->screenNum);
		}else {
			child->props = PySequence_Fast_GET_ITEM(obj->props, index);
			++index;
			++screenStack.back().curChildIndex;
		}

		if (!child->props || child->props == Py_None) continue;
		if (!PyList_CheckExact(child->props) && !PyTuple_CheckExact(child->props)) {
			std::string type = child->props->ob_type->tp_name;
			Utils::outMsg("Screen::checkEvents", "Expected list or tuple, got " + type);
			child->props = nullptr;
			continue;
		}

		if (!child->node->isScreenEnd) {
			obj = static_cast<Container*>(child);
			num = 0;
			index = 0;
			screenStack.push_back({obj, num, index});
			continue;
		}

		child->updateProps();

		if (child->node->withScreenEvent) {
			child->checkEvents();
			return;
		}
	}
}


Screen::Screen(Node *node, Screen *screen):
    Container(node, this, screen ? screen : this),
    name(node->params)
{
	if (this->screen != this) return;//not main screen, command <use>

	screenCode = ScreenCodeGenerator::get(node);

	PyUtils::callInPythonThread([&]() {
		co = PyUtils::getCompileObject(screenCode, "_SL_FILE_" + name, 1);
		if (!co) {
			PyUtils::errorProcessing(screenCode);
		}
	});
}

//called from python thread
void Screen::calcProps() {
	if (!co) return;

	screenStack.clear();
	calcedScreen = this;

	if (!PyEval_EvalCode(co, PyUtils::global, nullptr)) {
		PyUtils::errorProcessing(screenCode);
	}
}
