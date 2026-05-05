#include "renderer.h"

#include <map>
#include <thread>

#define USE_OPENGL32
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL.h>

#include "gv.h"
#include "config.h"

#include "utils/btn_rect.h"
#include "utils/math.h"
#include "utils/image_caches.h"
#include "utils/scope_exit.h"
#include "utils/stage.h"
#include "utils/utils.h"


//not static, also used in "gui/text_field.cpp"
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
static
bool operator!=(const SDL_Point &a, const SDL_Point &b) {
	return !(a == b);
}

[[maybe_unused]]//USED in std::vector<RenderStruct>::operator==, need to remove clang warning
static
bool operator==(const RenderStruct &a, const RenderStruct &b) {
	auto at = std::tie(a.surface, a.angle, a.alpha, a.clip, a.clipRect, a.srcRect, a.dstRect, a.center);
	auto bt = std::tie(b.surface, b.angle, b.alpha, b.clip, b.clipRect, b.srcRect, b.dstRect, b.center);
	return at == bt;
}



static int windowWidth, windowHeight;
static SDL_Renderer *renderer = nullptr;
static PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = nullptr;

bool Renderer::needToRender = false;
bool Renderer::needToRedraw = false;
bool Renderer::needToUpdateViewPort = true;

std::mutex Renderer::renderMutex;

std::mutex Renderer::renderDataMutex;
std::vector<RenderStruct> Renderer::renderData;

std::string Renderer::driver = "software";
int Renderer::maxSize = -1;


static bool fastOpenGL = false;
static bool checkOpenGlErrors = false;

bool Renderer::useFastOpenGL() {
	return fastOpenGL;
}


static void checkErrorsImpl(const char *from, const char *glFuncName) {
	GLuint error = glGetError();
	if (error == GL_NO_ERROR) return;

	bool mutexWasLocked = !Renderer::renderMutex.try_lock();
	Renderer::renderMutex.unlock();

	fastOpenGL = false;
	std::string fromStr = Utils::format("Renderer::%, %", from, glFuncName);
	Utils::outMsg(fromStr, "Using OpenGL failed");
	ImageCaches::clearTextures();

	const char *str = "Unknown";
	     if (error == GL_INVALID_ENUM)      str = "GL_INVALID_ENUM";
	else if (error == GL_INVALID_VALUE)     str = "GL_INVALID_VALUE";
	else if (error == GL_INVALID_OPERATION) str = "GL_INVALID_OPERATION";
	else if (error == GL_STACK_OVERFLOW)    str = "GL_STACK_OVERFLOW";
	else if (error == GL_STACK_UNDERFLOW)   str = "GL_STACK_UNDERFLOW";
	else if (error == GL_OUT_OF_MEMORY)     str = "GL_OUT_OF_MEMORY";

	Utils::outMsg(fromStr, str);

	if (mutexWasLocked) {
		Renderer::renderMutex.lock();
	}
}
#define checkErrors(glFuncName) checkErrorsImpl(__FUNCTION__, glFuncName)


static SDL_Texture *currentTexture = nullptr;
static bool currentTextureIsOpaque = false;
static void bindTexture(SDL_Texture *texture, bool opaque) {
	if (!texture) return;

	if (currentTexture != texture) {
		if (!currentTexture) currentTextureIsOpaque = !opaque;
		currentTexture = texture;

		SDL_FlushRenderer(renderer);

		SDL_PropertiesID props = SDL_GetTextureProperties(texture);
		Sint64 id = SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER, 0);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, GLuint(id));
		checkErrors("glBindTexture(GL_TEXTURE_2D, id)");
	}

	if (currentTextureIsOpaque != opaque) {
		currentTextureIsOpaque = opaque;

		if (opaque) {
			glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}else {
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		}
		checkErrors("glBlendFuncSeparate");
	}

	SDL_ScaleMode mode;
	SDL_GetTextureScaleMode(texture, &mode);

	GLint value = mode == SDL_SCALEMODE_NEAREST ? GL_NEAREST : GL_LINEAR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);
}
static void unbindTexture() {
	glBindTexture(GL_TEXTURE_2D, 0);
	checkErrors("glBindTexture(GL_TEXTURE_2D, 0)");

	glDisable(GL_TEXTURE_2D);

	SDL_FlushRenderer(renderer);
	currentTexture = nullptr;
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
		glEnable(GL_SCISSOR_TEST);
		if (checkOpenGlErrors) {
			checkErrors("glDisable(GL_SCISSOR_TEST)");
		}

		glScissor(rect.x, windowHeight - rect.y - rect.h, rect.w, rect.h);//at glScissor lower pixel - y=0
		if (checkOpenGlErrors) {
			checkErrors("glScissor");
		}
	}else {
		if (!SDL_SetRenderClipRect(renderer, &rect)) {
			Utils::outMsg("SDL_SetRenderClipRect", SDL_GetError());
		}
	}
}
static void disableClipRect() {
	if (fastOpenGL) {
		glDisable(GL_SCISSOR_TEST);
		checkErrors("glDisable(GL_SCISSOR_TEST)");
	}else {
		if (!SDL_SetRenderClipRect(renderer, nullptr)) {
			Utils::outMsg("SDL_SetRenderClipRect", SDL_GetError());
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
		if constexpr (check) {
			checkErrors("glPushMatrix");
		}
		glTranslated(dX, dY, 0);
		if constexpr (check) {
			checkErrors("glTranslated");
		}
		glRotatef(obj->angle, 0, 0, 1);
		if constexpr (check) {
			checkErrors("glRotated");
		}
		glTranslated(-dX, -dY, 0);
		if constexpr (check) {
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
	if constexpr (check) {
		checkErrors("glBegin(GL_QUADS)/glTexCoord2f/glVertex2i/glEnd");
	}

	if (angle % 360) {
		glPopMatrix();
		if constexpr (check) {
			checkErrors("glPopMatrix");
		}
	}
}

static const size_t MAX_RENDER_GROUP_SIZE = 1024;
template<bool check>
static void fastRenderGroup(const RenderStruct *startObj, size_t count) {
	SDL_FPoint texBuffer[MAX_RENDER_GROUP_SIZE * 4];
	SDL_Point vertexBuffer[MAX_RENDER_GROUP_SIZE * 4];

	SDL_FPoint *texPtr = texBuffer;
	SDL_Point *vertexPtr = vertexBuffer;

	float w = float(startObj->surface->w);
	float h = float(startObj->surface->h);

	const RenderStruct *obj = startObj;
	const RenderStruct *endObj = startObj + count;
	while (obj != endObj) {
		float texMinX = float(obj->srcRect.x) / w;
		float texMinY = float(obj->srcRect.y) / h;
		float texMaxX = float(obj->srcRect.x + obj->srcRect.w) / w;
		float texMaxY = float(obj->srcRect.y + obj->srcRect.h) / h;

		texPtr[0] = { texMinX, texMinY };
		texPtr[1] = { texMinX, texMaxY };
		texPtr[2] = { texMaxX, texMaxY };
		texPtr[3] = { texMaxX, texMinY };
		texPtr += 4;

		int vertMinX = obj->dstRect.x;
		int vertMinY = obj->dstRect.y;
		int vertMaxX = obj->dstRect.x + obj->dstRect.w;
		int vertMaxY = obj->dstRect.y + obj->dstRect.h;

		vertexPtr[0] = { vertMinX, vertMinY };
		vertexPtr[1] = { vertMinX, vertMaxY };
		vertexPtr[2] = { vertMaxX, vertMaxY };
		vertexPtr[3] = { vertMaxX, vertMinY };

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
			rotate(vertexPtr + 0);
			rotate(vertexPtr + 1);
			rotate(vertexPtr + 2);
			rotate(vertexPtr + 3);
		}
		vertexPtr += 4;

		++obj;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	if constexpr (check) {
		checkErrors("glEnableClientState(GL_VERTEX_ARRAY)");
	}
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if constexpr (check) {
		checkErrors("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");
	}

	glTexCoordPointer(2, GL_FLOAT, 0, texBuffer);
	if constexpr (check) {
		checkErrors("glTexCoordPointer");
	}
	glVertexPointer(2, GL_INT, 0, vertexBuffer);
	if constexpr (check) {
		checkErrors("glVertexPointer");
	}

	glDrawArrays(GL_QUADS, 0, int(count * 4));
	if constexpr (check) {
		checkErrors("glDrawArrays");
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	if constexpr (check) {
		checkErrors("glDisableClientState(GL_VERTEX_ARRAY)");
	}
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if constexpr (check) {
		checkErrors("glDisableClientState(GL_TEXTURE_COORD_ARRAY)");
	}
}

static void fastRender(const RenderStruct *obj, size_t count) {
	glColor4f(1, 1, 1, GLfloat(obj->alpha) / 255);
	checkErrors("glColor4f");
//	glBlendColor(1, 1, 1, obj->alpha / 255.0f);
//	checkErrors("glBlendColor");

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

	if (!SDL_SetRenderViewport(renderer, nullptr)) {
		Utils::outMsg("SDL_SetRenderViewport", SDL_GetError());
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

	if (!SDL_SetRenderTarget(renderer, tmpTexture)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget", SDL_GetError());
	}
	if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderDrawColor", SDL_GetError());
	}
	if (!SDL_RenderClear(renderer)) {
		Utils::outMsg("Renderer::scale, SDL_RenderClear", SDL_GetError());
	}
	if (fastOpenGL) {
		updateViewport(dstRect.w, dstRect.h);
		SDL_FlushRenderer(renderer);
		Renderer::needToUpdateViewPort = true;

		glEnable(GL_BLEND);
		checkErrors("glEnable(GL_BLEND)");
		glDisable(GL_SCISSOR_TEST);
		checkErrors("glDisable(GL_SCISSOR_TEST)");

		glColor4f(1, 1, 1, 1);
		checkErrors("glColor4f");

		bindTexture(toRender.get(), ImageCaches::surfaceIsOpaque(toScaleSurface));

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
		SDL_FRect dstFRect;
		SDL_RectToFRect(&dstRect, &dstFRect);

		if (!SDL_RenderTexture(renderer, toRender.get(), nullptr, &dstFRect)) {
			Utils::outMsg("Renderer::scale, SDL_RenderTexture", SDL_GetError());
		}
	}

	scaledSurface = SDL_RenderReadPixels(renderer, &dstRect);
	if (!scaledSurface) {
		Utils::outMsg("Renderer::scale, SDL_RenderReadPixels", SDL_GetError());
	}

	if (!SDL_SetRenderTarget(renderer, nullptr)) {
		Utils::outMsg("Renderer::scale, SDL_SetRenderTarget(nullptr)", SDL_GetError());
	}

	scaled = true;
}



static const SDL_Point NULL_POINT = { 0, 0 };
static std::array<SDL_Point, 4> selectedRect = { NULL_POINT, NULL_POINT, NULL_POINT, NULL_POINT };
void Renderer::updateSelectedRect() {
	selectedRect = BtnRect::getPointsOfSelectedRect();
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

	screenshot = SDL_RenderReadPixels(renderer, &rect);
	if (!screenshot) {
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

	std::array<SDL_Point, 4> prevSelectedRect = selectedRect;

	while (!GV::exit) {
		if (!scaled) {
			scale();
		}

		if (screenshoted && (!Renderer::needToRender || Stage::minimized)) {
			Utils::sleep(0.001, false);
			continue;
		}


		curRenderData.clear();
		{
			std::lock_guard g(Renderer::renderDataMutex);
			Renderer::renderData.swap(curRenderData);
		}
		Renderer::needToRender = false;

		if (!Renderer::needToRedraw && curRenderData == prevRenderData && selectedRect == prevSelectedRect) continue;
		Renderer::needToRedraw = false;

		prevSelectedRect = selectedRect;


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

		if (Renderer::needToUpdateViewPort) {
			Renderer::needToUpdateViewPort = false;
			SDL_GetWindowSize(Stage::window, &windowWidth, &windowHeight);
			updateViewport(windowWidth, windowHeight);
		}
		if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
			Utils::outMsg("SDL_SetRenderDrawColor", SDL_GetError());
		}
		if (!SDL_RenderClear(renderer)) {
			Utils::outMsg("SDL_RenderClear", SDL_GetError());
		}
		SDL_FlushRenderer(renderer);

		if (fastOpenGL && !textures.empty()) {
			glEnable(GL_BLEND);
			checkErrors("glEnable(GL_BLEND)");

			size_t start = 0;
			SDL_Texture *prevTexture = nullptr;
			bool prevIsOpaque = false;
			Uint8 prevAlpha = 0;
			bool prevClip = false;
			SDL_Rect prevClipRect = { 0, 0, 0, 0 };

			size_t count = 0;
			for (size_t i = 0; i < textures.size(); ++i) {
				if (GV::exit) break;

				const RenderStruct &rs = curRenderData[i];
				SDL_Texture *texture = textures[i].get();

				if (prevTexture != texture || prevAlpha != rs.alpha ||
				    prevClip != rs.clip || prevClipRect != rs.clipRect || count == MAX_RENDER_GROUP_SIZE)
				{
					setClipRect(prevClip ? &prevClipRect : nullptr);
					bindTexture(prevTexture, prevIsOpaque);
					fastRender(curRenderData.data() + start, count);

					prevTexture = texture;
					prevIsOpaque = ImageCaches::surfaceIsOpaque(rs.surface);
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
				if (GV::exit) break;

				const RenderStruct &rs = curRenderData[i];
				const TexturePtr &texture = textures[i];

				setClipRect(rs.clip ? &rs.clipRect : nullptr);

				if (!SDL_SetTextureAlphaMod(texture.get(), rs.alpha)) {
					Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
				}

				SDL_FRect srcRect, dstRect;
				SDL_RectToFRect(&rs.srcRect, &srcRect);
				SDL_RectToFRect(&rs.dstRect, &dstRect);

				SDL_FPoint center = {
				    float(rs.center.x),
				    float(rs.center.y),
				};

				if (!SDL_RenderTextureRotated(renderer, texture.get(),
				                              &srcRect,
				                              &dstRect,
				                              double(rs.angle),
				                              &center,
				                              SDL_FLIP_NONE))
				{
					Utils::outMsg("SDL_RenderTextureRotated", SDL_GetError());
				}
			}
		}
		disableClipRect();

		bool haveSelectedRect = selectedRect[0] != selectedRect[1];
		if (haveSelectedRect) {
			if (fastOpenGL) {
				glColor4f(0, 0.25, 1, 1);
			}else {
				if (!SDL_SetRenderDrawColor(renderer, 0, 64, 255, 255)) {
					Utils::outMsg("SDL_SetRenderDrawColor", SDL_GetError());
				}
			}

			for (size_t i = 0; i < 4; ++i) {
				SDL_Point start = selectedRect[i];
				SDL_Point end = selectedRect[(i + 1) % 4];

				if (fastOpenGL) {
					glBegin(GL_LINES);
					glVertex2i(start.x, start.y);
					glVertex2i(end.x, end.y);
					glEnd();
				}else {
					if (!SDL_RenderLine(renderer, float(start.x), float(start.y), float(end.x), float(end.y))) {
						Utils::outMsg("SDL_RenderDrawLine", SDL_GetError());
					}
				}
			}

			if (fastOpenGL) {
				checkErrors("drawing select rect");
			}
		}

		if (Stage::screens) {
			SDL_FRect empties[2];
			if (Stage::x) {
				empties[0] = { 0, 0, float(Stage::x), float(Stage::height) };
				float right = float(Stage::x + Stage::width);
				empties[1] = { right, 0, float(windowWidth) - right, float(Stage::height) };
			}else {
				empties[0] = { 0, 0, float(Stage::width), float(Stage::y) };
				float bottom = float(Stage::y + Stage::height);
				empties[1] = { 0, bottom, float(Stage::width), float(windowHeight) - bottom };
			}

			for (const SDL_FRect &empty : empties) {
				if (Math::floatsAreEq(empty.w, 0)) continue;
				if (Math::floatsAreEq(empty.h, 0)) continue;

				if (fastOpenGL) {
					glColor4f(0, 0, 0, 1);
					checkErrors("glColor4f");
					glRectf(empty.x, empty.y, empty.x + empty.w, empty.y + empty.h);
					checkErrors("glRecti");
				}else {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					if (!SDL_RenderFillRect(renderer, &empty)) {
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

		if (fastOpenGL) {
			checkErrors("End");
		}

		prevRenderData.swap(curRenderData);
		prevTextures.swap(textures);
	}

	SDL_DestroyRenderer(renderer);
}


static void initImpl(bool *inited, bool *error) {
	Utils::setThreadName("renderer");

	std::string opengl = "opengl";

	if (Config::get("software_renderer") != "True") {
		int countRenderDrivers = SDL_GetNumRenderDrivers();
		for (int i = 0; i < countRenderDrivers; ++i) {
			const char* name = SDL_GetRenderDriver(i);
			if (name == opengl) {
				Renderer::driver = opengl;
				break;
			}
		}
		if (Renderer::driver != opengl) {
			Utils::outMsg("Renderer::init", "OpenGL driver not found");
		}
	}
	
	renderer = SDL_CreateRenderer(Stage::window, Renderer::driver.c_str());
	if (renderer) {
		fastOpenGL = Config::get("fast_opengl") == "True";
		if (fastOpenGL) {
			glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)SDL_GL_GetProcAddress("glBlendFuncSeparate");
			if (!glBlendFuncSeparate) {
				fastOpenGL = false;
				Utils::outMsg("Renderer::init", "glBlendFuncSeparate not found, fast_opengl disabled");
			}
		}
	}else {
		Utils::outMsg("SDL_CreateRenderer", SDL_GetError());

		if (Renderer::driver == opengl) {
			Renderer::driver = "software";
			renderer = SDL_CreateRenderer(Stage::window, Renderer::driver.c_str());
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

	SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
	Renderer::maxSize = int(SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0));
	if (Renderer::maxSize <= 0) Renderer::maxSize = 2048;

	if (Renderer::driver == opengl) {
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

	if (Config::get("opengl_vsync") == "True") {
		SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
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
