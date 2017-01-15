#ifndef GROUP_H
#define GROUP_H

#include <vector>

#include "display_object.h"

class Group: public DisplayObject {
public:
	std::vector<DisplayObject*> children;

	Group();
	virtual ~Group();

	virtual void updateGlobalX();
	virtual void updateGlobalY();

	DisplayObject* getChildAt(size_t index) const;
	size_t getChildIndex(DisplayObject *child) const;
	size_t getNumChildren() const { return children.size(); }

	void addChild(DisplayObject *child);
	virtual void addChildAt(DisplayObject *child, size_t index);

	virtual void removeChild(DisplayObject *child);
	virtual void removeChildAt(size_t index);
	virtual void clearChildren();

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;

	virtual SDL_Texture* getTextureIfOne(size_t &count) const;
};

#endif // GROUP_H
