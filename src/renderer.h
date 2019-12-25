#ifndef RENDERER_H
#define RENDERER_H

#include <mutex>
#include <vector>

#include "utils/image_typedefs.h"


struct RenderStruct {
	SurfacePtr surface;
	int angle;
	Uint8 alpha;

	bool clip;
	SDL_Rect clipRect;

	SDL_Rect srcRect;
	SDL_Rect dstRect;

	SDL_Point center;

	bool operator==(const RenderStruct &o) const;
};

class Renderer {
public:
	static bool needToRender;
	static bool needToRedraw;

	static std::mutex renderMutex;

	static std::mutex toRenderMutex;
	static std::vector<RenderStruct> toRender;

	static SDL_RendererInfo info;
	static bool init();

	static void needMakeScreenshot();
	static SurfacePtr getScreenshot();

	static SurfacePtr getScaled(const SurfacePtr &src, int width, int height);
};

#endif // RENDERER_H
