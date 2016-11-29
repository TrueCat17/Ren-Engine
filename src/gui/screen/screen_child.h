#ifndef SCREENCHILD_H
#define SCREENCHILD_H

#include "../group.h"
#include "parser/node.h"

class ScreenChild: public Group {
protected:
	static std::vector<ScreenChild*> screenObjects;

	ScreenChild *screenParent;

	Node *node;


	double xAnchor = 0;
	double yAnchor = 0;

	double xPos = 0;
	double yPos = 0;

	double xSize = 0.5;
	double ySize = 0.5;

	virtual double getDefaultWidth() const;
	virtual double getDefaultHeight() const;

	bool needUpdateChildren = true;

public:
	static void disableAll();


	std::vector<ScreenChild*> screenChildren;
	ScreenChild *propsUpdater = nullptr;

	ScreenChild(Node *node, ScreenChild *screenParent);
	virtual ~ScreenChild();

	virtual void updateProps();
	virtual void update();
};

#endif // SCREENCHILD_H
