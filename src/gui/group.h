#ifndef GROUP_H
#define GROUP_H

#include "display_object.h"

class Group: public DisplayObject {
public:
	std::vector<DisplayObject*> children;

	Group() {}
	virtual ~Group();

	void removeChild(DisplayObject *child);
	void clearChildren();

	virtual void addChildAt(DisplayObject *child, uint32_t index);

	virtual void updateGlobal();

	virtual bool transparentForMouse(int x, int y) const;
	virtual void draw() const;
};

#endif // GROUP_H
