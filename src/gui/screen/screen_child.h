#ifndef SCREENCHILD_H
#define SCREENCHILD_H

#include "../group.h"

#include "parser/node.h"

class Screen;
class String;

class ScreenChild: public Group {
private:
	bool _isFakeContainer = false;
	bool _canCrop = false;

protected:
	static std::vector<ScreenChild*> screenObjects;

	const Screen *screen = nullptr;
	ScreenChild *screenParent;

	Node *node;

	double xAnchor = 0;
	double yAnchor = 0;
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

	const String& getType() const { return node->command; }

	virtual void calculateProps();
	virtual void updateSize();
	virtual void updatePos();

	bool isModal() const;
	bool isFakeContainer() const { return _isFakeContainer; }
	bool canCrop() const { return _canCrop; }

	const String& getFileName() const { return node->getFileName(); }
	size_t getNumLine() const { return node->getNumLine(); }
};

#endif // SCREENCHILD_H
