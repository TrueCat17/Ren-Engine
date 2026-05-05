#ifndef STAGE_H
#define STAGE_H

#include "SDL3/SDL_video.h"

class Group;


class Stage {
public:
	static const int MIN_WIDTH = 640;
	static const int MIN_HEIGHT = 360;

	static int x, y, width, height;
	static bool fullscreen;
	static bool needResize;
	static bool minimized;
	static bool maximized;

	static SDL_Window *window;
	static Group *screens;


	static SDL_DisplayMode getDisplayMode();
	static int getMaxWidth();
	static int getMaxHeight();

	static int getStageWidth();
	static int getStageHeight();

	static void setStageSize(int width, int height);
	static void setFullscreen(bool value);

	static void changeWindowSize(int startW = 0, int startH = 0, int prevWindowWidth = 0, int prevWindowHeight = 0);
	static void applyChanges();
};

#endif // STAGE_H
