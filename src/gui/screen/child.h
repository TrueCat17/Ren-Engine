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

	void updatePos();

public:
	bool xposIsDouble, yposIsDouble;
	bool xanchorPreIsDouble, yanchorPreIsDouble;
	bool xsizeIsDouble, ysizeIsDouble;
	bool xcropIsDouble, ycropIsDouble, wcropIsDouble, hcropIsDouble;

	bool xsizeIsTextureWidth, ysizeIsTextureHeight;

	double xpos, ypos;
	double xanchorPre, yanchorPre;
	double xsize, ysize;

	double xcrop, ycrop, wcrop, hcrop;

	String first_param;

	Node *const node;

	Container *screenParent;

	const std::vector<ScreenUpdateFunc> *const updateFuncs;
	PyObject *props = nullptr;


	Child(Node *node, Container *screenParent, Screen *screen);

	void updateProps();
	virtual void updateRect(bool callFromContainer = false);

	virtual void updateTexture(bool skipError = false);
	virtual void checkEvents();

	void setInBox(bool v, bool h) { inVBox = v; inHBox = h; }

	const String& getType() const { return node->command; }

	Screen* getScreen() const { return screen; }
	bool isModal() const;

	const String& getFileName() const { return node->getFileName(); }
	size_t getNumLine() const { return node->getNumLine(); }
};

#endif // CHILD_H
