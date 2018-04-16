#include "screen.h"

#include "gv.h"
#include "media/py_utils.h"
#include "utils/utils.h"

std::vector<Node*> Screen::declared;

std::vector<String> Screen::toShowList;
std::vector<String> Screen::toHideList;

Node* Screen::getDeclared(const String &name) {
	for (Node *node : declared) {
		if (node->name == name) {
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
	static std::mutex m;
	std::lock_guard<std::mutex> g(m);

	for (const String &name : toHideList) {
		hide(name);
	}
	toHideList.clear();

	for (const String &name : toShowList) {
		show(name);
	}
	toShowList.clear();
}


void Screen::show(const String &name) {
	Screen *scr = getMain(name);
	if (scr) {
		scr->parent->addChild(scr);//Наверх в списке потомков своего родителя
		return;
	}

	Node *node = getDeclared(name);
	if (!node) {
		Utils::outMsg("Screen::show", "Скрин с именем <" + name + "> не существует");
		return;
	}

	scr = new Screen(node, nullptr);
	GV::screens->addChild(scr);

	scr->calculateProps();
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


bool Screen::_hasModal = false;
void Screen::updateScreens() {
	if (!GV::screens) return;

	for (DisplayObject *d : GV::screens->children) {
		Screen *s = dynamic_cast<Screen*>(d);
		if (s) {
			s->updateScreenProps();
		}
	}

	_hasModal = false;
	for (DisplayObject *d : GV::screens->children) {
		Screen *s = dynamic_cast<Screen*>(d);
		if (s && s->_isModal) {
			_hasModal = true;
			break;
		}
	}

	auto zOrderCmp = [](DisplayObject *a, DisplayObject *b) -> bool {
		Screen *sA = dynamic_cast<Screen*>(a);
		Screen *sB = dynamic_cast<Screen*>(b);
		double zA = sA ? sA->zOrder() : 0;
		double zB = sB ? sB->zOrder() : 0;
		return zA < zB;
	};
	std::sort(GV::screens->children.begin(), GV::screens->children.end(), zOrderCmp);
}
void Screen::updateScreenProps() {
	xSize = GV::width;
	ySize = GV::height;
	xSizeIsDouble = ySizeIsDouble = false;
	needUpdateChildren = false;
	calculateProps();
	needUpdateChildren = true;

	if (propWasChanged[ScreenProp::MODAL]) {
		propWasChanged[ScreenProp::MODAL] = false;

		std::lock_guard<std::mutex> g(PyUtils::pyExecMutex);
		_isModal = py::extract<bool>(propValues[ScreenProp::MODAL]);
	}
	if (propWasChanged[ScreenProp::ZORDER]) {
		propWasChanged[ScreenProp::ZORDER] = false;

		py::object &zOrderObj = propValues[ScreenProp::ZORDER];
		bool isInt = PyUtils::isInt(zOrderObj);
		bool isFloat = !isInt && PyUtils::isFloat(zOrderObj);

		if (isInt || isFloat) {
			_zOrder = PyUtils::getDouble(zOrderObj, isFloat);
		}else {
			_zOrder = 0;
			Utils::outMsg("Screen::updateScreenProps",
						  "zorder is not a number (" + PyUtils::getStr(zOrderObj) + ")\n" + node->getPlace());
		}
	}
}



Screen::Screen(Node *node, Screen *screen):
	ScreenContainer(node, this, screen ? screen : this),
	name(node->name)
{
	setProp(ScreenProp::MODAL, node->getPropCode("modal"));
	setProp(ScreenProp::ZORDER, node->getPropCode("zorder"));

	preparationToUpdateCalcProps();
}
