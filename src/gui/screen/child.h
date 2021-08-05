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

public:
	bool xposIsFloat       = false, yposIsFloat       = false;
	bool xanchorPreIsFloat = false, yanchorPreIsFloat = false;
	bool xsizeIsFloat      = false, ysizeIsFloat      = false;

	bool xcropIsFloat = false, ycropIsFloat = false,
	     wcropIsFloat = false, hcropIsFloat = false;

	float xpos       = 0, ypos       = 0;
	float xanchorPre = 0, yanchorPre = 0;
	float xsize      = 0, ysize      = 0;

	float xcrop = 0, ycrop = 0,
	      wcrop = 0, hcrop = 0;

	std::string first_param;

	Node *node;

	Container *screenParent;

	const std::vector<ScreenUpdateFunc> *updateFuncs = nullptr;
	PyObject *props = nullptr;


	Child(Node *node, Container *screenParent, Screen *screen);

	void updateProps();
	virtual void updateZoom();
	virtual void updatePos();
	virtual void updateRect(bool needUpdatePos = true);

	virtual void updateTexture(bool skipError = false);
	virtual void checkEvents();

	void setInBox(bool v, bool h) { inVBox = v; inHBox = h; }

	bool wasInited() const { return inited; }

	Screen* getScreen() const { return screen; }
	bool isFakeContainer() const { return screenParent && size_t(screenParent) != size_t(this); }
	bool isModal() const;

	const std::string& getFileName() const { return node->getFileName(); }
	size_t getNumLine() const { return node->getNumLine(); }
};

#endif // CHILD_H
