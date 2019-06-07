#include "text_field.h"

#include <SDL2/SDL_ttf.h>

#include "group.h"
#include "renderer.h"

#include "utils/math.h"
#include "utils/string.h"
#include "utils/utils.h"


static const std::string defaultFontName = "Arial";

TextField::TextField() {}

void TextField::setFont(std::string fontName, int textSize, bool setAsOriginal) {
	if (fontName.empty()) {
		fontName = defaultFontName;
	}

	textSize = Math::inBounds(textSize, MIN_TEXT_SIZE, MAX_TEXT_SIZE);
	this->textSize = textSize;
	this->fontName = fontName;

	if (setAsOriginal) {
		originalTextSize = textSize;
	}

	font = Utils::getFont(fontName, textSize);
	if (!font && fontName != defaultFontName) {
		Utils::outMsg("TTF_Open_Font", TTF_GetError());
		Utils::outMsg("Text::setFont", "Loading font <" + defaultFontName + ">");

		fontName = defaultFontName;
		font = Utils::getFont(fontName, textSize);
	}
	if (!font && fontName == defaultFontName) {
		Utils::outMsg("TTF_Open_Font", TTF_GetError());
		Utils::outMsg("Text::setFont", "Could not load font <" + defaultFontName + ">. No text will be displayed.\n");
	}else {
		TTF_SizeUTF8(font, "t", &charWidth, nullptr);//just random symbol
		charHeight = TTF_FontHeight(font);
	}
}


int TextField::getMaxWidth() const {
	return maxWidth;
}
int TextField::getMaxHeight() const {
	return maxHeight;
}
void TextField::setMaxWidth(int width) {
	maxWidth = width;
}
void TextField::setMaxHeight(int height) {
	maxHeight = height;
}

void TextField::pushStyle() {
	bolds.push_back(isBold);
	italics.push_back(isItalic);
	underlines.push_back(isUnderline);
	strikes.push_back(isStrike);

	isBold = 0;
	isItalic = 0;
	isUnderline = 0;
	isStrike = 0;
}
void TextField::popStyle() {
	const size_t level = bolds.size() - 1;

	isBold = bolds[level];
	isItalic = italics[level];
	isUnderline = underlines[level];
	isStrike = strikes[level];

	bolds.pop_back();
	italics.pop_back();
	underlines.pop_back();
	strikes.pop_back();
}
void TextField::resetStyle() {
	size_t level = bolds.size() - 1;

	bolds[level] = 0;
	italics[level] = 0;
	underlines[level] = 0;
	strikes[level] = 0;
}

std::string TextField::getText() const {
	return prevText;
}

void TextField::setText(const std::string &text, Uint32 color) {
	if (!text.empty() && !font) {
		setFont("", 20, true);
	}
	if (!font) return;

	size_t countLinesAtStart = 0;

	const int maxW = maxWidth  > 0 ? maxWidth  : 1e9;
	const int maxH = maxHeight > 0 ? maxHeight : 1e9;

	if (text.empty()) {
		lines.clear();
	}else {
		lines = String::split(text, "\n");
	}

	curColor = mainColor = color;
	int maxLineWidth = 0;

	pushStyle();
	std::vector<SDL_Rect> tmpRects;
	for (size_t i = countLinesAtStart; i < lines.size(); ++i) {
		std::string line = lines[i];

		SDL_Rect lineRect = { 0, int(i) * charHeight, 0, charHeight };
		lineRect.w = getLineWidth(line);

		const int INDENT = int(charWidth * 1.5);

		if (wordWrap && maxW > INDENT && lineRect.w > maxW) {//wordwrap is on, possible and need
			size_t n = line.size() - 1;
			while (n) {
				size_t indexOpen = line.find_last_of('{', n);
				size_t indexClose = line.find_last_of('}', n);
				if (indexOpen != size_t(-1)) {
					if (indexClose < indexOpen) {
						n = indexOpen - 1;
					}
				}
				if (line[n] == ' ' && getLineWidth(line.substr(0, n), true) < maxW - INDENT) break;

				--n;
			}
			if (!n && //could not wordwrap
			    textSize > MIN_TEXT_SIZE) //try decrease font size
			{
				maxLineWidth = maxW + 1;
				break;
			}

			//if wordwrap is possible - divide line
			if (line.size() > n + 1) {
				const std::string newLine = line.substr(n + 1);
				lines.insert(lines.begin() + int(i) + 1, newLine);
			}
			lines[i].erase(n);
			//processing current line again
			--i;
			continue;
		}

		if (lineRect.w > maxLineWidth) {
			maxLineWidth = lineRect.w;
		}
		tmpRects.push_back(lineRect);
	}
	popStyle();

	rect.w = std::max(maxWidth, maxLineWidth);
	int textHeight = charHeight * int(lines.size());

	//if could not to set text with current font size
	if ((textHeight > maxH || maxLineWidth > maxW) && textSize > MIN_TEXT_SIZE) {
		setFont(fontName, textSize - 2, false);//decrease font size
		setText(text, color);//try again
		return;
	}

	//not redraw if no changes
	if (prevDrawFont == font && prevDrawTextColor == color && prevText == text && tmpRects.size() == rects.size()) {
		return;
	}

	prevDrawFont = font;
	prevDrawTextColor = color;
	prevText = text;

	charOutNum = 0;
	charX = 0;

	isBold = isItalic = isUnderline = isStrike = 0;
	rect.h = maxHeight > 0 ? maxHeight : textHeight;

	lineSurfaces.clear();
	rects = tmpRects;
	for (numLine = 0; numLine < lines.size(); ++numLine) {
		addText();
	}

	setAlign(hAlign, vAlign);
}

void TextField::addText() {
	SDL_Rect lineRect = rects[numLine];

	if (!lineRect.w) {
		lineSurfaces.push_back(nullptr);
		charOutNum = 0;
		return;
	}

	if (charOutNum == 0) {
		SurfacePtr surfacePtr(SDL_CreateRGBSurfaceWithFormat(0, lineRect.w, lineRect.h, 32, SDL_PIXELFORMAT_RGBA32),
							  SDL_FreeSurface);
		lineSurfaces.push_back(surfacePtr);

		if (!surfacePtr) {
			charOutNum = 1e9;
			return;
		}
	}

	const std::string line = lines[numLine];
	const size_t length = line.size();

	for (; charOutNum < length; ++charOutNum) {
		char c = line[charOutNum];

		if (c == '{') {
			size_t start = charOutNum + 1;
			size_t end = charOutNum + 2;
			while (end < length && line[end] != '}') {
				++end;
			}
			charOutNum = end;

			int k = 1;//open/close tag
			if (line[start] == '/') {
				++start;
				k = -1;
			}

			start = line.find_first_not_of(' ', start);
			end = line.find_last_not_of(' ', end);
			std::string type = line.substr(start, end - start);

			if (type == "b") isBold += k;
			else if (type == "i") isItalic += k;
			else if (type == "u") isUnderline += k;
			else if (type == "s") isStrike += k;
			else if (String::startsWith(type, "color")) {
				if (k == 1) {
					size_t sepIndex = type.find('=');
					std::string colorStr = type.substr(sepIndex + 1);
					if (!colorStr.empty() && colorStr[0] == '#') {
						colorStr.erase(0, 1);
					}
					Uint32 newColor = Uint32(String::toInt(colorStr, 16));

					curColor = newColor;
				}else {
					curColor = mainColor;
				}
			}

			else {
				Utils::outMsg("Text::addText",
				              "Unknown tag <" + type + ">");
			}

			if (isBold < 0) isBold = 0;
			if (isItalic < 0) isItalic = 0;
			if (isUnderline < 0) isUnderline = 0;
			if (isStrike < 0) isStrike = 0;
		}else {
			size_t startStyle = charOutNum;
			size_t endStyle = charOutNum + 1;

			while (endStyle < length && line[endStyle] != '{') {
				++endStyle;
			}
			charOutNum = endStyle - 1;
			std::string oneStyleStr = line.substr(startStyle, endStyle - startStyle);

			addChars(oneStyleStr, curColor);
		}
	}

	charOutNum = 0;
	charX = 0;
}

int TextField::getLineWidth(std::string text, bool resetPrevStyle) {
	if (resetPrevStyle) {
		resetStyle();
	}

	int res = 0;

	for (size_t i = 0; i < text.size(); ++i) {
		char c = text[i];

		if (c == '{') {
			size_t start = i + 1;
			size_t end = i + 2;
			while (end < text.size() && text[end] != '}') {
				++end;
			}
			i = end;

			int k = 1;//open/close tag
			if (text[start] == '/') {
				++start;
				k = -1;
			}

			start = text.find_first_not_of(' ', start);
			end = text.find_last_not_of(' ', end);
			std::string type = text.substr(start, end - start);

			if (type == "b") isBold += k;
			else if (type == "i") isItalic += k;
			else if (type == "u") isUnderline += k;
			else if (type == "s") isStrike += k;
			else if (String::startsWith(type, "color"));
			else {
				Utils::outMsg("Text::getLineWidth",
				              "Unknown tag <" + type + ">");
			}

			if (isBold < 0) isBold = 0;
			if (isItalic < 0) isItalic = 0;
			if (isUnderline < 0) isUnderline = 0;
			if (isStrike < 0) isStrike = 0;
		}else {
			size_t startStyle = i;
			size_t endStyle = i;

			while (endStyle < text.size() && text[endStyle] != '{') {
				++endStyle;
			}

			i = endStyle - 1;
			std::string oneStyleStr = text.substr(startStyle, endStyle - startStyle);

			int charsWidth, charsHeight;

			int style = TTF_STYLE_NORMAL;
			if (isBold) style |= TTF_STYLE_BOLD;
			if (isItalic) style |= TTF_STYLE_ITALIC;
			if (isUnderline) style |= TTF_STYLE_UNDERLINE;
			if (isStrike) style |= TTF_STYLE_STRIKETHROUGH;

			TTF_SetFontStyle(font, style);
			TTF_SizeUTF8(font, oneStyleStr.c_str(), &charsWidth, &charsHeight);

			res += charsWidth;
		}
	}

	return res;
}

void TextField::addChars(std::string c, Uint32 color) {
	if (!font) return;

	int style = TTF_STYLE_NORMAL;
	if (isBold) style |= TTF_STYLE_BOLD;
	if (isItalic) style |= TTF_STYLE_ITALIC;
	if (isUnderline) style |= TTF_STYLE_UNDERLINE;
	if (isStrike) style |= TTF_STYLE_STRIKETHROUGH;
	TTF_SetFontStyle(font, style);

	int strWidth;
	int charHeight;
	TTF_SizeUTF8(font, c.c_str(), &strWidth, &charHeight);
	SDL_Rect rect = {charX, 0, strWidth, charHeight};

	charX += strWidth;

	SDL_Color sdlColor;
	sdlColor.r = Uint8(color >> 16);
	sdlColor.g = (color >> 8) & 0xFF;
	sdlColor.b = color & 0xFF;
	sdlColor.a = 0xFF;

	SDL_Surface *surfaceText = TTF_RenderUTF8_Blended(font, c.c_str(), sdlColor);
	if (!surfaceText) {
		Utils::outMsg("TTF_RenderUTF8_Blended", TTF_GetError());
		return;
	}

	if (surfaceText->format->format != SDL_PIXELFORMAT_RGBA32) {
		SDL_Surface *rgbaSurface = SDL_ConvertSurfaceFormat(surfaceText, SDL_PIXELFORMAT_RGBA32, 0);
		if (rgbaSurface) {
			SDL_FreeSurface(surfaceText);
			surfaceText = rgbaSurface;
		}else {
			Utils::outMsg("SDL_ConvertSurfaceFormat", SDL_GetError());
		}
	}

	SurfacePtr surface = lineSurfaces.size() ? lineSurfaces.back() : nullptr;
	if (surface) {
		SDL_BlitSurface(surfaceText, nullptr, surface.get(), &rect);
	}
	SDL_FreeSurface(surfaceText);
}

void TextField::setAlign(std::string hAlign, std::string vAlign) {
	if (hAlign != "left" && hAlign != "center" && hAlign != "right") {
		Utils::outMsg("Text::setAlign", "Unexpected hAlign: <" + hAlign + ">\n");
		hAlign = "left";
	}
	if (vAlign != "top" && vAlign != "center" && vAlign != "down") {
		Utils::outMsg("Text::setAlign", "Unexpected vAlign: <" + vAlign + ">\n");
		vAlign = "top";
	}
	this->hAlign = hAlign;
	this->vAlign = vAlign;

	int indentY = 0;
	if (vAlign == "center") {
		indentY = (getHeight() - int(lines.size()) * charHeight) / 2;
	}else if (vAlign == "down") {
		indentY = getHeight() - int(lines.size()) * charHeight;
	}

	int maxWidth = this->maxWidth;
	for (size_t i = 0; i < rects.size(); ++i) {
		if (rects[i].w > maxWidth) {
			maxWidth = rects[i].w;
		}
	}

	if (hAlign == "left") {
		for (size_t i = 0; i < rects.size(); ++i) {
			rects[i].x = 0;
			rects[i].y = int(i) * charHeight + indentY;
		}
	}else {
		bool isCenter = hAlign == "center";
		for (size_t i = 0; i < rects.size(); ++i) {
			if (isCenter) {
				rects[i].x = (maxWidth - rects[i].w) / 2;
			}else {
				rects[i].x = maxWidth - rects[i].w;
			}
			rects[i].y = int(i) * charHeight + indentY;
		}
	}

	setWidth(this->maxWidth  <= 0 ? maxWidth : this->maxWidth);
	setHeight(this->maxHeight <= 0 ? (int(rects.size()) * charHeight) : this->maxHeight);


	double sinA = Math::getSin(getGlobalRotate());
	double cosA = Math::getCos(getGlobalRotate());

	for (size_t i = 0; i < rects.size(); ++i) {
		int x = rects[i].x;
		int y = rects[i].y;

		int rotX = int(x * cosA - y * sinA);
		int rotY = int(x * sinA + y * cosA);

		rects[i].x = rotX;
		rects[i].y = rotY;
	}
}

bool TextField::checkAlpha(int x, int y) const {
	if (!enable || globalAlpha <= 0) return false;

	if (globalClipping) {
		if (x + globalX < clipRect.x ||
		    y + globalY < clipRect.y ||
		    x + globalX >= clipRect.x + clipRect.w ||
		    y + globalY >= clipRect.y + clipRect.h
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

	for (size_t i = 0; i < lineSurfaces.size(); ++i) {
		SurfacePtr surface = lineSurfaces[i];
		if (!surface) continue;

		Uint8 intAlpha = Uint8(std::min(int(globalAlpha * 255), 255));

		SDL_Rect t = rects[i];
		t.x += globalX;
		t.y += globalY;

		SDL_Rect srcRect = { 0, 0, surface->w, surface->h };

		SDL_Point center = { int(xAnchor), int(yAnchor) };

		pushToRender(surface, globalRotate, intAlpha, globalClipping, clipRect, srcRect, t, center);
	}
}

TextField::~TextField() {
	font = nullptr;
}