#include "screen.h"

#include <algorithm>
#include <map>

#include "gv.h"
#include "logger.h"
#include "media/py_utils.h"
#include "parser/screen_node_utils.h"
#include "utils/utils.h"


bool Screen::destroyedScreenIsModal = false;


static std::map<std::string, Node*> declared;
static std::mutex screenMutex;

void Screen::declare(Node *screenNode) {
	declared[screenNode->params] = screenNode;
}
Node* Screen::getDeclared(const std::string &name) {
	auto it = declared.find(name);
	return it != declared.end() ? it->second : nullptr;
}
Screen* Screen::getMain(const std::string &name) {
	if (GV::screens) {
		for (DisplayObject *d : GV::screens->children) {
			Screen *scr = static_cast<Screen*>(d);
			if (scr->name == name) {
				return scr;
			}
		}
	}
	return nullptr;
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

	GV::screens->addChildAt(scr, GV::screens->children.size());
	PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "globals().has_key('signals') and signals.send('show_screen', '" + name + "')");
}
static void hide(const std::string &name) {
	if (!GV::screens) return;

	for (DisplayObject *d : GV::screens->children) {
		Screen *scr = static_cast<Screen*>(d);
		if (scr->getName() == name) {
			delete scr;
			PyUtils::exec("CPP_EMBED: main.cpp", __LINE__, "globals().has_key('signals') and signals.send('hide_screen', '" + name + "')");
			return;
		}
	}

	Utils::outMsg("Screen::hide", "Screen <" + name + "> is not shown");
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

void Screen::clear() {
	declared.clear();

	std::lock_guard g(screenMutex);
	toShowList.clear();
	toHideList.clear();
}


void Screen::addToShow(const std::string &name) {
	std::lock_guard g(screenMutex);

	for (size_t i = 0; i < toHideList.size(); ++i) {
		if (toHideList[i] == name) {
			toHideList.erase(toHideList.cbegin() + long(i));
			return;
		}
	}

	toShowList.push_back(name);
}
void Screen::addToHide(const std::string &name) {
	std::lock_guard g(screenMutex);

	for (size_t i = 0; i < toShowList.size(); ++i) {
		if (toShowList[i] == name) {
			toShowList.erase(toShowList.cbegin() + long(i));
			return;
		}
	}

	toHideList.push_back(name);
}
bool Screen::hasScreen(const std::string &name) {
	std::lock_guard g(screenMutex);

	for (const std::string &screenName : toShowList) {
		if (screenName == name) return true;
	}

	return getMain(name);
}

void Screen::logScreenCode(const std::string &name) {
	Node* node = getDeclared(name);
	std::string code = ScreenNodeUtils::getScreenCode(node);
	Logger::log("\n\n\nCode of screen <" + name + ">:\n\n" + code + "\n\n\n");
}



static bool _hasModal = false;
bool Screen::hasModal() {
	return _hasModal;
}

void Screen::updateScreens() {
	if (!GV::screens) return;

	_hasModal = false;
	for (DisplayObject *d : GV::screens->children) {
		const Screen *s = static_cast<Screen*>(d);
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
	std::sort(GV::screens->children.begin(), GV::screens->children.end(), zOrderCmp);
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

			elem = screenStack.back();
			obj = elem.obj;
			num = elem.curChildNum;
			index = elem.curChildIndex;
			continue;
		}


		Child *child = obj->screenChildren[num];
		++num;
		++screenStack.back().curChildNum;

		if (child->node->screenNum == size_t(-1)) {
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
			Utils::outMsg("Screen::checkEvents",
			              std::string("Expected list or tuple, got ") + child->props->ob_type->tp_name);
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

	screenCode = ScreenNodeUtils::getScreenCode(node);
	co = PyUtils::getCompileObject(screenCode, "_SL_FILE_" + name, 1);
	if (!co) {
		PyUtils::errorProcessing(screenCode);
	}
}

void Screen::calcProps() {
	if (!co) return;

	screenStack.clear();
	calcedScreen = this;

	std::lock_guard g(PyUtils::pyExecMutex);

	if (!PyEval_EvalCode(static_cast<PyCodeObject*>(co), PyUtils::global, nullptr)) {
		PyUtils::errorProcessing(screenCode);
	}
}
