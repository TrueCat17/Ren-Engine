#include "screen.h"

#include <algorithm>

#include "gv.h"
#include "media/py_utils.h"
#include "parser/screen_node_utils.h"
#include "utils/utils.h"

std::vector<Node*> Screen::declared;

std::mutex Screen::screenMutex;

std::vector<String> Screen::toShowList;
std::vector<String> Screen::toHideList;

Node* Screen::getDeclared(const String &name) {
	for (Node *node : declared) {
		if (node->params == name) {
			return node;
		}
	}
	return nullptr;
}
Screen* Screen::getMain(const String &name) {
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


void Screen::updateLists() {
	std::vector<String> toHideCopy, toShowCopy;
	{
		std::lock_guard<std::mutex> g(screenMutex);
		toHideCopy.swap(toHideList);
		toShowCopy.swap(toShowList);
	}

	for (const String &name : toHideCopy) {
		hide(name);
	}
	for (const String &name : toShowCopy) {
		show(name);
	}
}


void Screen::show(const String &name) {
	Screen *scr = getMain(name);
	if (!scr) {
		Node *node = getDeclared(name);
		if (!node) {
			Utils::outMsg("Screen::show", "Скрин с именем <" + name + "> не существует");
			return;
		}

		ScreenNodeUtils::init(node);
		scr = new Screen(node, nullptr);
	}

	GV::screens->addChildAt(scr, GV::screens->children.size());
}
void Screen::hide(const String &name) {
	if (!GV::screens) return;

	for (DisplayObject *d : GV::screens->children) {
		Screen *scr = static_cast<Screen*>(d);
		if (scr->name == name) {
			delete scr;
			return;
		}
	}

	Utils::outMsg("Screen::hide", "Скрин с именем <" + name + "> не отображается, поэтому его нельзя скрыть");
}


void Screen::addToShow(const std::string &name) {
	std::lock_guard<std::mutex> g(screenMutex);

	for (size_t i = 0; i < toHideList.size(); ++i) {
		if (toHideList[i] == name) {
			toHideList.erase(toHideList.cbegin() + int(i));
			return;
		}
	}

	toShowList.push_back(name);
}
void Screen::addToHide(const std::string &name) {
	std::lock_guard<std::mutex> g(screenMutex);

	for (size_t i = 0; i < toShowList.size(); ++i) {
		if (toShowList[i] == name) {
			toShowList.erase(toShowList.cbegin() + int(i));
			return;
		}
	}

	toHideList.push_back(name);
}
bool Screen::hasScreen(const std::string &name) {
	std::lock_guard<std::mutex> g(screenMutex);

	for (const String &screenName : toShowList) {
		if (screenName == name) return true;
	}

	return getMain(name);
}


bool Screen::_hasModal = false;
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
		String rootVar = "_SL_" + calcedScreen->name;
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
		if (obj->getNumLine() == 142) {
			int q = 1;
			q += 2;
		}

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
						  String("Expected list or tuple, got ") + child->props->ob_type->tp_name);
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
	co = PyUtils::getCompileObject(screenCode, getFileName(), 0);
	if (!co) {
		PyUtils::errorProcessing(screenCode);
	}
}

void Screen::calcProps() {
	if (!co) return;

	screenStack.clear();
	calcedScreen = this;

	std::lock_guard g(PyUtils::pyExecMutex);

	if (!PyEval_EvalCode(co, PyUtils::global, nullptr)) {
		PyUtils::errorProcessing(screenCode);
	}
}
