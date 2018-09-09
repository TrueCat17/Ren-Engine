#ifndef SCREENCHILD_H
#define SCREENCHILD_H

#include <boost/python.hpp>

#include "../group.h"

#include  "screen_prop.h"

#include "parser/node.h"

class Screen;
class ScreenContainer;


class ScreenChild: public Group {
private:
	bool _isFakeContainer = false;
	bool _canCrop = false;

protected:
	static std::vector<ScreenChild*> screenObjects;

	bool inVBox = false;
	bool inHBox = false;

	bool needUpdateChildren = true;
	bool needUpdateFields = true;//xAnchor, yAnchor, xPos... Next fields:

	bool xAnchorIsDouble = false;
	bool yAnchorIsDouble = false;

	bool xPosIsDouble = false;
	bool yPosIsDouble = false;

	bool usingXAlign = false;
	bool usingYAlign = false;

	bool xSizeIsDouble = false;
	bool ySizeIsDouble = false;

	bool xSizeIsTextureWidth = false;
	bool ySizeIsTextureHeight = false;

	double xPos = 0;
	double yPos = 0;
	double preXAnchor = 0;
	double preYAnchor = 0;
	double xSize = 0;
	double ySize = 0;

	String codeForCalcProps;
	PyCodeObject *co = nullptr;

	Screen *screen;
	ScreenContainer *screenParent;

	Node *node;

	//props -> calculate propValue
	std::vector<NodeProp> props;
	std::vector<py::object> propValues;
	std::vector<ScreenProp> propIndeces;
	std::vector<char> propWasChanged;

public:
	static void disableAll();
	static const std::vector<ScreenChild*>& getScreenObjects() { return screenObjects; }

	std::vector<ScreenChild*> screenChildren;
	ScreenChild *propsUpdater = nullptr;

	ScreenChild(Node *node, ScreenContainer *screenParent, Screen *screen);
	virtual ~ScreenChild();

	void setInBox(bool v, bool h) { inVBox = v; inHBox = h; }

	void clearProps();
	void setProp(const ScreenProp prop, const NodeProp &nodeProp);
	void preparationToUpdateCalcProps();

	const String& getType() const { return node->command; }

	virtual void calculateProps();
	virtual void updateTexture() {}
	virtual void updateSize();
	virtual void updatePos();

	Screen* getScreen() const { return screen; }
	bool isModal() const;
	bool isFakeContainer() const { return _isFakeContainer; }
	bool canCrop() const { return _canCrop; }

	const String& getFileName() const { return node->getFileName(); }
	size_t getNumLine() const { return node->getNumLine(); }
};

#endif // SCREENCHILD_H
