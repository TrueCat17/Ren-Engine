#ifndef SCREENCHILD_H
#define SCREENCHILD_H

#include "../group.h"

class ScreenWindow;
class Node;

class String;

class ScreenChild: public Group {
private:
	bool _isFakeContainer = false;

protected:
	static std::vector<ScreenChild*> screenObjects;

	const ScreenWindow *window = nullptr;
	ScreenChild *screenParent;

	Node *node;

	double xAnchor = 0;
	double yAnchor = 0;

	double xPos = 0;
	double yPos = 0;

	double xSize = 0.5;
	double ySize = 0.5;

	bool needUpdateChildren = true;

public:
	static void disableAll();
	static const std::vector<ScreenChild*> getScreenObjects() { return screenObjects; }

	std::vector<ScreenChild*> screenChildren;
	ScreenChild *propsUpdater = nullptr;

	ScreenChild(Node *node, ScreenChild *screenParent);
	virtual ~ScreenChild();

	const String& getType() const;

	virtual void updateProps();
	virtual void updateSize();
	virtual void updatePos();

	bool isModal() const;
	bool isFakeContainer() const { return _isFakeContainer; }
};

#endif // SCREENCHILD_H
