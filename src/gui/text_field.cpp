#include "text_field.h"

#include <map>
#include <set>

#include <SDL2/SDL_ttf.h>

#include "renderer.h"

#include "media/image_manipulator.h"
#include "media/py_utils.h"
#include "media/sprite.h"

#include "utils/file_system.h"
#include "utils/math.h"
#include "utils/string.h"
#include "utils/utils.h"


bool operator==(const SDL_Rect &a, const SDL_Rect &b);

inline int getCountOutlines(const TextStyle &textStyle) {
	if (textStyle.enableOutline) {
		return textStyle.fontSize >= 25 ? 2 : 1;
	}
	return 0;
}


static const int MIN_TEXT_SIZE = 8;
static const int MAX_TEXT_SIZE = 128;

const std::string TextField::DEFAULT_FONT_NAME = "Arial";

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

	if (name != TextField::DEFAULT_FONT_NAME) {
		Utils::outError("getFont",
		                "Failed to load font <%>.\n"
		                "Try to load default font.",
		                name);
		res = getFont(TextField::DEFAULT_FONT_NAME, size);
	}else {
		Utils::outError("getFont",
		                "Failed to load default font <%>.\n"
		                "Text will not be displayed.",
		                name);
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

	int origW = *w;
	int origH = *h;

	int countOutlines = getCountOutlines(style);
	*w += countOutlines * 2;
	*h += countOutlines * 2;

	SDL_Rect rect = {x, std::max(0, maxH - *h), *w, *h};
	if (rect.y < 3) rect.y = 0;//fix for fonts, where some symbols have height more than font size on init

	SurfacePtr tmpSurface;
	if (style.enableOutline) {
		//masks from TTF_RenderUTF8_Blended
		tmpSurface = SDL_CreateRGBSurface(0, *w, *h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

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

		std::pair<int, int> pairs[8] = {{-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}};
		for (auto [x, y] : pairs) {
			for (int i = 1; i <= countOutlines; ++i) {
				SDL_Rect to = {countOutlines + x * i, countOutlines + y * i, origW, origH};
				SDL_BlitSurface(surfaceOutlineText, nullptr, tmpSurface.get(), &to);
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
		SDL_Rect to = {countOutlines, countOutlines, origW, origH};
		SDL_BlitSurface(surfaceText, nullptr, tmpSurface.get(), &to);

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
	if (Sprite::imageWasRegistered(path)) {
		std::vector<std::string> commands = Sprite::getChildrenImagesDeclAt(path);
		if (commands.empty()) return nullptr;

		std::string realPath = PyUtils::exec("CPP_EMBED: text_field.cpp", __LINE__, commands.front(), true);
		return ImageManipulator::getImage(realPath);
	}

	return ImageManipulator::getImage(path);
}
static Uint32 getColor(std::string_view value) {
	bool shortForm = false;
	if (String::startsWith(value, "#")) {
		value.remove_prefix(1);
		if (value.size() == 3) {
			shortForm = true;
		}
	}else
	if (String::startsWith(value, "0x")) {
		value.remove_prefix(2);
	}

	if (value.size() > 6) {
		Utils::outError("TextField::getColor", "Expected color (format RRGGBB), got <%>", value);
		value.remove_suffix(value.size() - 6);
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
		if (shortForm) {
			res = res * 16 + Uint32(n);
		}
	}
	if (invalid) {
		Utils::outError("TextField::getColor", "Color <%> is invalid", value);
	}
	return res;
}
static void updateStyle(TextStyle &textStyle) {
	TTF_SetFontStyle(textStyle.font, textStyle.fontStyle);
}
static void updateFont(TextStyle &textStyle) {
	textStyle.font = getFont(*textStyle.fontName, int(std::round(textStyle.fontSize)));
}

static bool invisible;
static std::set<std::string> tagsWithValue = { "color", "outlinecolor", "alpha", "size", "font", "image" };
static size_t makeStep(const std::string &line, size_t i, std::vector<TextStyle> &styleStack, int x, int *w, int *h, SDL_Surface *surface) {
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
		size_t end = line.find('}', start);
		if (start >= end || end == size_t(-1)) {
			Utils::outMsg("TextField::makeStep", "Incomplete tag");
			return line.size();
		}

		i = end;
		bool closeTag = line[start] == '/';
		if (closeTag) ++start;

		end = line.find_last_not_of(' ', end);
		std::string tag = line.substr(start, end - start);

		if (closeTag) {
			if (tag == styleStack.back().tag) {
				styleStack.pop_back();
				updateStyle(styleStack.back());
			}else {
				Utils::outError("TextField::makeStep",
				                "Closed tag <%> != last opened tag <%>",
				                tag, styleStack.back().tag);
			}
		}else {
			TextStyle style = styleStack.back();//copy

			std::string value = getTagValue(tag);
			if (value.empty() && tagsWithValue.count(tag)) {
				Utils::outError("TextField::makeStep", "Tag <%> must have value", tag);
				return i + 1;
			}
			style.tag = tag;

			auto apply = [](float &prop, std::string_view value) {
				if (value[0] == '*') {
					value.remove_prefix(1);
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
			else if (tag == "font") style.fontName = String::getConstPtr(value);

			else if (tag != "image" && tag != "invisible") {
				unknownTag = true;
				Utils::outError("TextField::makeStep", "Unknown tag <%>", tag);
			}

			if (tag == "invisible") {
				invisible = true;
			}else
			if (tag == "image") {
				SurfacePtr image = getImage(value);
				if (!image) {
					Utils::outError("TextField::makeStep", "Failed to load image <%>", value);
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
			}else
			if (!unknownTag) {
				if (tag == "font" || tag == "size") {
					updateFont(style);
				}
				updateStyle(style);

				styleStack.push_back(style);
			}
		}
	}else {
		size_t start = i;
		size_t end = line.find('{', i + 1);
		if (end == size_t(-1)) {
			end = line.size();
		}
		i = end - 1;
		std::string word = line.substr(start, end - start);

		TextStyle &style = styleStack.back();
		if (onlySize) {
			if (TTF_SizeUTF8(style.font, word.c_str(), w, h)) {
				Utils::outMsg("TTF_SizeUTF8", TTF_GetError());
			}
			int countOutlines = getCountOutlines(style);
			*w += countOutlines * 2;
			*h += countOutlines * 2;
		}else if (!invisible) {
			addChars(word, x, w, h, style, surface);
		}
	}

	return i + 1;
}

void TextField::setFont(const std::string *fontName, float fontSize) {
	mainStyle.fontSize = fontSize;
	mainStyle.fontName = String::getConstPtr(fontName->empty() ? DEFAULT_FONT_NAME : *fontName);
	updateFont(mainStyle);
}


static size_t findWrapIndex(const std::string &line, std::vector<TextStyle> styleStack, int maxWidth) {
	std::vector<TextStyle> originStyleStack = styleStack;

	struct TextPart {
		size_t start;
		size_t len;
		int widthOnEnd;
	};
	std::vector<TextPart> parts;

	updateStyle(styleStack.back());
	int width = 0;
	size_t i = 0;
	while (i < line.size() && width <= maxWidth) {
		TextPart part;
		part.start = i;

		int w, h;
		i = makeStep(line, i, styleStack, -1, &w, &h, nullptr);
		if (!w) continue;

		width += w;

		part.len = i - part.start;
		part.widthOnEnd = width;

		parts.push_back(part);
	}
	if (width <= maxWidth) {
		return line.size();
	}


	const TextPart &part = parts.back();

	int prevWidth = 0;
	if (parts.size() > 1) {
		prevWidth = parts[parts.size() - 2].widthOnEnd;
	}
	int partMaxWidth = maxWidth - prevWidth;

	//calculate style stack for the part
	styleStack.swap(originStyleStack);
	updateStyle(styleStack.back());
	i = 0;
	while (i < part.start) {
		int w, h;
		i = makeStep(line, i, styleStack, -1, &w, &h, nullptr);
	}
	TTF_Font *font = styleStack.back().font;

	std::string partStr = line.substr(part.start, part.len);

	size_t spaceBeforePart = size_t(-1);
	for (size_t i = parts.size() - 2; i != size_t(-1); --i) {
		const TextPart &prevPart = parts[i];
		std::string_view prevPartStr = std::string_view(line).substr(prevPart.start, prevPart.len);
		size_t prevPartSpaceIndex = prevPartStr.rfind(' ');
		if (prevPartSpaceIndex != size_t(-1)) {
			spaceBeforePart = prevPart.start + prevPartSpaceIndex;
			break;
		}
	}

	width = 0;
	i = 0;
	while (i < partStr.size() && width <= partMaxWidth) {
		size_t start = i++;
		while (!String::isFirstByte(partStr[i])) ++i;
		const std::string symbol = std::string(partStr.substr(start, i - start));

		int w;
		if (TTF_SizeUTF8(font, symbol.c_str(), &w, nullptr)) {
			Utils::outMsg("TTF_SizeUTF8", TTF_GetError());
			return line.size();
		}
		if (!width) {
			width += getCountOutlines(styleStack.back()) * 2;
		}
		width += w;
	}

	size_t firstSpaceIndex = partStr.find(' ');
	if (i < partStr.size() && partStr[i] != ' ') {//index in the middle of a word
		if (i > firstSpaceIndex) {
			i = partStr.rfind(' ', i);
		}else {
			if (spaceBeforePart != size_t(-1)) {
				return spaceBeforePart;
			}
		}
	}
	partStr.erase(i);
	firstSpaceIndex = partStr.find(' ');


	while (!partStr.empty()) {
		while (!partStr.empty() && partStr.back() == ' ') {
			partStr.pop_back();
		}

		if (TTF_SizeUTF8(font, partStr.c_str(), &width, nullptr)) {
			Utils::outMsg("TTF_SizeUTF8", TTF_GetError());
			return line.size();
		}
		width += getCountOutlines(styleStack.back()) * 2;
		if (width <= partMaxWidth) break;

		size_t spaceIndex = partStr.rfind(' ');
		if (spaceIndex != size_t(-1) && firstSpaceIndex != size_t(-1) && spaceIndex >= firstSpaceIndex) {
			partStr.erase(spaceIndex);
		}else {
			if (spaceBeforePart != size_t(-1)) {
				return spaceBeforePart;
			}

			while (!partStr.empty() && !String::isFirstByte(partStr.back())) {
				partStr.pop_back();
			}
			if (!partStr.empty()) {
				partStr.pop_back();
			}
		}
	}
	return part.start + partStr.size();
}

void TextField::setText(const std::string &text) {
	if (!mainStyle.font) return;

	std::vector<TextStyle> styleStack;
	styleStack.reserve(4);
	styleStack.push_back(mainStyle);
	updateStyle(mainStyle);

	std::vector<std::string> tmpLines = String::split(text, "\n");
	int curTextWidth = 0;
	int curTextHeight = 0;

	//calc sizes & apply wordwrap
	std::vector<SDL_Rect> tmpLineRects;
	for (size_t numLine = 0; numLine < tmpLines.size(); ++numLine) {
		std::string &line = tmpLines[numLine];

		//check text wrap
		size_t wrapIndex = line.size();
		if (maxWidth > 0) {
			size_t firstNotSpace = line.find_first_not_of(' ');
			wrapIndex = findWrapIndex(line, styleStack, maxWidth);
			if (firstNotSpace != size_t(-1) && wrapIndex <= firstNotSpace) {
				line.erase(0, firstNotSpace);
				wrapIndex = line.empty() ? 0 : 1;
				while (!String::isFirstByte(line[wrapIndex])) {
					++wrapIndex;
				}
			}
		}
		if (wrapIndex != line.size()) {
			std::string nextLine;
			size_t startNextLine = line.find_first_not_of(' ', wrapIndex);
			if (startNextLine != size_t(-1)) {
				nextLine = line.substr(startNextLine);
			}

			line.erase(wrapIndex);
			while (!line.empty() && line.back() == ' ') {
				line.pop_back();
			}

			if (line.empty()) {
				line = nextLine;
			}else {
				tmpLines.insert(tmpLines.begin() + long(numLine + 1), nextLine);
			}
			--numLine;
			continue;
		}

		SDL_Rect lineRect = { 0, curTextHeight, 0, 0};
		int lineWidth = 0;
		int lineHeight = TTF_FontHeight(styleStack.back().font);

		updateStyle(styleStack.back());
		size_t i = 0;
		while (i < line.size()) {
			int w, h;
			i = makeStep(line, i, styleStack, lineWidth, &w, &h, nullptr);
			lineWidth += w;
			lineHeight = std::max(lineHeight, h);
		}

		lineWidth = std::min(lineWidth, Renderer::info.max_texture_width);
		curTextWidth = std::max(curTextWidth, lineWidth);

		lineRect.w = lineWidth;
		lineRect.h = lineHeight;
		tmpLineRects.push_back(lineRect);

		curTextHeight += lineHeight;
		if (maxHeight > 0 && curTextHeight >= maxHeight) {
			tmpLineRects.back().h -= curTextHeight - maxHeight;
			curTextHeight = maxHeight;
			tmpLines.erase(tmpLines.begin() + long(numLine + 1), tmpLines.end());
			break;
		}
	}

	if (
	    prevMainStyle == mainStyle && tmpLines == lines && tmpLineRects == lineRects &&
	    prevMaxWidth == maxWidth && prevMaxHeight == maxHeight &&
	    prevUseMaxForWidth == useMaxForWidth && prevUseMaxForHeight == useMaxForHeight
	) return;

	prevMainStyle = mainStyle;
	lines.swap(tmpLines);
	lineRects.swap(tmpLineRects);
	prevMaxWidth = maxWidth;
	prevMaxHeight = maxHeight;
	prevUseMaxForWidth = useMaxForWidth;
	prevUseMaxForHeight = useMaxForHeight;

	if (useMaxForWidth || maxWidth <= 0) {
		setWidth(float(std::max(maxWidth, curTextWidth)));
	}else {
		setWidth(float(std::min(maxWidth, curTextWidth)));
	}
	if (useMaxForHeight || maxHeight <= 0) {
		setHeight(float(std::max(maxHeight, curTextHeight)));
	}else {
		setHeight(float(std::min(maxHeight, curTextHeight)));
	}

	//draw with calced sizes
	styleStack.erase(styleStack.begin() + 1, styleStack.end());
	updateStyle(mainStyle);
	lineSurfaces.clear();
	invisible = false;
	for (size_t numLine = 0; numLine < lines.size(); ++numLine) {
		setLine(numLine, lines[numLine], styleStack);
	}
}

void TextField::setLine(size_t numLine, const std::string &line, std::vector<TextStyle> &styleStack) {
	const SDL_Rect &lineRect = lineRects[numLine];

	SDL_Surface *surface = nullptr;
	if (lineRect.w) {
		surface = SDL_CreateRGBSurfaceWithFormat(0, lineRect.w, lineRect.h, 32, SDL_PIXELFORMAT_RGBA32);
	}
	SurfacePtr surfacePtr = surface;

	lineSurfaces.push_back(surfacePtr);

	size_t i = 0;
	int x = 0;
	int w, h;
	while (i < line.size()) {
		h = lineRect.h;
		i = makeStep(line, i, styleStack, x, &w, &h, surface);
		x += w;
	}
}

void TextField::setAlign(float hAlign, float vAlign) {
	this->hAlign = hAlign;
	this->vAlign = vAlign;

	int height = 0;
	for (const SDL_Rect &lineRect : lineRects) {
		height += lineRect.h;
	}
	float indentY = (getHeight() - float(height)) * vAlign;

	float sinA = Math::getSin(int(getGlobalRotate()));
	float cosA = Math::getCos(int(getGlobalRotate()));

	rects = lineRects;
	for (SDL_Rect &rect : rects) {
		float x = float(getWidth() - float(rect.w)) * hAlign;
		float y = float(rect.y) + indentY;

		rect.x = int(x * cosA - y * sinA);
		rect.y = int(x * sinA + y * cosA);
	}
}

bool TextField::transparentForMouse(int x, int y) const {
	if (!enable || globalAlpha <= 0 || globalSkipMouse) return true;

	if (globalClipping) {
		float fx = float(x);
		float fy = float(y);

		if (fx + globalX < clipRect.x ||
		    fy + globalY < clipRect.y ||
		    fx + globalX >= clipRect.x + clipRect.w ||
		    fy + globalY >= clipRect.y + clipRect.h
		) return true;
	}

	for (const SDL_Rect &rect : rects) {
		if (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h) {
			return false;
		}
	}
	return true;
}

void TextField::draw() const {
	if (!enable || globalAlpha <= 0) return;

	Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));
	SDL_Rect clipIRect = DisplayObject::buildIntRect(clipRect.x, clipRect.y, clipRect.w, clipRect.h, true);
	SDL_Point center = { int(calcedXanchor), int(calcedYanchor) };

	for (size_t i = 0; i < lineSurfaces.size(); ++i) {
		SurfacePtr surface = lineSurfaces[i];
		if (!surface) continue;

		SDL_Rect srcRect = { 0, 0, surface->w, surface->h };

		const SDL_Rect &rect = rects[i];
		SDL_Rect dstIRect = DisplayObject::buildIntRect(float(rect.x) + globalX, float(rect.y) + globalY, float(rect.w), float(rect.h), true);

		pushToRender(surface, globalRotate, intAlpha, globalClipping, clipIRect, srcRect, dstIRect, center);
	}
}
