#include "renderer.h"
#include "utils/math.h"


static
bool operator==(const SDL_Rect &a, const SDL_Rect &b) {
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}
static
bool operator!=(const SDL_Rect &a, const SDL_Rect &b) {
	return !(a == b);
}

static
bool operator==(const SDL_Point &a, const SDL_Point &b) {
	return a.x == b.x && a.y == b.y;
}

bool RenderStruct::operator==(const RenderStruct &o) const {
	auto a = std::tie(  surface,   angle,   alpha,   clip,   clipRect,   srcRect,   dstRect,   center);
	auto b = std::tie(o.surface, o.angle, o.alpha, o.clip, o.clipRect, o.srcRect, o.dstRect, o.center);
	return a == b;
}


#include <thread>
#include <map>
#include <cmath>

#ifdef __WIN32__
    #undef _DLL
    #undef GL_NO_STDCALL
#endif
#include <SDL2/SDL_opengl.h>

#include "gv.h"
#include "config.h"
#include "gui/group.h"
#include "media/image_manipulator.h"

#include "utils/math.h"
#include "utils/image_caches.h"
#include "utils/utils.h"



static SDL_Renderer *renderer = nullptr;
static PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;

bool Renderer::needToRender = false;
bool Renderer::needToRedraw = false;
bool Renderer::needToUpdateViewPort = true;

std::mutex Renderer::renderMutex;

std::mutex Renderer::toRenderMutex;
std::vector<RenderStruct> Renderer::toRender;

SDL_RendererInfo Renderer::info;


static bool fastOpenGL = false;
static bool checkOpenGlErrors = false;

static void checkErrorsImpl(const char *from, const char *glFuncName) {
	GLuint error = glGetError();
	if (error == GL_NO_ERROR) return;

	fastOpenGL = false;
	Utils::outMsg("Renderer::" + std::string(from) + ", " + glFuncName, "Using OpenGL failed");
	ImageCaches::clearTextures();

	const char *str = "Unknown";
	     if (error == GL_INVALID_ENUM)      str = "GL_INVALID_ENUM";
	else if (error == GL_INVALID_VALUE)     str = "GL_INVALID_VALUE";
	else if (error == GL_INVALID_OPERATION) str = "GL_INVALID_OPERATION";
	else if (error == GL_STACK_OVERFLOW)    str = "GL_STACK_OVERFLOW";
	else if (error == GL_STACK_UNDERFLOW)   str = "GL_STACK_UNDERFLOW";
	else if (error == GL_OUT_OF_MEMORY)     str = "GL_OUT_OF_MEMORY";

	Utils::outMsg("Renderer::" + std::string(from) + ", " + glFuncName, str);
}
#define checkErrors(glFuncName) checkErrorsImpl(__FUNCTION__, glFuncName)


static bool surfaceIsOpaque(const SurfacePtr &surface) {
	SDL_Palette *palette = surface->format->palette;
	if (palette) {
		for (int i = 0; i < palette->ncolors; ++i) {
			if (palette->colors[i].a != 255) return false;
		}
		return true;
	}
	return surface->format->Amask == 0;
}

static SDL_Texture *currentTexture = nullptr;
static bool currentTextureIsOpaque = false;
static void bindTexture(SDL_Texture *texture, bool opaque) {
	if (!texture) return;

	if (currentTexture != texture) {
		if (!currentTexture) currentTextureIsOpaque = !opaque;
		currentTexture = texture;

		if (SDL_GL_BindTexture(texture, nullptr, nullptr)) {
			Utils::outMsg("SDL_GL_BindTexture", SDL_GetError());
		}
	}

	if (currentTextureIsOpaque != opaque) {
		currentTextureIsOpaque = opaque;

		if (opaque) {
			glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
			checkErrors("glBlendFuncSeparate");
		}else {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			checkErrors("glBlendFunc");
		}
	}
}
static void unbindTexture() {
	if (currentTexture) {
		SDL_GL_UnbindTexture(currentTexture);
		checkErrors("glBindTexture(GL_TEXTURE_2D, 0)");
		currentTexture = nullptr;
	}
}


static SDL_Rect currentClipRect = {0, 0, -1, -1};
static void setClipRect(const SDL_Rect *clipRect) {
	SDL_Rect rect;
	if (clipRect) {
		rect = *clipRect;
	}else {
		rect = {GV::screens->getX(), GV::screens->getY(), GV::width, GV::height};
	}

	if (currentClipRect == rect) return;

	currentClipRect = rect;
	if (fastOpenGL) {
		glScissor(rect.x, rect.y, rect.w, rect.h);
		if (checkOpenGlErrors) {
			checkErrors("glScissor");
		}
	}else {
		if (SDL_RenderSetClipRect(renderer, &rect)) {
			Utils::outMsg("SDL_RenderSetClipRect", SDL_GetError());
		}
	}
}


static void fastRenderOne(const RenderStruct *obj) {
	if (!obj->alpha || !obj->srcRect.w || !obj->srcRect.h || !obj->dstRect.w || !obj->dstRect.h) return;

	const bool check = checkOpenGlErrors;

	if (obj->angle) {
		int dX = obj->dstRect.x + obj->center.x;
		int dY = obj->dstRect.y + obj->center.y;

		glPushMatrix();
		if (check) {
			checkErrors("glPushMatrix");
		}
		glTranslated(dX, dY, 0);
		if (check) {
			checkErrors("glTranslated");
		}
		glRotated(obj->angle, 0, 0, 1);
		if (check) {
			checkErrors("glRotated");
		}
		glTranslated(-dX, -dY, 0);
		if (check) {
			checkErrors("glTranslated");
		}
	}

	float w = obj->surface->w;
	float h = obj->surface->h;

	float texMinX = obj->srcRect.x / w;
	float texMinY = obj->srcRect.y / h;
	float texMaxX = (obj->srcRect.x + obj->srcRect.w) / w;
	float texMaxY = (obj->srcRect.y + obj->srcRect.h) / h;

	int vertMinX = obj->dstRect.x;
	int vertMinY = obj->dstRect.y;
	int vertMaxX = obj->dstRect.x + obj->dstRect.w;
	int vertMaxY = obj->dstRect.y + obj->dstRect.h;

	glBegin(GL_QUADS);
	glTexCoord2f(texMinX, texMinY); glVertex2i(vertMinX, vertMinY);
	glTexCoord2f(texMinX, texMaxY); glVertex2i(vertMinX, vertMaxY);
	glTexCoord2f(texMaxX, texMaxY); glVertex2i(vertMaxX, vertMaxY);
	glTexCoord2f(texMaxX, texMinY); glVertex2i(vertMaxX, vertMinY);
	glEnd();
	if (check) {
		checkErrors("glBegin(GL_QUADS)/glTexCoord2f/glVertex2i/glEnd");
	}

	if (obj->angle) {
		glPopMatrix();
		if (check) {
			checkErrors("glPopMatrix");
		}
	}
}

static const size_t MAX_RENDER_GROUP_SIZE = 1024;
static void fastRenderGroup(const RenderStruct *startObj, size_t count) {
	float texBuffer[MAX_RENDER_GROUP_SIZE * 8];
	int vertexBuffer[MAX_RENDER_GROUP_SIZE * 8];

	float *texPtr = texBuffer;
	int *vertexPtr = vertexBuffer;

	float w = startObj->surface->w;
	float h = startObj->surface->h;

	const RenderStruct *obj = startObj;
	const RenderStruct *endObj = startObj + count;
	while (obj != endObj) {
		float texMinX = obj->srcRect.x / w;
		float texMinY = obj->srcRect.y / h;
		float texMaxX = (obj->srcRect.x + obj->srcRect.w) / w;
		float texMaxY = (obj->srcRect.y + obj->srcRect.h) / h;

		texPtr[0] = texMinX;
		texPtr[1] = texMinY;
		texPtr[2] = texMinX;
		texPtr[3] = texMaxY;
		texPtr[4] = texMaxX;
		texPtr[5] = texMaxY;
		texPtr[6] = texMaxX;
		texPtr[7] = texMinY;
		texPtr += 8;

		int vertMinX = obj->dstRect.x;
		int vertMinY = obj->dstRect.y;
		int vertMaxX = obj->dstRect.x + obj->dstRect.w;
		int vertMaxY = obj->dstRect.y + obj->dstRect.h;

		vertexPtr[0] = vertMinX;
		vertexPtr[1] = vertMinY;
		vertexPtr[2] = vertMinX;
		vertexPtr[3] = vertMaxY;
		vertexPtr[4] = vertMaxX;
		vertexPtr[5] = vertMaxY;
		vertexPtr[6] = vertMaxX;
		vertexPtr[7] = vertMinY;

		if (obj->angle) {
			const double sinA = Math::getSin(obj->angle);
			const double cosA = Math::getCos(obj->angle);
			const SDL_Point center = {vertMinX + obj->center.x, vertMinY + obj->center.y};

			auto rotate = [&](SDL_Point *a) {
				int x = a->x - center.x;
				int y = a->y - center.y;

				a->x = int(x * cosA - y * sinA) + center.x;
				a->y = int(x * sinA + y * cosA) + center.y;
			};
			rotate(reinterpret_cast<SDL_Point*>(vertexPtr + 0));
			rotate(reinterpret_cast<SDL_Point*>(vertexPtr + 2));
			rotate(reinterpret_cast<SDL_Point*>(vertexPtr + 4));
			rotate(reinterpret_cast<SDL_Point*>(vertexPtr + 6));
		}
		vertexPtr += 8;

		++obj;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	checkErrors("glEnableClientState(GL_VERTEX_ARRAY)");
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	checkErrors("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");

	glTexCoordPointer(2, GL_FLOAT, 0, texBuffer);
	checkErrors("glTexCoordPointer");
	glVertexPointer(2, GL_INT, 0, vertexBuffer);
	checkErrors("glVertexPointer");

	glDrawArrays(GL_QUADS, 0, int(count * 4));
	checkErrors("glDrawArrays");

	glDisableClientState(GL_VERTEX_ARRAY);
	checkErrors("glDisableClientState(GL_VERTEX_ARRAY)");
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	checkErrors("glDisableClientState(GL_TEXTURE_COORD_ARRAY)");
}

static void fastRender(const RenderStruct *obj, size_t count) {
	glColor4f(1, 1, 1, GLfloat(obj->alpha) / 255);
	checkErrors("glColor4f");
	glBlendColor(1, 1, 1, GLfloat(obj->alpha) / 255);
	checkErrors("glBlendColor");

	if (count < 7) {
		for (size_t i = 0; i < count; ++i) {
			fastRenderOne(obj + i);
		}
	}else {
		fastRenderGroup(obj, count);
	}
}



static SDL_Texture *tmpTexture = nullptr;
static int textureWidth = 0;
static int textureHeight = 0;

static SurfacePtr toScaleSurface;
static SurfacePtr scaledSurface;
static bool scaled = true;
static int scaleWidth = 0;
static int scaleHeight = 0;
static std::mutex scaledMutex;

SurfacePtr Renderer::getScaled(const SurfacePtr &src, int width, int height) {
	std::lock_guard g(scaledMutex);

	toScaleSurface = src;
	scaleWidth = width;
	scaleHeight = height;

	scaled = false;
	while (!scaled) {
		Utils::sleep(1);
	}
	toScaleSurface.reset();

	return scaledSurface;
}
static void scale() {
	if (scaleWidth > textureWidth || scaleHeight > textureHeight) {
		SDL_DestroyTexture(tmpTexture);

		textureWidth  = int(std::ceil(scaleWidth / 256.0)) * 256;
		textureHeight = int(std::ceil(scaleHeight / 256.0)) * 256;

		tmpTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, textureWidth, textureHeight);
		if (!tmpTexture) {
			Utils::outMsg("SDL_CreateTexture", SDL_GetError());
			scaledSurface = nullptr;
			scaled = true;
			return;
		}
	}

	SDL_Rect dstRect = {0, 0, scaleWidth, scaleHeight};
	TexturePtr toRender = ImageCaches::getTexture(renderer, toScaleSurface);

	if (SDL_SetRenderTarget(renderer, tmpTexture)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget", SDL_GetError());
	}
	if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderDrawColor", SDL_GetError());
	}
	if (SDL_RenderClear(renderer)) {
		Utils::outMsg("Renderer::scale, SDL_RenderClear", SDL_GetError());
	}
	if (SDL_RenderCopy(renderer, toRender.get(), nullptr, &dstRect)) {
		Utils::outMsg("Renderer::scale, SDL_RenderCopy", SDL_GetError());
	}

	scaledSurface = ImageManipulator::getNewNotClear(scaleWidth, scaleHeight);
	if (SDL_RenderReadPixels(renderer, &dstRect, SDL_PIXELFORMAT_RGBA32, scaledSurface->pixels, scaledSurface->pitch)) {
		Utils::outMsg("Renderer::scale, SDL_RenderReadPixels", SDL_GetError());
	}

	if (SDL_SetRenderTarget(renderer, nullptr)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget(nullptr)", SDL_GetError());
	}

	scaled = true;
	Renderer::needToUpdateViewPort = true;
}



static SurfacePtr screenshot;
static bool screenshoted = true;

void Renderer::needMakeScreenshot() {
	screenshoted = false;
	needToRender = needToRedraw = true;
}
SurfacePtr Renderer::getScreenshot() {
	while (!screenshoted) {
		Utils::sleep(1);
	}
	return screenshot;
}

static void readPixels() {
	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);

	screenshot = ImageManipulator::getNewNotClear(w, h);
	if (SDL_RenderReadPixels(renderer, &screenshot->clip_rect,
							 SDL_PIXELFORMAT_RGBA32, screenshot->pixels, screenshot->pitch))
	{
		Utils::outMsg("SDL_RenderReadPixels", SDL_GetError());
	}
}


static void loop() {
	if (fastOpenGL) {
		checkOpenGlErrors = Config::get("check_fast_opengl_errors") == "True";
	}

	static std::vector<RenderStruct> toRender;
	static std::vector<RenderStruct> prevToRender;

	static std::vector<TexturePtr> textures;
	static std::vector<TexturePtr> prevTextures;


	while (!GV::exit) {
		if (!scaled) {
			scale();
			continue;
		}

		if (!Renderer::needToRender || GV::minimized) {
			Utils::sleep(1, false);
			continue;
		}


		{
			std::lock_guard g(Renderer::toRenderMutex);
			toRender.clear();
			Renderer::toRender.swap(toRender);
		}
		Renderer::needToRender = false;

		if (!Renderer::needToRedraw && toRender == prevToRender) continue;
		Renderer::needToRedraw = false;


		std::lock_guard g(Renderer::renderMutex);

		{
			std::map<SurfacePtr, TexturePtr> cache;
			textures.clear();
			textures.reserve(toRender.size());

			for (const RenderStruct &rs : toRender) {
				auto it = cache.find(rs.surface);
				if (it != cache.end()) {
					textures.push_back(it->second);
				}else {
					const TexturePtr texture = ImageCaches::getTexture(renderer, rs.surface);
					textures.push_back(texture);
					cache[rs.surface] = texture;
				}
			}
		}

		if (fastOpenGL) {
			checkErrors("Start");

			if (Renderer::needToUpdateViewPort) {
				Renderer::needToUpdateViewPort = false;

				int x = GV::screens->getX();
				int y = GV::screens->getY();
				int w = GV::width;
				int h = GV::height;

				glViewport(x, y, w, h);
				checkErrors("glViewport");
				glMatrixMode(GL_PROJECTION);
				checkErrors("glMatrixMode");
				glLoadIdentity();
				glOrtho(x, w + x, h + y, y, 0.0, 1.0);
				checkErrors("glOrtho");
				glMatrixMode(GL_MODELVIEW);
				checkErrors("glMatrixMode");
			}
		}
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
			Utils::outMsg("SDL_SetRenderDrawColor", SDL_GetError());
		}
		if (SDL_RenderClear(renderer)) {
			Utils::outMsg("SDL_RenderClear", SDL_GetError());
		}

		if (fastOpenGL && !textures.empty()) {
			glEnable(GL_BLEND);
			checkErrors("glEnable(GL_BLEND)");
			glEnable(GL_SCISSOR_TEST);
			checkErrors("glEnable(GL_SCISSOR_TEST)");

			size_t start = 0;
			SDL_Texture *prevTexture = nullptr;
			bool prevIsOpaque = false;
			Uint8 prevAlpha = 0;
			bool prevClip = false;
			SDL_Rect prevClipRect = {0, 0, 0, 0};

			size_t count = 0;
			for (size_t i = 0; i < textures.size(); ++i) {
				if (!GV::inGame) break;

				const RenderStruct &rs = toRender[i];
				SDL_Texture *texture = textures[i].get();

				if (prevTexture != texture || prevAlpha != rs.alpha ||
				    prevClip != rs.clip || prevClipRect != rs.clipRect || count == MAX_RENDER_GROUP_SIZE)
				{
					setClipRect(prevClip ? &prevClipRect : nullptr);
					bindTexture(prevTexture, prevIsOpaque);
					fastRender(toRender.data() + start, count);

					prevTexture = texture;
					prevIsOpaque = surfaceIsOpaque(rs.surface);
					prevAlpha = rs.alpha;
					prevClip = rs.clip;
					prevClipRect = rs.clipRect;
					start = i;
					count = 0;
				}
				++count;
			}
			setClipRect(prevClip ? &prevClipRect : nullptr);
			bindTexture(prevTexture, prevIsOpaque);
			fastRender(toRender.data() + start, count);
			unbindTexture();
		}else {
			for (size_t i = 0; i < textures.size(); ++i) {
				if (!GV::inGame) break;

				const RenderStruct &rs = toRender[i];
				const TexturePtr &texture = textures[i];

				setClipRect(rs.clip ? &rs.clipRect : nullptr);

				if (SDL_SetTextureAlphaMod(texture.get(), rs.alpha)) {
					Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
				}

				if (SDL_RenderCopyEx(renderer, texture.get(),
				                     &rs.srcRect,
				                     &rs.dstRect,
				                     rs.angle,
				                     &rs.center,
				                     SDL_FLIP_NONE))
				{
					Utils::outMsg("SDL_RenderCopyEx", SDL_GetError());
				}
			}
		}

		if (GV::screens) {
			int w = GV::screens->getX();
			int h = GV::screens->getY();
			SDL_Rect empty = {0, 0, GV::width, GV::height};
			if (!h) {
				empty.w = w;
			}else {
				empty.h = h;
			}

			if (empty.w && empty.h) {
				setClipRect(&empty);

				if (fastOpenGL) {
					glColor4f(0, 0, 0, 1);
					checkErrors("glColor4f");
					glRecti(0, 0, empty.w, empty.h);
					checkErrors("glRecti");
				}else {
					if (SDL_RenderFillRect(renderer, &empty)) {
						Utils::outMsg("SDL_RenderFillRect", SDL_GetError());
					}
				}
			}

			if (!screenshoted) {
				readPixels();
				Renderer::needToRedraw = true;
				screenshoted = true;
			}else {
				SDL_RenderPresent(renderer);
			}
		}

		prevToRender.swap(toRender);
		prevTextures.swap(textures);
	}

	SDL_DestroyRenderer(renderer);
}


static void initImpl(bool *inited, bool *error) {
	int renderDriver = -1;

	Uint32 flags;
	if (Config::get("software_renderer") == "True") {
		flags = SDL_RENDERER_SOFTWARE;
	}else {
		flags = SDL_RENDERER_ACCELERATED;
		if (Config::get("opengl_vsync") == "True") {
			flags |= SDL_RENDERER_PRESENTVSYNC;
		}

		std::string opengl = "opengl";
		int countRenderDrivers = SDL_GetNumRenderDrivers();
		for (int i = 0; i < countRenderDrivers; ++i) {
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(i, &info);
			if (info.name == opengl) {
				renderDriver = i;
				break;
			}
		}
		if (renderDriver == -1) {
			Utils::outMsg("Renderer::init", "OpenGL driver not found");
		}else {
			fastOpenGL = Config::get("fast_opengl") == "True";
			if (fastOpenGL) {
				SDL_SetHint(SDL_HINT_RENDER_BATCHING, "false");
			}
		}
	}

	renderer = SDL_CreateRenderer(GV::mainWindow, -1, flags);
	if (!renderer) {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());

		if (flags & SDL_RENDERER_ACCELERATED) {
			renderer = SDL_CreateRenderer(GV::mainWindow, -1, SDL_RENDERER_SOFTWARE);
			if (!renderer) {
				Utils::outMsg("SDL_CreateRenderer", SDL_GetError());
			}
		}

		if (!renderer) {
			*error = true;
			*inited = true;
			return;
		}
	}

	SDL_GetRendererInfo(renderer, &Renderer::info);
	if (std::string(Renderer::info.name) == "opengl") {
		size_t countErrors = 0;
		const size_t maxCountErrors = 10;

		while (++countErrors < maxCountErrors && glGetError() != GL_NO_ERROR) {}
		if (countErrors == maxCountErrors) {
			Utils::outMsg("Renderer::init", "Using OpenGL failed");
			fastOpenGL = false;
		}
	}
	if (fastOpenGL) {
		glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)SDL_GL_GetProcAddress("glBlendFuncSeparate");
		if (!glBlendFuncSeparate) {
			Utils::outMsg("Renderer::init", "glBlendFuncSeparate not found");
			fastOpenGL = false;
		}
	}

	*inited = true;
	loop();
}
bool Renderer::init() {
	bool inited = false;
	bool error = false;

	std::thread(initImpl, &inited, &error).detach();
	while (!inited) {
		Utils::sleep(1, false);
	}

	return error;
}
