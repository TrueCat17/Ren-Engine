#ifndef CHILD_H
#define CHILD_H

#include "../group.h"

#include "parser/node.h"
#include "parser/screen_node_utils.h"

class Screen;
class Container;


class Child: public Group {
protected:
	Screen *screen;

	bool inited = false;

	bool inVBox = false;
	bool inHBox = false;

	bool xsizeIsTextureWidth = false, ysizeIsTextureHeight = false;

	void updatePos();

public:
	bool xposIsDouble       = false, yposIsDouble       = false;
	bool xanchorPreIsDouble = false, yanchorPreIsDouble = false;
	bool xsizeIsDouble      = false, ysizeIsDouble      = false;

	bool xcropIsDouble = false, ycropIsDouble = false,
		 wcropIsDouble = false, hcropIsDouble = false;

	double xpos       = 0, ypos       = 0;
	double xanchorPre = 0, yanchorPre = 0;
	double xsize      = 0, ysize      = 0;

	double xcrop = 0, ycrop = 0,
		   wcrop = 0, hcrop = 0;

	String first_param;

	Node *const node;

	Container *screenParent;

	const std::vector<ScreenUpdateFunc> *const updateFuncs;
	PyObject *props = nullptr;


	Child(Node *node, Container *screenParent, Screen *screen);

	void updateProps();
	virtual void updateRect(bool needUpdatePos = true);

	virtual void updateTexture(bool skipError = false);
	virtual void checkEvents();

	void setInBox(bool v, bool h) { inVBox = v; inHBox = h; }

	bool wasInited() const { return inited; }
	const String& getType() const { return node->command; }

	Screen* getScreen() const { return screen; }
	bool isFakeContainer() const { return screenParent && size_t(screenParent) != size_t(this); }
	bool isModal() const;

	const String& getFileName() const { return node->getFileName(); }
	size_t getNumLine() const { return node->getNumLine(); }
};

#endif // CHILD_H
