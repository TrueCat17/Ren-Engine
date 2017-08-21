#ifndef SCREENCHILD_H
#define SCREENCHILD_H

#include <functional>

#include "../group.h"

#include "parser/node.h"


class Screen;


enum ScreenProp {
	//common
	X_POS,
	Y_POS,
	X_ANCHOR,
	Y_ANCHOR,
	X_ALIGN,
	Y_ALIGN,
	X_SIZE,
	Y_SIZE,
	CROP,
	ROTATE,
	ALPHA,

	//image, buttons, imagemap
	IMAGE_PATH,
	GROUND,
	HOVER,

	//hotspot
	RECT,

	//buttons
	MOUSE,

	//screen, vbox, hbox
	SPACING,
	MODAL,
	ZORDER,

	//key
	KEY,
	DELAY,
	FIRST_DELAY,

	//text
	TEXT,
	FONT,
	TEXT_SIZE,
	TEXT_ALIGN,
	TEXT_VALIGN,
	COLOR,


	//not use, most be ALWAYS last
	LAST_POINT
};
const size_t COUNT_PROPS = ScreenProp::LAST_POINT;


class ScreenChild: public Group {
private:
	bool _isFakeContainer = false;
	bool _canCrop = false;

protected:
	static std::vector<ScreenChild*> screenObjects;


	const Screen *screen = nullptr;
	ScreenChild *screenParent;

	Node *node;

	//pyExprs -> calculate propValue
	std::vector<String> propCodes;
	std::vector<String> propValues;
	std::vector<size_t> propNumLines;

	//styleProps -> propValue
	std::vector<NodeProp> propInStyle;//propName - path in style-Object


	bool needUpdateFields = true;//xAnchor, yAnchor, xPos... Next fields:

	bool xAnchorIsDouble = false;
	bool yAnchorIsDouble = false;

	double xPos = 0;
	double yPos = 0;
	bool xPosIsDouble = false;
	bool yPosIsDouble = false;

	double xSize = 0.5;
	double ySize = 0.5;
	bool xSizeIsDouble = false;
	bool ySizeIsDouble = false;


	bool needUpdateChildren = true;

public:
	static void disableAll();
	static const std::vector<ScreenChild*> getScreenObjects() { return screenObjects; }

	std::vector<ScreenChild*> screenChildren;
	ScreenChild *propsUpdater = nullptr;

	ScreenChild(Node *node, ScreenChild *screenParent);
	virtual ~ScreenChild();

	void clearProps();
	void setProp(const ScreenProp prop, const NodeProp &nodeProp);

	const String& getType() const { return node->command; }

	virtual void calculateProps();
	virtual void updateTexture() {}
	virtual void updateSize();
	virtual void updatePos();

	bool isModal() const;
	bool isFakeContainer() const { return _isFakeContainer; }
	bool canCrop() const { return _canCrop; }

	const String& getFileName() const { return node->getFileName(); }
	size_t getNumLine() const { return node->getNumLine(); }
};

#endif // SCREENCHILD_H
