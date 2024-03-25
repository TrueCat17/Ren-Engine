#ifndef CHILD_H
#define CHILD_H

#include "../group.h"

#include "parser/node.h"
#include "parser/screen_update_funcs.h"

class Screen;
class Container;
struct Style;


class Child: public Group {
private:
	void updateStyle();
protected:
	Screen *screen;

	bool inited:1;

	bool inVBox:1;
	bool inHBox:1;

public:
	bool selected:1;//for TextButton and Hotspot

	bool xpos_is_float:1;
	bool ypos_is_float:1;
	bool xanchor_is_float:1;
	bool yanchor_is_float:1;

	bool xsize_is_float:1;
	bool ysize_is_float:1;
	bool xsize_min_is_float:1;
	bool ysize_min_is_float:1;
	bool xsize_max_is_float:1;
	bool ysize_max_is_float:1;

	bool xcrop_is_float:1;
	bool ycrop_is_float:1;
	bool wcrop_is_float:1;
	bool hcrop_is_float:1;

	float xpos    = 0, ypos    = 0;
	float xanchor = 0, yanchor = 0;

	float xsize     = 0, ysize     = 0;
	float xsize_min = 0, ysize_min = 0;
	float xsize_max = 0, ysize_max = 0;

	float xcrop = 0, ycrop = 0,
	      wcrop = 0, hcrop = 0;

	std::string first_param;

	Node *node;

	Container *screenParent;

	const std::vector<ScreenUpdateFunc> *updateFuncs = nullptr;
	PyObject *props = nullptr;

	const Style *style = nullptr;
	const Style *prevStyle = nullptr;


	Child(Node *node, Container *screenParent, Screen *screen);

	void updateProps();
	virtual void updateZoom();
	virtual void updatePos();
	virtual void updateRect(bool needUpdatePos = true);

	virtual void updateTexture();
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
