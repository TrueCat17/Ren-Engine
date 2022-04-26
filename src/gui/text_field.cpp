#include "text_field.h"

#include <map>

#include <SDL2/SDL_ttf.h>

#include "renderer.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/file_system.h"
#include "utils/math.h"
#include "utils/string.h"
#include "utils/utils.h"


bool operator==(const SDL_Rect &a, const SDL_Rect &b);


static const int MIN_TEXT_SIZE = 8;
static const int MAX_TEXT_SIZE = 72;
static const std::string defaultFontName = "Arial";

static std::map<std::string, TTF_Font*> fonts;
static TTF_Font* getFont(const std::string &name, int size) {
	size = Math::inBounds(size, MIN_TEXT_SIZE, MAX_TEXT_SIZE);
	const std::string t = name + "|" + std::to_string(size);

	auto it = fonts.find(t);
	if (it != fonts.end()) {
		return it->second;
	}

	std::string startName = "fonts/" + name + '.';
	std::string dir = FileSystem::getParentDirectory(startName);
	std::vector<std::string> files = FileSystem::getFiles(dir);

	TTF_Font *res;
	for (const std::string &file : files) {
		if (!String::startsWith(file, startName)) continue;
		if (file.find('.', startName.size()) != size_t(-1)) continue;

		res = TTF_OpenFont(file.c_str(), size);
		if (res) {
			return fonts[t] = res;
		}
		SDL_ClearError();
	}

	if (name != defaultFontName) {
		Utils::outMsg("getFont", "Failed to load font <" + name + ">.\nTry to load default font.");
		res = getFont(defaultFontName, size);
	}else {
		Utils::outMsg("getFont", "Failed to load default font <" + defaultFontName + ">.\nText will not be displayed.");
		res = nullptr;
	}
	return fonts[t] = res;
}

static void addChars(const std::string &str, int x, int *w, int *h, TextStyle &style, SDL_Surface *surface) {
	if (!style.font) return;

	int maxH = *h;
	if (TTF_SizeUTF8(style.font, str.c_str(), w, h)) {
		Utils::outMsg("TTF_SizeUTF8", TTF_GetError());
		return;
	}
	if (style.alpha * 255 < 1) return;

	SDL_Rect rect = {x, std::max(0, maxH - *h), *w, *h};

	SurfacePtr tmpSurface;
	if (style.enableOutline) {
		//masks from TTF_RenderUTF8_Blended
		tmpSurface.reset(SDL_CreateRGBSurface(0, *w, *h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000), SDL_FreeSurface);

		SDL_Color outlineColor;
		outlineColor.r = Uint8(style.outlineColor >> 16);
		outlineColor.g = (style.outlineColor >> 8) & 0xFF;
		outlineColor.b = style.outlineColor & 0xFF;
		outlineColor.a = 255;

		SDL_Surface *surfaceOutlineText = TTF_RenderUTF8_Blended(style.font, str.c_str(), outlineColor);
		if (!surfaceOutlineText) {
			Utils::outMsg("TTF_RenderUTF8_Blended", TTF_GetError());
			return;
		}

		int countOutlines = style.fontSize >= 25 ? 2 : 1;

		std::pair<int, int> pairs[8] = {{-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}};
		for (auto [x, y] : pairs) {
			for (int i = 1; i <= countOutlines; ++i) {
				SDL_Rect rect = {x * i, y * i, *w, *h};
				SDL_BlitSurface(surfaceOutlineText, nullptr, tmpSurface.get(), &rect);
			}
		}
		SDL_FreeSurface(surfaceOutlineText);
	}

	SDL_Color color;
	color.r = Uint8(style.color >> 16);
	color.g = (style.color >> 8) & 0xFF;
	color.b = style.color & 0xFF;
	color.a = 255;

	SDL_Surface *surfaceText = TTF_RenderUTF8_Blended(style.font, str.c_str(), color);
	if (!surfaceText) {
		Utils::outMsg("TTF_RenderUTF8_Blended", TTF_GetError());
		return;
	}

	if (style.enableOutline) {
		SDL_BlitSurface(surfaceText, nullptr, tmpSurface.get(), nullptr);
		SDL_SetSurfaceAlphaMod(tmpSurface.get(), Uint8(std::min<float>(style.alpha * 255, 255)));
		SDL_BlitSurface(tmpSurface.get(), nullptr, surface, &rect);
	}else {
		SDL_SetSurfaceAlphaMod(surfaceText, Uint8(std::min<float>(style.alpha * 255, 255)));
		SDL_BlitSurface(surfaceText, nullptr, surface, &rect);
	}
	SDL_FreeSurface(surfaceText);
}

static std::string getTagValue(std::string &tag) {
	size_t i = tag.find('=');
	if (i != size_t(-1)) {
		std::string value = tag.substr(i + 1);
		tag.erase(i);
		return value;
	}
	return "";
}
static SurfacePtr getImage(const std::string &path) {
	if (Utils::imageWasRegistered(path)) {
		std::vector<std::string> commands = Utils::getVectorImageDeclAt(path);
		if (commands.empty()) return nullptr;

		std::string realPath = PyUtils::exec("CPP_EMBED: text_field.cpp", __LINE__, commands.front(), true);
		return ImageManipulator::getImage(realPath);
	}

	return ImageManipulator::getImage(path);
}
static Uint32 getColor(std::string value) {
	if (String::startsWith(value, "#")) {
		value.erase(0, 1);
	}else
	if (String::startsWith(value, "0x")) {
		value.erase(0, 2);
	}

	if (value.size() > 6) {
		Utils::outMsg("TextField::getColor", "Expected color (format RRGGBB), got <" + value + ">");
		value.erase(6);
	}

	bool invalid = false;
	Uint32 res = 0;
	for (char c : value) {
		char n;

		if (c >= '0' && c <= '9') {
			n = c - '0';
		}else
		if (c >= 'a' && c <= 'f') {
			n = c - 'a' + 10;
		}else
		if (c >= 'A' && c <= 'F') {
			n = c - 'A' + 10;
		}else {
			n = 0;
			invalid = true;
		}

		res = res * 16 + Uint32(n);
	}
	if (invalid) {
		Utils::outMsg("TextField::getColor", "Color <" + value + "> is invalid");
	}
	return res;
}
static void updateStyle(TextStyle &textStyle) {
	TTF_SetFontStyle(textStyle.font, textStyle.fontStyle);
}
static void updateFont(TextStyle &textStyle) {
	textStyle.font = getFont(textStyle.fontName, int(round(textStyle.fontSize)));
}

static bool invisible;
static size_t makeStep(const std::string &line, size_t i, std::vector<TextStyle> &stackStyles, int x, int *w, int *h, SDL_Surface *surface) {
	bool onlySize = !surface;

	bool isStyle = line[i] == '{';
	if (isStyle && i + 1 < line.size() && line[i + 1] == '{') {
		++i;
		isStyle = false;
	}


	if (isStyle) {
		int maxH = *h;
		*w = *h = 0;

		size_t start = line.find_first_not_of(' ', i + 1);
		size_t end = start;
		while (end < line.size() && line[end] != '}') {
			++end;
		}
		if (start >= end || end == line.size()) {
			if (!onlySize) {
				Utils::outMsg("TextField::makeStep", "Incomplete tag");
			}
			return line.size();
		}

		i = end;
		bool closeTag = line[start] == '/';
		if (closeTag) ++start;

		end = line.find_last_not_of(' ', end);
		std::string tag = line.substr(start, end - start);

		if (closeTag) {
			if (tag == stackStyles.back().tag) {
				stackStyles.pop_back();
				updateStyle(stackStyles.back());
			}else {
				if (!onlySize) {
					Utils::outMsg("TextField::makeStep", "Tag <" + tag + "> != last opened tag <" + stackStyles.back().tag + ">");
				}
			}
		}else {
			TextStyle style = stackStyles.back();//copy

			std::string value = getTagValue(tag);
			if (value.empty() && (tag == "color" || tag == "outlinecolor" || tag == "alpha" || tag == "size" || tag == "font" || tag == "image")) {
				if (!onlySize) {
					Utils::outMsg("TextField::makeStep", "Tag <" + tag + "> must have value");
				}
				return i + 1;
			}
			style.tag = tag;

			auto apply = [](float &prop, std::string &value) {
				if (value[0] == '*') {
					value.erase(0, 1);
					prop *= float(String::toDouble(value));
				}else {
					if (value[0] == '+' || value[0] == '-') {
						prop += float(String::toDouble(value));
					}else {
						prop = float(String::toDouble(value));
					}
				}
			};

			bool unknownTag = false;

			if (tag == "plain") style.fontStyle = TTF_STYLE_NORMAL;
			else if (tag == "b") style.fontStyle |= TTF_STYLE_BOLD;
			else if (tag == "i") style.fontStyle |= TTF_STYLE_ITALIC;
			else if (tag == "u") style.fontStyle |= TTF_STYLE_UNDERLINE;
			else if (tag == "s") style.fontStyle |= TTF_STYLE_STRIKETHROUGH;
			else if (tag == "color") style.color = getColor(value);
			else if (tag == "outlinecolor") {
				style.outlineColor = getColor(value);
				style.enableOutline = true;
			}
			else if (tag == "alpha") apply(style.alpha,    value);
			else if (tag == "size")  apply(style.fontSize, value);
			else if (tag == "font") style.fontName = value;

			else {
				unknownTag = true;
				if (!onlySize && tag != "image" && tag != "invisible") {
					Utils::outMsg("TextField::makeStep", "Unknown tag <" + tag + ">");
				}
			}

			if (!unknownTag) {
				if (tag == "font" || tag == "size") {
					updateFont(style);
				}
				updateStyle(style);

				stackStyles.push_back(style);
			}else if (tag == "image") {
				SurfacePtr image = getImage(value);
				if (!image) {
					if (!onlySize) {
						Utils::outMsg("TextField::makeStep", "Failed to load image <" + value + ">");
					}
				}else {

					*h = int(style.fontSize);
					*w = int(style.fontSize * float(image->w) / float(image->h));
					if (!onlySize && !invisible) {
						SDL_Rect rect = {x, std::max(0, maxH - *h), *w, *h};
						SDL_SetSurfaceAlphaMod(image.get(), Uint8(Math::inBounds(style.alpha * 255, 0, 255)));
						SDL_BlitScaled(image.get(), nullptr, surface, &rect);
						SDL_SetSurfaceAlphaMod(image.get(), 255);
					}
				}
			}else if (tag == "invisible") {
				invisible = true;
			}
		}
	}else {
		size_t start = i;
		size_t end = i + 1;
		while (end < line.size()) {
			if (line[end] == '{' || line[end] == ' ') break;
			++end;
		}
		i = end - 1;
		std::string word = line.substr(start, end - start);

		if (onlySize) {
			if (TTF_SizeUTF8(stackStyles.back().font, word.c_str(), w, h)) {
				Utils::outMsg("TTF_SizeUTF8", TTF_GetError());
			}
		}else if (!invisible) {
			addChars(word, x, w, h, stackStyles.back(), surface);
		}
	}

	return i + 1;
}

void TextField::setFont(std::string fontName, float fontSize) {
	mainStyle.fontSize = fontSize;
	mainStyle.fontName = fontName.empty() ? defaultFontName : fontName;
	updateFont(mainStyle);
}

static std::vector<TextStyle> startWordStackStyles, tmpStackStyles;
void TextField::setText(const std::string &text) {
	if (!mainStyle.font) return;

	const int maxW = maxWidth  > 0 ? maxWidth  : int(1e9);
	const int maxH = maxHeight > 0 ? maxHeight : int(1e9);

	std::vector<TextStyle> stackStyles;
	stackStyles.reserve(4);
	stackStyles.push_back(mainStyle);
	updateStyle(mainStyle);

	std::vector<std::string> tmpLines = String::split(text, "\n");
	int maxLineWidth = 0;
	int currentHeight = 0;

	//calc sizes & apply wordwrap
	std::vector<SDL_Rect> tmpLineRects;
	for (size_t numLine = 0; numLine < tmpLines.size(); ++numLine) {
		std::string &line = tmpLines[numLine];

		SDL_Rect lineRect = { 0, currentHeight, 0, 0};

		int x = 0;
		int lineWidth = 0;
		int lineHeight = TTF_FontHeight(stackStyles.back().font);

		size_t startWord = 0;
		int startWordLineWidth = lineWidth;
		int startWordLineHeight = lineHeight;
		startWordStackStyles = stackStyles;

		size_t i = 0;
		while (i < line.size()) {
			if (line[i] == ' ') {
				startWord = i;
				startWordLineWidth = lineWidth;
				startWordLineHeight = lineHeight;
				startWordStackStyles = stackStyles;
			}

			int w, h;
			i = makeStep(line, i, stackStyles, x, &w, &h, nullptr);

			bool lineBreak = false;
			if (wordwrap && lineWidth + w >= maxW) {
				tmpStackStyles = stackStyles;
				size_t tmpI = i;
				if (tmpI != line.size() && line[tmpI] != ' ') {
					while (tmpI < line.size()) {
						int tmpW, tmpH;
						size_t prevTmpI = tmpI;
						tmpI = makeStep(line, tmpI, stackStyles, 0, &tmpW, &tmpH, nullptr);
						if (tmpW) {
							tmpI = prevTmpI;
							break;
						}
					}
				}
				lineBreak = tmpI == line.size() || line[tmpI] == ' ';
				stackStyles = tmpStackStyles;
			}

			if (lineBreak) {
				bool wordBreak = (lineWidth - startWordLineWidth) + w >= maxW;

				size_t startNextLine;
				if (wordBreak) {
					//binary search of string with max len < maxW:
					size_t min = startWord;
					size_t max = std::min(i, min + size_t(maxW / (MIN_TEXT_SIZE / 2)));

					w = 0;
					while (true) {
						size_t mid = (min + max) / 2;
						while (!Algo::isFirstByte(line[mid])) --mid;
						if (mid == min) break;

						std::string tmpStr = line.substr(startWord, mid - startWord);
						tmpStackStyles = startWordStackStyles;

						size_t tmpI = 0;
						int tmpW = 0;
						while (tmpI < tmpStr.size()) {
							int w, h;
							tmpI = makeStep(tmpStr, tmpI, tmpStackStyles, tmpW, &w, &h, nullptr);
							tmpW += w;
						}

						if (startWordLineWidth + tmpW >= maxW) {
							max = mid;
						}else {
							min = mid;
							w = tmpW;
						}
					}

					startNextLine = line.find_first_not_of(' ', min);

					lineWidth += w;
					lineHeight = std::max(lineHeight, h);
				}else {
					startNextLine = startWord + 1;
					lineWidth = startWordLineWidth;
					lineHeight = startWordLineHeight;
					stackStyles = startWordStackStyles;
					updateStyle(stackStyles.back());
				}

				std::string nextLine = line.substr(startNextLine);
				line.erase(startNextLine);
				tmpLines.insert(tmpLines.begin() + long(numLine + 1), nextLine);
				break;
			}

			x += w;
			lineWidth += w;
			lineHeight = std::max(lineHeight, h);
		}

		maxLineWidth = std::max(maxLineWidth, lineWidth);

		lineRect.w = lineWidth;
		lineRect.h = lineHeight;
		tmpLineRects.push_back(lineRect);

		currentHeight += lineHeight;
		if (currentHeight >= maxH) {
			tmpLineRects.back().h -= currentHeight - maxH;
			currentHeight = maxH;
			tmpLines.erase(tmpLines.begin() + long(numLine + 1), tmpLines.end());
			break;
		}
	}

	if (prevMainStyle == mainStyle && tmpLines == lines && tmpLineRects == lineRects) return;
	prevMainStyle = mainStyle;
	lines.swap(tmpLines);
	lineRects.swap(tmpLineRects);

	rect.w = float(std::max(maxWidth, maxLineWidth));
	rect.h = float(currentHeight);

	//draw with calced sizes
	stackStyles.erase(stackStyles.begin() + 1, stackStyles.end());
	updateStyle(mainStyle);
	lineSurfaces.clear();
	invisible = false;
	for (size_t numLine = 0; numLine < lines.size(); ++numLine) {
		setLine(numLine, lines[numLine], stackStyles);
	}

	setAlign(hAlign, vAlign);
}

void TextField::setLine(size_t numLine, const std::string &line, std::vector<TextStyle> &stackStyles) {
	const SDL_Rect &lineRect = lineRects[numLine];
	if (!lineRect.w) {
		lineSurfaces.push_back(nullptr);
		return;
	}

	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, lineRect.w, lineRect.h, 32, SDL_PIXELFORMAT_RGBA32);
	SurfacePtr surfacePtr(surface, SDL_FreeSurface);

	lineSurfaces.push_back(surfacePtr);
	if (!surfacePtr) return;

	size_t i = 0;
	int x = 0;
	int w, h;
	while (i < line.size()) {
		h = lineRect.h;
		i = makeStep(line, i, stackStyles, x, &w, &h, surface);
		x += w;
	}
}

void TextField::setAlign(std::string hAlign, std::string vAlign) {
	if (hAlign != "left" && hAlign != "center" && hAlign != "right") {
		Utils::outMsg("TextField::setAlign", "Unexpected hAlign: <" + hAlign + ">\n");
		hAlign = "left";
	}
	if (vAlign != "top" && vAlign != "center" && vAlign != "bottom") {
		Utils::outMsg("TextField::setAlign", "Unexpected vAlign: <" + vAlign + ">\n");
		vAlign = "top";
	}
	this->hAlign = hAlign;
	this->vAlign = vAlign;

	int height = 0;
	for (const SDL_Rect &lineRect : lineRects) {
		height += lineRect.h;
	}

	int indentY = 0;
	if (vAlign == "center") {
		indentY = (int(getHeight()) - height) / 2;
	}else if (vAlign == "bottom") {
		indentY = int(getHeight()) - height;
	}

	int maxWidth = this->maxWidth;
	for (const SDL_Rect &lineRect : lineRects) {
		maxWidth = std::max(maxWidth, lineRect.w);
	}

	rects = lineRects;
	if (hAlign == "left") {
		for (SDL_Rect &rect : rects) {
			rect.x = 0;
			rect.y += indentY;
		}
	}else {
		bool isCenter = hAlign == "center";
		for (SDL_Rect &rect : rects) {
			if (isCenter) {
				rect.x = (maxWidth - rect.w) / 2;
			}else {
				rect.x = maxWidth - rect.w;
			}
			rect.y += indentY;
		}
	}


	float sinA = Math::getSin(int(getGlobalRotate()));
	float cosA = Math::getCos(int(getGlobalRotate()));

	for (SDL_Rect &rect : rects) {
		float x = float(rect.x);
		float y = float(rect.y);

		rect.x = int(x * cosA - y * sinA);
		rect.y = int(x * sinA + y * cosA);
	}
}

bool TextField::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0) return false;

	float fx = float(x);
	float fy = float(y);

	if (globalClipping) {
		if (fx + globalX < clipRect.x ||
		    fy + globalY < clipRect.y ||
		    fx + globalX >= clipRect.x + clipRect.w ||
		    fy + globalY >= clipRect.y + clipRect.h
		) return false;
	}

	for (const SDL_Rect &rect : rects) {
		if (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h) {
			return true;
		}
	}
	return false;
}

void TextField::draw() const {
	if (!enable || globalAlpha <= 0) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, true);
	SDL_Point center = { int(xAnchor), int(yAnchor) };

	for (size_t i = 0; i < lineSurfaces.size(); ++i) {
		SurfacePtr surface = lineSurfaces[i];
		if (!surface) continue;

		SDL_Rect srcRect = { 0, 0, surface->w, surface->h };

		const SDL_Rect &rect = rects[i];
		SDL_Rect dstIRect = DisplayObject::buildIntRect(float(rect.x) + globalX, float(rect.y) + globalY, float(rect.w), float(rect.h), true);

		pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, srcRect, dstIRect, center);
	}
}
