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
			Screen *scr = dynamic_cast<Screen*>(d);
			if (scr && scr->name == name) {
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
	if (scr) {
		GV::screens->addChild(scr);//Наверх в списке потомков своего родителя
		return;
	}

	Node *node = getDeclared(name);
	if (!node) {
		Utils::outMsg("Screen::show", "Скрин с именем <" + name + "> не существует");
		return;
	}

	scr = new Screen(node, nullptr);
	scr->updateProps();
	GV::screens->addChild(scr);
}
void Screen::hide(const String &name) {
	if (GV::screens) {
		for (DisplayObject *d : GV::screens->children) {
			Screen *scr = dynamic_cast<Screen*>(d);
			if (scr && scr->name == name) {
				delete scr;
				return;
			}
		}
	}
	Utils::outMsg("Screen::hide", "Скрин с именем <" + name + "> не отображается, поэтому его нельзя скрыть");
}


void Screen::addToShow(const std::string &name) {
	std::lock_guard<std::mutex> g(screenMutex);

	for (size_t i = 0; i < toHideList.size(); ++i) {
		if (toHideList[i] == name) {
			toHideList.erase(toHideList.cbegin() + i);
			return;
		}
	}

	toShowList.push_back(name);
}
void Screen::addToHide(const std::string &name) {
	std::lock_guard<std::mutex> g(screenMutex);

	for (size_t i = 0; i < toShowList.size(); ++i) {
		if (toShowList[i] == name) {
			toShowList.erase(toShowList.cbegin() + i);
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

	for (DisplayObject *d : GV::screens->children) {
		Screen *s = dynamic_cast<Screen*>(d);
		if (s) {
		//	s->updateScreenProps();
		}
	}

	_hasModal = false;
	for (DisplayObject *d : GV::screens->children) {
		Screen *s = dynamic_cast<Screen*>(d);
		if (s && s->modal) {
			_hasModal = true;
			break;
		}
	}

	auto zOrderCmp = [](DisplayObject *a, DisplayObject *b) -> bool {
		Screen *sA = dynamic_cast<Screen*>(a);
		Screen *sB = dynamic_cast<Screen*>(b);
		double zA = sA ? sA->zorder : 0;
		double zB = sB ? sB->zorder : 0;
		return zA < zB;
	};
	std::sort(GV::screens->children.begin(), GV::screens->children.end(), zOrderCmp);
}

void Screen::checkEvents() {

}


Screen::Screen(Node *node, Screen *screen):
	Container(node, this, screen ? screen : this),
	name(node->params)
{
	ScreenNodeUtils::init(node);

	screenCode = ScreenNodeUtils::getScreenCode(node);
	co = PyUtils::getCompileObject(screenCode, getFileName(), 0);
	if (!co) {
		PyUtils::errorProcessing(screenCode);
	}
}

void Screen::updateProps() {
	if (!co) return;

	std::lock_guard g(PyUtils::pyExecMutex);

	if (!PyEval_EvalCode(co, PyUtils::global, nullptr)) {
		PyUtils::errorProcessing(screenCode);
		return;
	}

	if (props) {
		Py_DECREF(props);
	}
	props = PyDict_GetItemString(PyUtils::global, "SL_last");
	if (!props) {
		Utils::outMsg("Screen::updateProps", "SL_last not found");
		return;
	}
	Py_INCREF(props);

	Container::updateProps();
}
