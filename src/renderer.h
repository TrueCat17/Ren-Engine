#ifndef RENDERER_H
#define RENDERER_H

#include <mutex>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "gv.h"


class Renderer {
public:
	static bool needToRender;
	static bool needToRedraw;

	static std::mutex renderMutex;

	static std::mutex toRenderMutex;
	static std::vector<RenderStruct> toRender;


	static void renderThreadFunc();

private:
	static void renderWithOpenGL(SDL_Texture *texture, Uint8 alpha,
								 int angle, const SDL_Point *center,
								 const SDL_Rect *src, const SDL_Rect *dst);
	static GLuint getTextureId(SDL_Texture *texture);
};

#endif // RENDERER_H
