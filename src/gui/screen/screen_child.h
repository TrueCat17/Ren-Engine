#ifndef SCREENCHILD_H
#define SCREENCHILD_H

#include "../group.h"

#include  "screen_prop.h"

#include "parser/node.h"

class Screen;


class ScreenChild: public Group {
private:
	bool _isFakeContainer = false;
	bool _canCrop = false;

protected:
	static std::vector<ScreenChild*> screenObjects;


	const Screen *screen = nullptr;
	ScreenChild *screenParent;

	Node *node;

	//props -> calculate propValue
	std::vector<NodeProp> props;
	std::vector<py::object> propValues;
	std::vector<ScreenProp> propIndeces;
	std::vector<char> propWasChanged;

	String codeForCalcProps;
	PyCodeObject *co = nullptr;


	bool needUpdateChildren = true;
	bool needUpdateFields = true;//xAnchor, yAnchor, xPos... Next fields:

	bool xAnchorIsDouble = false;
	bool yAnchorIsDouble = false;
	double preXAnchor = 0;
	double preYAnchor = 0;

	double xPos = 0;
	double yPos = 0;
	bool xPosIsDouble = false;
	bool yPosIsDouble = false;

	bool usingXAlign = false;
	bool usingYAlign = false;

	double xSize = 0;
	double ySize = 0;
	bool xSizeIsDouble = false;
	bool ySizeIsDouble = false;

	bool xSizeIsTextureWidth = false;
	bool ySizeIsTextureHeight = false;


public:
	static void disableAll();
	static const std::vector<ScreenChild*> getScreenObjects() { return screenObjects; }

	std::vector<ScreenChild*> screenChildren;
	ScreenChild *propsUpdater = nullptr;

	ScreenChild(Node *node, ScreenChild *screenParent);
	virtual ~ScreenChild();

	void clearProps();
	void setProp(const ScreenProp prop, const NodeProp &nodeProp);
	void preparationToUpdateCalcProps();

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
