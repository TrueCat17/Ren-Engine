#include "text.h"

#include "group.h"
#include "renderer.h"

#include "utils/math.h"
#include "utils/utils.h"


const char *Text::defaultFontName = "Arial";

Text::Text() {

}

void Text::setFont(String fontName, int textSize, bool setAsOriginal) {
	if (!fontName) {
		fontName = defaultFontName;
	}

	if (textSize < MIN_TEXT_SIZE) textSize = MIN_TEXT_SIZE;
	if (textSize > MAX_TEXT_SIZE) textSize = MAX_TEXT_SIZE;
	this->textSize = textSize;
	this->fontName = fontName;

	if (setAsOriginal) {
		originalTextSize = textSize;
	}

	font = Utils::getFont(fontName, textSize);
	if (!font && fontName != defaultFontName) {
		Utils::outMsg("TTF_Open_Font", TTF_GetError());
		Utils::outMsg("Text::setFont", "Загружается шрифт <" + String(defaultFontName) + ">");

		fontName = defaultFontName;
		font = Utils::getFont(fontName, textSize);
	}
	if (!font && fontName == defaultFontName) {
		Utils::outMsg("TTF_Open_Font", TTF_GetError());
		Utils::outMsg("Text::setFont", "Шрифт <" + String(defaultFontName) + "> не загрузился. Текст не будет отображаться.\n");
	}else {
		TTF_SizeUTF8(font, "t", &charWidth, nullptr);//просто буква наугад
		charHeight = TTF_FontHeight(font);
	}
}


int Text::getMaxWidth() const {
	return maxWidth;
}
int Text::getMaxHeight() const {
	return maxHeight;
}
void Text::setMaxWidth(int width) {
	maxWidth = width;
}
void Text::setMaxHeight(int height) {
	maxHeight = height;
}

void Text::pushStyle() {
	bolds.push_back(isBold);
	italics.push_back(isItalic);
	underlines.push_back(isUnderline);
	strikes.push_back(isStrike);

	isBold = 0;
	isItalic = 0;
	isUnderline = 0;
	isStrike = 0;
}
void Text::popStyle() {
	const int level = bolds.size() - 1;

	isBold = bolds[level];
	isItalic = italics[level];
	isUnderline = underlines[level];
	isStrike = strikes[level];

	bolds.pop_back();
	italics.pop_back();
	underlines.pop_back();
	strikes.pop_back();
}
void Text::resetStyle() {
	int level = bolds.size() - 1;

	bolds[level] = 0;
	italics[level] = 0;
	underlines[level] = 0;
	strikes[level] = 0;
}

String Text::getText() const {
	return prevText;
}

void Text::setText(const String &text, int color) {
	if (text && !font) {
		setFont("", 20, true);
	}
	if (!font) return;

	size_t countLinesAtStart = 0;

	const int maxW = maxWidth  > 0 ? maxWidth  : 1e9;
	const int maxH = maxHeight > 0 ? maxHeight : 1e9;

	if (text) {
		lines = text.split("\n");
	}else {
		lines.clear();
	}

	curColor = mainColor = color;
	int maxLineWidth = 0;

	pushStyle();
	std::vector<SDL_Rect> tmpRects;
	for (size_t i = countLinesAtStart; i < lines.size(); ++i) {
		String line = lines[i];

		SDL_Rect lineRect = { 0, int(i * charHeight), 0, charHeight };
		lineRect.w = getLineWidth(line);

		const int INDENT = charWidth * 1.5;

		if (wordWrap && maxW > INDENT && lineRect.w > maxW) {//Включён перенос строк по словам (и он возможен) и он нужен
			int n = line.size() - 1;
			while (n > 0) {
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
			if (!n && //Если перенести по пробелу нельзя и
				textSize > MIN_TEXT_SIZE) //можно уменьшить шрифт
			{
				maxLineWidth = maxW + 1;//переход к попытке уместить текст меньшим шрифтом
				break;
			}

			//Если перенос возможен, то разделяем текущую строку на две
			if (int(line.size()) > n + 1) {
				String newLine = line.substr(n + 1);
				lines.insert(lines.begin() + i + 1, newLine);
			}
			lines[i].erase(n);
			//И обрабатываем текущую строку снова
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
	int textHeight = charHeight * lines.size();

	//Если не уместились с этим размером шрифта
	if ((textHeight > maxH || maxLineWidth > maxW) && textSize > MIN_TEXT_SIZE) {
		setFont(fontName, textSize - 2, false);//Устанавливаем меньший шрифт
		setText(text, color);//И пытаемся ещё раз
		return;
	}

	//Не перерисовываем, если нарисовано будет ровно то же самое, что есть уже сейчас
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

void Text::addText() {
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

	const String line = lines[numLine];
	const int length = line.size();

	for (; charOutNum < length; ++charOutNum) {
		char c = line[charOutNum];

		if (c == '{') {
			int start = charOutNum + 1;
			int end = charOutNum + 2;
			while (end < length && line[end] != '}') {
				++end;
			}
			charOutNum = end;

			int k = line[start] == '/' ? (++start, -1) : 1;//открытие или закрытие тэга

			start = line.find_first_not_of(' ', start);
			end = line.find_last_not_of(' ', end);
			String type = line.substr(start, end - start);

			if (type == "b") isBold += k;
			else if (type == "i") isItalic += k;
			else if (type == "u") isUnderline += k;
			else if (type == "s") isStrike += k;
			else if (type.startsWith("color")) {
				if (k == 1) {
					int sepIndex = type.find('=');
					String colorStr = type.substr(sepIndex + 1);
					if (colorStr && colorStr[0] == '#') {
						colorStr.erase(0, 1);
					}
					int newColor = colorStr.toInt(16);

					curColor = newColor;
				}else {
					curColor = mainColor;
				}
			}

			else {
				Utils::outMsg("Text::addText",
							  "Неизвестный тэг <" + type + ">");
			}

			if (isBold < 0) isBold = 0;
			if (isItalic < 0) isItalic = 0;
			if (isUnderline < 0) isUnderline = 0;
			if (isStrike < 0) isStrike = 0;
		}else {
			int startStyle = charOutNum;
			int endStyle = charOutNum + 1;

			while (endStyle < length && line[endStyle] != '{') {
				++endStyle;
			}
			charOutNum = endStyle - 1;
			String oneStyleStr = line.substr(startStyle, endStyle - startStyle);

			addChars(oneStyleStr, curColor);
		}
	}

	charOutNum = 0;
	charX = 0;
}

int Text::getLineWidth(String text, bool resetPrevStyle) {
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

			int k = text[start] == '/' ? (++start, -1) : 1;//открытие или закрытие тэга

			start = text.find_first_not_of(' ', start);
			end = text.find_last_not_of(' ', end);
			String type = text.substr(start, end - start);

			if (type == "b") isBold += k;
			else if (type == "i") isItalic += k;
			else if (type == "u") isUnderline += k;
			else if (type == "s") isStrike += k;
			else if (type.startsWith("color"));
			else {
				Utils::outMsg("Text::getLineWidth",
							  "Неизвестный тэг <" + type + ">");
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
			String oneStyleStr = text.substr(startStyle, endStyle - startStyle);

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

void Text::addChars(String c, int color) {
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
	sdlColor.r = color >> 16;
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

void Text::setAlign(String hAlign, String vAlign) {
	if (hAlign != "left" && hAlign != "center" && hAlign != "right") {
		Utils::outMsg("Text::setAlign", "Недопустимое значение hAlign: <" + hAlign + ">\n");
		hAlign = "left";
	}
	if (vAlign != "top" && vAlign != "center" && vAlign != "down") {
		Utils::outMsg("Text::setAlign", "Недопустимое значение vAlign: <" + vAlign + ">\n");
		vAlign = "top";
	}
	this->hAlign = hAlign;
	this->vAlign = vAlign;

	int indentY = 0;
	if (vAlign == "center") {
		indentY = (getHeight() - lines.size() * charHeight) / 2;
	}else if (vAlign == "down") {
		indentY = getHeight() - lines.size() * charHeight;
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
			rects[i].y = i * charHeight + indentY;
		}
	}else {
		bool isCenter = hAlign == "center";
		for (size_t i = 0; i < rects.size(); ++i) {
			if (isCenter) {
				rects[i].x = (maxWidth - rects[i].w) / 2;
			}else {
				rects[i].x = maxWidth - rects[i].w;
			}
			rects[i].y = i * charHeight + indentY;
		}
	}

	int w = this->maxWidth  <= 0 ? maxWidth : this->maxWidth;
	int h = this->maxHeight <= 0 ? (int(rects.size()) * charHeight) : this->maxHeight;
	setSize(w, h);


	double sinA = Math::getSin(getGlobalRotate());
	double cosA = Math::getCos(getGlobalRotate());

	for (size_t i = 0; i < rects.size(); ++i) {
		int x = rects[i].x;
		int y = rects[i].y;

		int rotX = x * cosA - y * sinA;
		int rotY = x * sinA + y * cosA;

		rects[i].x = rotX;
		rects[i].y = rotY;
	}
}

bool Text::checkAlpha(int x, int y) const {
	if (!alpha || !enable) return false;

	for (const SDL_Rect &rect : rects) {
		if (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h) {
			return true;
		}
	}
	return false;
}

void Text::draw() const {
	if (!enable || globalAlpha <= 0) return;

	for (size_t i = 0; i < lineSurfaces.size(); ++i) {
		SurfacePtr surface = lineSurfaces[i];
		if (!surface) continue;

		Uint8 intAlpha = Math::inBounds(int(globalAlpha * 255), 0, 255);

		SDL_Rect t = rects[i];
		t.x += globalX;
		t.y += globalY;

		SDL_Point center = { int(xAnchor), int(yAnchor) };

		pushToRender(surface, globalRotate, intAlpha, nullptr, &t, &center);
	}
}

Text::~Text() {
	font = nullptr;
}
