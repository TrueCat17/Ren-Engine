#include "btn_rect.h"

#include <algorithm>

#include "gui/screen/screen.h"
#include "gui/screen/style.h"

#include "media/audio_manager.h"
#include "media/py_utils.h"

#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/stage.h"
#include "utils/utils.h"


static std::vector<BtnRect*> btnRects;
static Uint32 btnRectId = Uint32(-1);


struct FPoint {
	float x, y;
};

using FRect = std::array<FPoint, 4>;

struct BtnRectAndFRect {
	BtnRect *btnRect;
	FRect fRect;
};

static bool selectMode = false;
static Uint32 selectBtnId = Uint32(-1);
static FRect selectBtnFRect;

static BtnRect* getSelectedBtnRect() {
	for (BtnRect *btnRect : btnRects) {
		if (btnRect->id == selectBtnId) {
			return btnRect;
		}
	}
	return nullptr;
}

static FRect getFRect(const Child *obj) {
	FRect res;

	float sinA = Math::getSin(int(obj->getGlobalRotate()));
	float cosA = Math::getCos(int(obj->getGlobalRotate()));

	FPoint points[4] = {
	    { 0, 0 },
	    { obj->getWidth(), 0 },
	    { obj->getWidth(), obj->getHeight() },
	    { 0, obj->getHeight() }
	};
	for (size_t i = 0; i < 4; ++i) {
		float x = points[i].x - obj->calcedXanchor;
		float y = points[i].y - obj->calcedYanchor;

		res[i].x = obj->getGlobalX() + (x * cosA - y * sinA) + obj->calcedXanchor;
		res[i].y = obj->getGlobalY() + (x * sinA + y * cosA) + obj->calcedYanchor;
	}

	return res;
}


static bool objInTop(const DisplayObject *obj, int mouseX, int mouseY) {
	while (obj->parent && obj->parent->enable) {
		for (uint32_t i = uint32_t(obj->index) + 1; i < obj->parent->children.size(); ++i) {
			const DisplayObject *child = obj->parent->children[i];
			if (!child->enable) continue;

			float x = float(mouseX) - child->getGlobalX() - child->calcedXanchor;
			float y = float(mouseY) - child->getGlobalY() - child->calcedYanchor;

			float sinA = Math::getSin(int(-child->getGlobalRotate()));
			float cosA = Math::getCos(int(-child->getGlobalRotate()));

			int rotX = int(x * cosA - y * sinA + child->calcedXanchor);
			int rotY = int(x * sinA + y * cosA + child->calcedYanchor);

			if (!child->transparentForMouse(rotX, rotY)) {
				return false;
			}
		}
		obj = obj->parent;
	}
	return true;
}


BtnRect::BtnRect(Child *owner):
    owner(owner),
    id(++btnRectId),
    buttonMode(true),
    selectedByKeyboard(false),
    prevHovered(false),
    mouseOvered(false),
    mouseLeftDown(false),
    mouseRightDown(false)
{
	btnRects.push_back(this);
}

BtnRect::~BtnRect() {
	for (size_t i = 0; i < btnRects.size(); ++i) {
		BtnRect *btnRect = btnRects[i];
		if (btnRect == this) {
			btnRects.erase(btnRects.begin() + long(i));
			return;
		}
	}
}

void BtnRect::checkMouseCursor() {
	Mouse::setLocal(-1, -1);

	BtnRect *prevHovered = nullptr;
	for (BtnRect *btnRect : btnRects) {
		btnRect->mouseLeftDown = false;
		btnRect->mouseRightDown = false;

		if (btnRect->mouseOvered) {
			btnRect->mouseOvered = false;
			prevHovered = btnRect;
		}
	}

	bool haveMouseActionInLastFrame = Mouse::haveActionInLastFrame();

	int mouseX = Mouse::getX();
	int mouseY = Mouse::getY();

	for (BtnRect *btnRect : btnRects) {
		if (prevHovered != btnRect && !haveMouseActionInLastFrame && selectMode) continue;
		if (btnRect->needIgnore()) continue;

		Child *owner = btnRect->owner;

		float x = float(mouseX) - owner->getGlobalX() - owner->calcedXanchor;
		float y = float(mouseY) - owner->getGlobalY() - owner->calcedYanchor;

		float sinA = Math::getSin(int(-owner->getGlobalRotate()));
		float cosA = Math::getCos(int(-owner->getGlobalRotate()));

		int rotX = int(x * cosA - y * sinA + owner->calcedXanchor);
		int rotY = int(x * sinA + y * cosA + owner->calcedYanchor);

		if (rotX >= 0 && rotX < int(owner->getWidth()) &&
		    rotY >= 0 && rotY < int(owner->getHeight())
		) {
			if (owner->transparentForMouse(rotX, rotY)) continue;
			if (!objInTop(owner, mouseX, mouseY)) continue;

			Mouse::setLocal(rotX, rotY);

			btnRect->mouseOvered = true;
			if (btnRect->buttonMode) {
				BtnRect *prevSelected = getSelectedBtnRect();
				if (prevSelected != btnRect) {
					if (prevSelected) {
						prevSelected->selectedByKeyboard = false;
					}

					selectMode = false;
					selectBtnId = btnRect->id;
					selectBtnFRect = getFRect(btnRect->owner);
				}

				Mouse::setButtonMode();
			}else {
				Mouse::setUsualMode();
			}
			return;
		}
	}

	Mouse::setUsualMode();
}

bool BtnRect::checkMouseClick(bool left, bool withKeyboard) {
	if (withKeyboard && selectMode) {
		BtnRect *selectedBtnRect = getSelectedBtnRect();
		if (selectedBtnRect && selectedBtnRect->selectedByKeyboard) {
			selectedBtnRect->mouseLeftDown = true;
			return true;
		}
	}

	if (left && !withKeyboard) {
		Mouse::setMouseDown(true);
	}

	int mouseX = Mouse::getX();
	int mouseY = Mouse::getY();

	for (BtnRect *btnRect : btnRects) {
		if (btnRect->needIgnore()) continue;
		if (withKeyboard && !btnRect->buttonMode) continue;

		Child *owner = btnRect->owner;

		float x = float(mouseX) - owner->getGlobalX() - owner->calcedXanchor;
		float y = float(mouseY) - owner->getGlobalY() - owner->calcedYanchor;

		float sinA = Math::getSin(int(-owner->getGlobalRotate()));
		float cosA = Math::getCos(int(-owner->getGlobalRotate()));

		int rotX = int(x * cosA - y * sinA + owner->calcedXanchor);
		int rotY = int(x * sinA + y * cosA + owner->calcedYanchor);

		if (rotX >= 0 && rotX < int(owner->getWidth()) &&
		    rotY >= 0 && rotY < int(owner->getHeight())
		) {
			if (owner->transparentForMouse(rotX, rotY)) continue;
			if (!objInTop(owner, mouseX, mouseY)) continue;

			if (left) {
				btnRect->mouseLeftDown = true;
			}else {
				btnRect->mouseRightDown = true;
			}
			return true;
		}
	}
	return false;
}


void BtnRect::onHovered() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node *hoverSound = node->getProp("hover_sound");
	if (hoverSound) {
		std::string path = PyUtils::exec(hoverSound->getFileName(), hoverSound->getNumLine(),
		                                 hoverSound->params, true);
		AudioManager::play("button_hover", path, 0, 0, 1.0,
		                   hoverSound->getFileName(), hoverSound->getNumLine());
	}else {
		PyObject *hoverSoundObj = StyleManager::getProp(style, "hover_sound");

		if (PyUnicode_CheckExact(hoverSoundObj)) {
			std::string path = PyUtils::objToStr(hoverSoundObj);
			AudioManager::play("button_hover", path, 0, 0, 1.0, node->getFileName(), node->getNumLine());
		}else if (hoverSoundObj != Py_None) {
			Utils::outError("BtnRect::onHovered",
			                "In style.%.hover_sound expected type str, got %",
			                style->name, hoverSoundObj->ob_type->tp_name);
		}
	}

	const Node *hovered = node->getProp("hovered");
	if (hovered) {
		PyUtils::exec(hovered->getFileName(), hovered->getNumLine(),
		              "exec_funcs(" + hovered->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "hovered");
	}
}

void BtnRect::onUnhovered() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node *unhovered = node->getProp("unhovered");
	if (unhovered) {
		PyUtils::exec(unhovered->getFileName(), unhovered->getNumLine(),
		              "exec_funcs(" + unhovered->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "unhovered");
	}
}

void BtnRect::onLeftClick() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node *activateSound = node->getProp("activate_sound");
	if (activateSound) {
		std::string path = PyUtils::exec(activateSound->getFileName(), activateSound->getNumLine(),
		                                 activateSound->params, true);
		AudioManager::play("button_click", path, 0, 0, 1.0,
		                   activateSound->getFileName(), activateSound->getNumLine());
	}else {
		PyObject *activateSoundObj = StyleManager::getProp(style, "activate_sound");

		if (PyUnicode_CheckExact(activateSoundObj)) {
			std::string path = PyUtils::objToStr(activateSoundObj);
			AudioManager::play("button_click ", path, 0, 0, 1.0,
			                   node->getFileName(), node->getNumLine());
		}else if (activateSoundObj != Py_None) {
			Utils::outError("BtnRect::onLeftClick",
			                "In style.%.activate_sound expected type str, got %",
			                style->name, activateSoundObj->ob_type->tp_name);
		}
	}

	const Node *action = node->getProp("action");
	if (action) {
		PyUtils::exec(action->getFileName(), action->getNumLine(),
		              "exec_funcs(" + action->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "action");
	}
}

void BtnRect::onRightClick() const {
	const Node *node = owner->node;
	const Style *style = owner->style;

	const Node* alternate = node->getProp("alternate");
	if (alternate) {
		PyUtils::exec(alternate->getFileName(), alternate->getNumLine(),
		              "exec_funcs(" + alternate->params + ")");
	}else {
		StyleManager::execAction(node->getFileName(), node->getNumLine(), style, "alternate");
	}
}


bool BtnRect::needIgnore() const {
	return !owner->enable     || owner->globalSkipMouse       ||
	        owner->alpha <= 0 || owner->getGlobalAlpha() <= 0 ||
	       !owner->isModal();
	//alpha == 0 and globalAlpha != 0 is ok, because:
	// 1. update_alpha()
	// 2. checkEvents() - if [needIgnore()]
	// 3. updateGlobal()
}

void BtnRect::checkEvents() {
	if (needIgnore()) {
		mouseOvered = false;
		mouseLeftDown = false;
		mouseRightDown = false;
	}

	bool hovered = isHovered();
	if (hovered) {
		if (!prevHovered) {
			onHovered();
		}
	}else {
		if (prevHovered) {
			onUnhovered();
		}
	}
	prevHovered = hovered;

	if (mouseLeftDown) {
		onLeftClick();
	}
	if (mouseRightDown) {
		onRightClick();
	}
}



bool BtnRect::getSelectMode() {
	return selectMode;
}
void BtnRect::disableSelectMode(bool clearLastBtn) {
	selectMode = false;

	BtnRect *btnRect = getSelectedBtnRect();
	if (btnRect) {
		btnRect->selectedByKeyboard = false;
	}

	if (clearLastBtn) {
		selectBtnId = Uint32(-1);
	}
}



std::array<SDL_Point, 4> BtnRect::getPointsOfSelectedRect() {
	static std::array<SDL_Point, 4> res;

	BtnRect *rect = selectMode ? getSelectedBtnRect() : nullptr;
	if (rect) {
		FRect selectedBtnFRect = getFRect(rect->owner);
		for (size_t i = 0; i < 4; ++i) {
			res[i].x = int(std::round(selectedBtnFRect[i].x));
			res[i].y = int(std::round(selectedBtnFRect[i].y));
		}
	}else {
		for (size_t i = 0; i < 4; ++i) {
			res[i] = { 0, 0 };
		}
	}

	return res;
}


static FPoint getRectCenter(FRect rect) {
	return {
		(rect[0].x + rect[2].x) / 2,
		(rect[0].y + rect[2].y) / 2
	};
}

static bool isRight(float prevPos, FRect rect) {
	return prevPos < getRectCenter(rect).x;
}
static bool isLeft(float prevPos, FRect rect) {
	return prevPos > getRectCenter(rect).x;
}
static bool isDown(float prevPos, FRect rect) {
	return prevPos < getRectCenter(rect).y;
}
static bool isUp(float prevPos, FRect rect) {
	return prevPos > getRectCenter(rect).y;
}


static float kX, kY;
static const float kMax = 10;

static float getDist(const FPoint &a, const FPoint &b, float kX = 1, float kY = 1) {
	float dX = (a.x - b.x) * kX;
	float dY = (a.y - b.y) * kY;
	return std::sqrt(dX * dX + dY * dY);
}

static float getDistBetweenPointAndLine(const FPoint &point, const FPoint &start, const FPoint &end) {
	float dX = end.x - start.x;
	float dY = end.y - start.y;

	FPoint c;
	if (dX < 1) {
		c.x = end.x;
		c.y = point.y;
	}else
	if (dY < 1) {
		c.x = point.x;
		c.y = end.y;
	}else {
		//y = a * x + b
		float a = dY / dX;
		float b = end.y - a * end.x;

		//y = normalA * x + normalB
		float normalA = dX / dY;
		float normalB = point.y - normalA * point.x;

		//a * c.x + b = normalA * c.x + normalB
		//(a - normalA) * c.x = normalB - b
		c.x = (normalB - b) / (a - normalA);
		c.y = a * c.x + b;
	}

	float distBetweenStartAndC   = getDist(start, c);
	float distBetweenEndAndC     = getDist(end,   c);
	float distBetweenStartAndEnd = getDist(start, end);

	bool cBetweenStartAndEnd = std::max(distBetweenStartAndC, distBetweenEndAndC) < distBetweenStartAndEnd;
	if (cBetweenStartAndEnd) {
		return getDist(point, c, kX, kY);
	}else {
		return std::min(getDist(point, start, kX, kY), getDist(point, end, kX, kY));
	}
}

static float getDistBetweenRects(const FRect &r1, const FRect &r2) {
	float res = INFINITY;

	for (int i = 0; i < 2; ++i) {
		const FRect &a = i ? r1 : r2;
		const FRect &b = i ? r2 : r1;

		for (size_t j = 0; j < 4; ++j) {
			const FPoint &point = a[j];

			for (size_t k = 0; k < 4; ++k) {
				const FPoint &start = b[k];
				const FPoint &end = b[(k + 1) % 4];

				res = std::min(res, getDistBetweenPointAndLine(point, start, end));
			}
		}
	}

	return res;
}

//examples:
// each point is to the only left (/right/top/bottom) from the screen -> true
// each point is corner (full-screen-btn) -> true
// rect is full-height right panel (with all points on screen borders) -> false
// rect with align 0.5 and size 1.1 -> false (!)
static bool outsideScreen(const FRect &rect) {
	bool left, right, top, bottom;
	left = right = top = bottom = false;

	float sw = float(Stage::width);
	float sh = float(Stage::height);

	for (const FPoint &p : rect) {
		if (Math::floatsAreEq(p.x, 0))  left   = true;
		if (Math::floatsAreEq(p.x, sw)) right  = true;
		if (Math::floatsAreEq(p.y, 0))  top    = true;
		if (Math::floatsAreEq(p.y, sh)) bottom = true;

		if (p.x > 0 && p.x < sw && p.y > 0 && p.y < sh) {
			return false;
		}
	}

	return left && right && top && bottom;
}

static bool clickable(const Child *obj, const FRect &rect) {
	FPoint start = rect[0];
	FPoint end = rect[2];

	for (float k : { 0.2f, 0.5f, 0.8f }) {
		float x = start.x + (end.x - start.x) * k;
		float y = start.y + (end.y - start.y) * k;

		if (objInTop(obj, int(x), int(y))) {
			return true;
		}
	}

	return false;
}


void BtnRect::processTabKey(bool shift) {
	std::vector<BtnRectAndFRect> goodVariants;
	goodVariants.reserve(btnRects.size());

	for (BtnRect *btnRect : btnRects) {
		if (btnRect->needIgnore()) continue;

		FRect fRect = getFRect(btnRect->owner);
		if (outsideScreen(fRect)) continue;
		if (!clickable(btnRect->owner, fRect)) continue;

		goodVariants.push_back({ btnRect, fRect });
	}
	if (goodVariants.empty()) return;

	size_t index = 0;
	for (size_t i = 0; i < goodVariants.size(); ++i) {
		if (goodVariants[i].btnRect->id == selectBtnId) {
			index = i;
			break;
		}
	}

	if (shift) {
		--index;
		if (index == size_t(-1)) {
			index = goodVariants.size() - 1;
		}
	}else {
		++index;
		if (index == goodVariants.size()) {
			index = 0;
		}
	}

	BtnRect *prevBtnRect = getSelectedBtnRect();
	if (prevBtnRect) {
		prevBtnRect->selectedByKeyboard = false;
	}

	BtnRect *btnRect = goodVariants[index].btnRect;

	selectBtnId = btnRect->id;
	selectBtnFRect = goodVariants[index].fRect;

	btnRect->selectedByKeyboard = true;
}

void BtnRect::processKey(SDL_Keycode key, bool shift) {
	for (DisplayObject *d : Stage::screens->children) {
		Screen *screen = static_cast<Screen*>(d);
		if (screen->allowArrows && screen->isModal()) return;
	}

	if (key != SDLK_RIGHT && key != SDLK_LEFT && key != SDLK_DOWN && key != SDLK_UP) {
		if (key == SDLK_TAB) {
			if (selectMode) {
				processTabKey(shift);
			}
		}else {
			disableSelectMode(true);
		}
		return;
	}


	using Condition = bool(*)(float, FRect);
	Condition condition;

	kX = kY = 1;

	if (key == SDLK_RIGHT) {
		condition = isRight;
		kY = kMax;
	}else
	if (key == SDLK_LEFT) {
		condition = isLeft;
		kY = kMax;
	}else
	if (key == SDLK_DOWN) {
		condition = isDown;
		kX = kMax;
	}else {
		condition = isUp;
		kX = kMax;
	}


	FRect prevFRect;
	BtnRect *selectedBtnRect = nullptr;
	if (selectBtnId != Uint32(-1)) {
		selectedBtnRect = getSelectedBtnRect();
		if (selectedBtnRect && selectedBtnRect->needIgnore()) {
			selectedBtnRect = nullptr;
		}

		if (selectedBtnRect) {//still not removed?

			bool restoreLastBtnSelection = !selectMode && !selectedBtnRect->isHovered();
			if (restoreLastBtnSelection) {
				selectMode = true;
				selectBtnId = selectedBtnRect->id;
				selectBtnFRect = getFRect(selectedBtnRect->owner);
				selectedBtnRect->selectedByKeyboard = true;
				return;
			}

			prevFRect = getFRect(selectedBtnRect->owner);//take cur coords
		}else {
			prevFRect = selectBtnFRect;                  //take last coords
		}
	}else {
		float sw = float(Stage::width);
		float sh = float(Stage::height);

		SDL_FRect rect;
		if (key == SDLK_RIGHT) {
			rect = { -100, 0, 1, sh };
		}else
		if (key == SDLK_LEFT) {
			rect = { sw + 100, 0, 1, sh };
		}else
		if (key == SDLK_DOWN) {
			rect = { 0, -100, sw, 1 };
		}else {
			rect = { 0, sh + 100, sw, 1 };
		}

		prevFRect[0] = { rect.x, rect.y };
		prevFRect[1] = { rect.x + rect.w, rect.y };
		prevFRect[2] = { rect.x + rect.w, rect.y + rect.h };
		prevFRect[3] = { rect.x, rect.y + rect.h };
	}

	FPoint centerPrev = getRectCenter(prevFRect);
	float prevPos = (key == SDLK_RIGHT || key == SDLK_LEFT) ? centerPrev.x : centerPrev.y;

	std::vector<BtnRectAndFRect> goodVariants;
	goodVariants.reserve(btnRects.size());

	for (BtnRect *btnRect : btnRects) {
		if (btnRect == selectedBtnRect) continue;
		if (btnRect->needIgnore()) continue;

		FRect fRect = getFRect(btnRect->owner);
		if (outsideScreen(fRect)) continue;
		if (!condition(prevPos, fRect)) continue;

		goodVariants.push_back({ btnRect, fRect });
	}

	std::stable_sort(goodVariants.begin(), goodVariants.end(),
	          [=](const BtnRectAndFRect &a, const BtnRectAndFRect &b) -> bool {
		float distA = getDistBetweenRects(prevFRect, a.fRect);
		float distB = getDistBetweenRects(prevFRect, b.fRect);

		if (distA - distB > 1) return false;
		if (distB - distA > 1) return true;

		FPoint centerA = getRectCenter(a.fRect);
		FPoint centerB = getRectCenter(b.fRect);
		return getDist(centerPrev, centerA) < getDist(centerPrev, centerB);
	});


	BtnRect *nextBtnRect = nullptr;
	FRect nextFRect;
	for (BtnRectAndFRect &i : goodVariants) {
		if (clickable(i.btnRect->owner, i.fRect)) {
			nextBtnRect = i.btnRect;
			nextFRect = i.fRect;
			break;
		}
	}

	if (!selectMode && !nextBtnRect && selectedBtnRect) {
		nextBtnRect = selectedBtnRect;
		nextFRect = getFRect(selectedBtnRect->owner);
	}

	if (!nextBtnRect) return;


//	Child *o = nextBtnRect->owner;
//	printf("%i, %i\n", int(o->getGlobalX()), int(o->getGlobalY()));

	for (BtnRect *btnRect : btnRects) {
		btnRect->mouseOvered = false;
	}

	if (selectedBtnRect) {
		selectedBtnRect->selectedByKeyboard = false;
	}

	selectMode = true;
	selectBtnId = nextBtnRect->id;
	selectBtnFRect = nextFRect;

	nextBtnRect->selectedByKeyboard = true;
}


void BtnRect::checkSelectedBtn() {
	BtnRect *rect = getSelectedBtnRect();
	if (!rect) {
		disableSelectMode(true);
	}else {
		if (rect->needIgnore()) {
			disableSelectMode();
		}else
		if (!clickable(rect->owner, getFRect(rect->owner))) {
			disableSelectMode(true);
		}
	}
}
