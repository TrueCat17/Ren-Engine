#include "renderer.h"

#include <thread>
#include <map>
#include <cmath>

#ifdef __WIN32__
    #undef _DLL
    #undef GL_NO_STDCALL
#endif
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>

#include "gv.h"
#include "config.h"
#include "media/image_manipulator.h"

#include "utils/math.h"
#include "utils/image_caches.h"
#include "utils/scope_exit.h"
#include "utils/stage.h"
#include "utils/utils.h"


bool operator==(const SDL_Rect &a, const SDL_Rect &b) {
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}
bool operator!=(const SDL_Rect &a, const SDL_Rect &b) {
	return !(a == b);
}

bool operator==(const SDL_FRect &a, const SDL_FRect &b) {
	return Math::floatsAreEq(a.x, b.x) && Math::floatsAreEq(a.y, b.y) &&
	       Math::floatsAreEq(a.w, b.w) && Math::floatsAreEq(a.h, b.h);
}
bool operator!=(const SDL_FRect &a, const SDL_FRect &b) {
	return !(a == b);
}

bool operator==(const SDL_Point &a, const SDL_Point &b) {
	return a.x == b.x && a.y == b.y;
}

bool RenderStruct::operator==(const RenderStruct &o) const {
	auto a = std::tie(  surface,   angle,   alpha,   clip,   clipRect,   srcRect,   dstRect,   center);
	auto b = std::tie(o.surface, o.angle, o.alpha, o.clip, o.clipRect, o.srcRect, o.dstRect, o.center);
	return a == b;
}



typedef void (GLAPIENTRY *BLENDCOLOR_TYPE)(GLclampf, GLclampf, GLclampf, GLclampf);
static BLENDCOLOR_TYPE blendColor;
static PFNGLBLENDFUNCSEPARATEPROC blendFuncSeparate;

static int windowWidth, windowHeight;
static SDL_Renderer *renderer = nullptr;

bool Renderer::needToRender = false;
bool Renderer::needToRedraw = false;
bool Renderer::needToUpdateViewPort = true;

std::mutex Renderer::renderMutex;

std::mutex Renderer::renderDataMutex;
std::vector<RenderStruct> Renderer::renderData;

SDL_RendererInfo Renderer::info;


static bool fastOpenGL = false;
static bool checkOpenGlErrors = false;

static void checkErrorsImpl(const char *from, const char *glFuncName) {
	GLuint error = glGetError();
	if (error == GL_NO_ERROR) return;

	bool mutexWasLocked = !Renderer::renderMutex.try_lock();
	Renderer::renderMutex.unlock();

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

	if (mutexWasLocked) {
		Renderer::renderMutex.lock();
	}
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
			blendFuncSeparate(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_ONE, GL_ONE_MINUS_CONSTANT_ALPHA);
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
		rect = { Stage::x, Stage::y, Stage::width, Stage::height };
	}

	if (currentClipRect == rect) return;
	currentClipRect = rect;

	if (fastOpenGL) {
		glScissor(rect.x, windowHeight - rect.y - rect.h, rect.w, rect.h);//at glScissor lower pixel - y=0
		if (checkOpenGlErrors) {
			checkErrors("glScissor");
		}
	}else {
		if (SDL_RenderSetClipRect(renderer, &rect)) {
			Utils::outMsg("SDL_RenderSetClipRect", SDL_GetError());
		}
	}
}
static void disableClipRect() {
	if (fastOpenGL) {
		glDisable(GL_SCISSOR_TEST);
		checkErrors("glDisable(GL_SCISSOR_TEST)");
	}else {
		if (SDL_RenderSetClipRect(renderer, nullptr)) {
			Utils::outMsg("SDL_RenderSetClipRect", SDL_GetError());
		}
	}
}


template<bool check>
static void fastRenderOne(const RenderStruct *obj) {
	if (!obj->alpha || !obj->srcRect.w || !obj->srcRect.h || !obj->dstRect.w || !obj->dstRect.h) return;

	int angle = int(obj->angle);
	if (angle % 360) {
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
		glRotatef(obj->angle, 0, 0, 1);
		if (check) {
			checkErrors("glRotated");
		}
		glTranslated(-dX, -dY, 0);
		if (check) {
			checkErrors("glTranslated");
		}
	}

	float w = float(obj->surface->w);
	float h = float(obj->surface->h);

	float texMinX = float(obj->srcRect.x) / w;
	float texMinY = float(obj->srcRect.y) / h;
	float texMaxX = float(obj->srcRect.x + obj->srcRect.w) / w;
	float texMaxY = float(obj->srcRect.y + obj->srcRect.h) / h;

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

	if (angle % 360) {
		glPopMatrix();
		if (check) {
			checkErrors("glPopMatrix");
		}
	}
}

static const size_t MAX_RENDER_GROUP_SIZE = 1024;
template<bool check>
static void fastRenderGroup(const RenderStruct *startObj, size_t count) {
	float texBuffer[MAX_RENDER_GROUP_SIZE * 8];
	int vertexBuffer[MAX_RENDER_GROUP_SIZE * 8];

	float *texPtr = texBuffer;
	int *vertexPtr = vertexBuffer;

	float w = float(startObj->surface->w);
	float h = float(startObj->surface->h);

	const RenderStruct *obj = startObj;
	const RenderStruct *endObj = startObj + count;
	while (obj != endObj) {
		float texMinX = float(obj->srcRect.x) / w;
		float texMinY = float(obj->srcRect.y) / h;
		float texMaxX = float(obj->srcRect.x + obj->srcRect.w) / w;
		float texMaxY = float(obj->srcRect.y + obj->srcRect.h) / h;

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

		int angle = int(obj->angle) % 360;
		if (angle) {
			const float sinA = Math::getSin(angle);
			const float cosA = Math::getCos(angle);
			const SDL_Point center = { vertMinX + obj->center.x, vertMinY + obj->center.y };

			auto rotate = [&](SDL_Point *a) {
				float x = float(a->x - center.x);
				float y = float(a->y - center.y);

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
	if (check) {
		checkErrors("glEnableClientState(GL_VERTEX_ARRAY)");
	}
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (check) {
		checkErrors("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");
	}

	glTexCoordPointer(2, GL_FLOAT, 0, texBuffer);
	if (check) {
		checkErrors("glTexCoordPointer");
	}
	glVertexPointer(2, GL_INT, 0, vertexBuffer);
	if (check) {
		checkErrors("glVertexPointer");
	}

	glDrawArrays(GL_QUADS, 0, int(count * 4));
	if (check) {
		checkErrors("glDrawArrays");
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	if (check) {
		checkErrors("glDisableClientState(GL_VERTEX_ARRAY)");
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (check) {
		checkErrors("glDisableClientState(GL_TEXTURE_COORD_ARRAY)");
	}
}

static void fastRender(const RenderStruct *obj, size_t count) {
	glColor4f(1, 1, 1, GLfloat(obj->alpha) / 255);
	checkErrors("glColor4f");
	blendColor(1, 1, 1, GLfloat(obj->alpha) / 255);
	checkErrors("glBlendColor");

	if (count < 7) {
		for (size_t i = 0; i < count; ++i) {
			if (checkOpenGlErrors) {
				fastRenderOne<true>(obj + i);
			}else {
				fastRenderOne<false>(obj + i);
			}
		}
	}else {
		if (checkOpenGlErrors) {
			fastRenderGroup<true>(obj, count);
		}else {
			fastRenderGroup<false>(obj, count);
		}
	}
}

static void updateViewport(int w, int h) {
	if (fastOpenGL) {
		glViewport(0, 0, w, h);
		checkErrors("glViewport");
		glMatrixMode(GL_PROJECTION);
		checkErrors("glMatrixMode");
		glLoadIdentity();
		glOrtho(0, w, h, 0, 0.0, 1.0);
		checkErrors("glOrtho");
		glMatrixMode(GL_MODELVIEW);
		checkErrors("glMatrixMode");
		glLoadIdentity();
	}

	if (SDL_RenderSetViewport(renderer, nullptr)) {
		Utils::outMsg("SDL_RenderSetViewport", SDL_GetError());
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
		Utils::sleep(0.001);
	}
	toScaleSurface = nullptr;

	return scaledSurface;
}
static void scale() {
	if (scaleWidth > textureWidth || scaleHeight > textureHeight) {
		if (tmpTexture) {
			SDL_DestroyTexture(tmpTexture);
		}

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
	if (fastOpenGL) {
		updateViewport(dstRect.w, dstRect.h);
		Renderer::needToUpdateViewPort = true;

		glEnable(GL_BLEND);
		checkErrors("glEnable(GL_BLEND)");
		glDisable(GL_SCISSOR_TEST);
		checkErrors("glDisable(GL_SCISSOR_TEST)");

		glColor4f(1, 1, 1, 1);
		checkErrors("glColor4f");
		blendColor(1, 1, 1, 1);
		checkErrors("glBlendColor");

		bindTexture(toRender.get(), surfaceIsOpaque(toScaleSurface));

		glBegin(GL_QUADS);
		//inverse Y, because render to texture in opengl
		glTexCoord2f(0, 0); glVertex2i(0, dstRect.h);
		glTexCoord2f(0, 1); glVertex2i(0, 0);
		glTexCoord2f(1, 1); glVertex2i(dstRect.w, 0);
		glTexCoord2f(1, 0); glVertex2i(dstRect.w, dstRect.h);
		glEnd();
		checkErrors("glBegin(GL_QUADS)/glTexCoord2f/glVertex2i/glEnd");

		unbindTexture();
		glFlush();
	}else {
		if (SDL_RenderCopy(renderer, toRender.get(), nullptr, &dstRect)) {
			Utils::outMsg("Renderer::scale, SDL_RenderCopy", SDL_GetError());
		}
	}

	scaledSurface = ImageManipulator::getNewNotClear(scaleWidth, scaleHeight);
	if (SDL_RenderReadPixels(renderer, &dstRect, SDL_PIXELFORMAT_RGBA32, scaledSurface->pixels, scaledSurface->pitch)) {
		Utils::outMsg("Renderer::scale, SDL_RenderReadPixels", SDL_GetError());
	}

	if (SDL_SetRenderTarget(renderer, nullptr)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget(nullptr)", SDL_GetError());
	}

	scaled = true;
}



static SurfacePtr screenshot;
static bool screenshoted = true;

void Renderer::needMakeScreenshot() {
	screenshoted = false;
	needToRender = needToRedraw = true;
}
SurfacePtr Renderer::getScreenshot() {
	while (!screenshoted) {
		Utils::sleep(0.001);
	}
	return screenshot;
}

static void readPixels() {
	SDL_Rect rect = { Stage::x, Stage::y, Stage::width, Stage::height };

	screenshot = ImageManipulator::getNewNotClear(rect.w, rect.h, SDL_PIXELFORMAT_RGB24);
	if (SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_RGB24, screenshot->pixels, screenshot->pitch)) {
		Utils::outMsg("SDL_RenderReadPixels", SDL_GetError());
	}
}


static void loop() {
	if (fastOpenGL) {
		checkOpenGlErrors = Config::get("check_fast_opengl_errors") == "True";
	}

	static std::vector<RenderStruct> curRenderData;
	static std::vector<RenderStruct> prevRenderData;

	static std::vector<TexturePtr> textures;
	static std::vector<TexturePtr> prevTextures;


	while (!GV::exit) {
		if (!scaled) {
			scale();
		}

		if (screenshoted && (!Renderer::needToRender || Stage::minimized)) {
			Utils::sleep(0.001, false);
			continue;
		}


		{
			std::lock_guard g(Renderer::renderDataMutex);
			curRenderData.clear();
			Renderer::renderData.swap(curRenderData);
		}
		Renderer::needToRender = false;

		if (!Renderer::needToRedraw && curRenderData == prevRenderData) continue;
		Renderer::needToRedraw = false;


		Renderer::renderMutex.lock();
		ScopeExit se([]() {
			Renderer::renderMutex.unlock();

			if (GV::inGame) {
				//pause between loop iterations for no long locks
				// when rendering takes the whole frame time (on software renderer, for example)
				//with long locks - (keyboard) event processing is bad
				Utils::sleep(0.001, false);
			}
		});

		{
			std::map<SurfacePtr, TexturePtr> cache;
			textures.clear();
			textures.reserve(curRenderData.size());

			for (const RenderStruct &rs : curRenderData) {
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
		}

		SDL_GetWindowSize(Stage::window, &windowWidth, &windowHeight);
		if (Renderer::needToUpdateViewPort) {
			Renderer::needToUpdateViewPort = false;
			updateViewport(windowWidth, windowHeight);
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
			SDL_Rect prevClipRect = { 0, 0, 0, 0 };

			size_t count = 0;
			for (size_t i = 0; i < textures.size(); ++i) {
				if (!GV::inGame) break;

				const RenderStruct &rs = curRenderData[i];
				SDL_Texture *texture = textures[i].get();

				if (prevTexture != texture || prevAlpha != rs.alpha ||
				    prevClip != rs.clip || prevClipRect != rs.clipRect || count == MAX_RENDER_GROUP_SIZE)
				{
					setClipRect(prevClip ? &prevClipRect : nullptr);
					bindTexture(prevTexture, prevIsOpaque);
					fastRender(curRenderData.data() + start, count);

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
			if (prevTexture) {
				setClipRect(prevClip ? &prevClipRect : nullptr);
				bindTexture(prevTexture, prevIsOpaque);
				fastRender(curRenderData.data() + start, count);
			}
			unbindTexture();
		}else {
			for (size_t i = 0; i < textures.size(); ++i) {
				if (!GV::inGame) break;

				const RenderStruct &rs = curRenderData[i];
				const TexturePtr &texture = textures[i];

				setClipRect(rs.clip ? &rs.clipRect : nullptr);

				if (SDL_SetTextureAlphaMod(texture.get(), rs.alpha)) {
					Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
				}

				if (SDL_RenderCopyEx(renderer, texture.get(),
				                     &rs.srcRect,
				                     &rs.dstRect,
				                     double(rs.angle),
				                     &rs.center,
				                     SDL_FLIP_NONE))
				{
					Utils::outMsg("SDL_RenderCopyEx", SDL_GetError());
				}
			}
		}

		if (Stage::screens) {
			SDL_Rect empties[2];
			if (Stage::x) {
				empties[0] = { 0, 0, Stage::x, Stage::height };
				empties[1] = { Stage::x + Stage::width, 0, windowWidth - Stage::x - Stage::width, Stage::height };
			}else {
				empties[0] = { 0, 0, Stage::width, Stage::y };
				empties[1] = { 0, Stage::y + Stage::height, Stage::width, windowHeight - Stage::y - Stage::height };
			}

			disableClipRect();
			for (size_t i = 0; i < 2; ++i) {
				const SDL_Rect &empty = empties[i];
				if (!empty.w || !empty.h) continue;

				if (fastOpenGL) {
					glColor4f(0, 0, 0, 1);
					checkErrors("glColor4f");
					glRecti(empty.x, empty.y, empty.x + empty.w, empty.y + empty.h);
					checkErrors("glRecti");
				}else {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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

		prevRenderData.swap(curRenderData);
		prevTextures.swap(textures);
	}

	SDL_DestroyRenderer(renderer);
}


static void initImpl(bool *inited, bool *error) {
	Utils::setThreadName("renderer");

	Uint32 flags;
	if (Config::get("software_renderer") == "True") {
		flags = SDL_RENDERER_SOFTWARE;
	}else {
		flags = SDL_RENDERER_ACCELERATED;
		if (Config::get("opengl_vsync") == "True") {
			flags |= SDL_RENDERER_PRESENTVSYNC;
		}

		std::string opengl = "opengl";
		int renderDriver = -1;
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
			flags = SDL_RENDERER_SOFTWARE;
		}else {
			fastOpenGL = Config::get("fast_opengl") == "True";
			if (fastOpenGL) {
				SDL_SetHint(SDL_HINT_RENDER_BATCHING, "false");
			}
		}
	}

	renderer = SDL_CreateRenderer(Stage::window, -1, flags);
	if (!renderer) {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());

		if (flags & SDL_RENDERER_ACCELERATED) {
			renderer = SDL_CreateRenderer(Stage::window, -1, SDL_RENDERER_SOFTWARE);
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
	if (Renderer::info.max_texture_width <= 0)  Renderer::info.max_texture_width  = 2048;
	if (Renderer::info.max_texture_height <= 0) Renderer::info.max_texture_height = 2048;

	if (std::string(Renderer::info.name) == "opengl") {
		size_t countErrors = 0;
		const size_t maxCountErrors = 10;

		while (++countErrors < maxCountErrors && glGetError() != GL_NO_ERROR) {}
		if (countErrors == maxCountErrors) {
			Utils::outMsg("Renderer::init", "Using OpenGL failed");
			fastOpenGL = false;
		}
	}else {
		fastOpenGL = false;
	}
	if (fastOpenGL) {
		blendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)SDL_GL_GetProcAddress("glBlendFuncSeparate");
		blendColor = (BLENDCOLOR_TYPE)SDL_GL_GetProcAddress("glBlendColor");
		if (!blendFuncSeparate || !blendColor) {
			Utils::outMsg("Renderer::init", "glBlendFuncSeparate or glBlendColor not found");
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
		while (Utils::realOutMsg()) {}
		Utils::sleep(0.001, false);
	}

	return error;
}
