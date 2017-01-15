#ifndef DISPLAY_OBJECT_H
#define DISPLAY_OBJECT_H

#include <vector>

#include <SDL2/SDL.h>

class Group;

class DisplayObject {
protected:
	int globalX = 0;
	int globalY = 0;

	SDL_Rect rect;

public:
	static std::vector<DisplayObject*> objects;
	static bool useTexture(SDL_Texture *texture);
	static void destroyAll();

	bool enable = true;

	SDL_Texture* texture = nullptr;

	Group *parent = nullptr;

	DisplayObject();
	virtual ~DisplayObject();

	void removeFromParent();

	int getGlobalX() const { return globalX; }
	int getGlobalY() const { return globalY; }

	virtual void updateGlobalX();
	virtual void updateGlobalY();

	int getX() const { return rect.x; }
	int getY() const { return rect.y; }

	void setX(int value) { rect.x = value; updateGlobalX(); }
	void setY(int value) { rect.y = value; updateGlobalY(); }

	int getWidth() const { return rect.w; }
	int getHeight() const { return rect.h; }

	void setPos(int x, int y);
	void setSize(int w, int h);

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;

	virtual SDL_Texture* getTextureIfOne(size_t &count) const;
};

#endif // DISPLAY_OBJECT_H
