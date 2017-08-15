#include "text.h"

#include "gv.h"

#include "group.h"

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

	font = Utils::getFont(Utils::FONTS + fontName + ".ttf", textSize);
	if (!font && fontName != defaultFontName) {
		Utils::outMsg("TTF_Open_Font", TTF_GetError());
		Utils::outMsg("Text::setFont", "Загружается шрифт '" + String(defaultFontName) + "'");

		fontName = defaultFontName;
		font = Utils::getFont(Utils::FONTS + fontName + ".ttf", textSize);
	}
	if (!font && fontName == defaultFontName) {
		Utils::outMsg("TTF_Open_Font", TTF_GetError());
		Utils::outMsg("Text::setFont", "Шрифт '" + String(defaultFontName) + "' не загрузился. Текст не будет отображаться.\n");
	}else {
		TTF_SizeUTF8(font, "t", &charWidth, &charHeight);//просто буква наугад, роль играет только высота
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

	size_t countLinesAtStart = 0;

	const int maxW = maxWidth > 0  ? maxWidth  : 1e9;
	const int maxH = maxHeight > 0 ? maxHeight : 1e9;

	if (text) {
		lines = text.split("\n");
	}else {
		lines.clear();
	}

	curColor = mainColor = color;
	int maxLineWidth = 0;

	prevLineTextures = lineTextures;
	prevRects = rects;

	lineTextures.clear();
	rects.clear();

	pushStyle();
	for (size_t i = countLinesAtStart; i < lines.size(); ++i) {
		String line = lines[i];

		SDL_Rect lineRect = { 0, int(i * charHeight), 0, charHeight };
		lineRect.w = getLineWidth(line);

		const int INDENT = charWidth * 1.5;

		if (wordWrap && maxW > INDENT && lineRect.w > maxW - INDENT) {//Включён перенос строк по словам (и он возможен) и он нужен
			int n = line.size() - 1;
			while (n && (line[n] != ' ' || getLineWidth(line.substr(0, n - 1), true) >= maxW - INDENT)) {
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
		rects.push_back(lineRect);
	}
	popStyle();

	rect.w = std::max(maxWidth, maxLineWidth);
	int textHeight = charHeight * lines.size();

	//Если не уместились с этим размером шрифта
	if ((textHeight > maxH || maxLineWidth > maxW) && textSize > MIN_TEXT_SIZE) {
		setFont(fontName, textSize - 2, false);//Устанавливаем меньший шрифт
		lineTextures = prevLineTextures;
		rects = prevRects;
		setText(text, color);//И пытаемся ещё раз
		return;
	}

	//Не перерисовываем, если нарисовано будет ровно то же самое, что есть уже сейчас
	if (prevDrawFont == font && prevDrawTextColor == color && prevText == text && prevRects.size() == rects.size()) {
		lineTextures = prevLineTextures;
		rects = prevRects;
		return;
	}

	prevDrawFont = font;
	prevDrawTextColor = color;
	prevText = text;

	charOutNum = 0;
	charX = 0;

	for (size_t i = 0; i < prevLineTextures.size(); ++i) {
		SDL_Texture *textLine = prevLineTextures[i];
		SDL_DestroyTexture(textLine);
	}

	isBold = isItalic = isUnderline = isStrike = 0;
	rect.h = maxHeight > 0 ? maxHeight : textHeight;
	for (numLine = 0; numLine < lines.size(); ++numLine) {
		addText();
	}

	setAlign(hAlign, vAlign);
}

void Text::addText() {
	SDL_Rect lineRect = rects[numLine];

	if (!lineRect.w) {
		lineTextures.push_back(nullptr);
		charOutNum = 0;
		return;
	}

	if (charOutNum == 0) {
		GV::renderGuard.lock();
		SDL_Texture *texture = SDL_CreateTexture(GV::mainRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, lineRect.w, lineRect.h);

		if (texture) {
			SDL_SetRenderTarget(GV::mainRenderer, texture);
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
			SDL_SetRenderDrawColor(GV::mainRenderer, 0, 0, 0, 0);
			SDL_RenderClear(GV::mainRenderer);
			//Важно! SDL_RenderPresent не нужен!
			SDL_SetRenderTarget(GV::mainRenderer, nullptr);

			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
		}else {
			Utils::outMsg("SDL_CreateTexture", SDL_GetError());
		}
		GV::renderGuard.unlock();

		lineTextures.push_back(texture);
		if (!texture) {
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
					int newColor = String(type.substr(sepIndex + 1)).toInt(16);

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

	int charWidth;
	int charHeight;
	TTF_SizeUTF8(font, c.c_str(), &charWidth, &charHeight);
	SDL_Rect rect = {charX, 0, charWidth, charHeight};

	charX += charWidth;
//	charX -= bool(isBold || isItalic || isStrike || isUnderline) * c.size() * 1.5;

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
	SDL_Surface *rgbaSurface = SDL_ConvertSurfaceFormat(surfaceText, SDL_PIXELFORMAT_RGBA8888, 0);
	if (rgbaSurface) {
		SDL_FreeSurface(surfaceText);
		surfaceText = rgbaSurface;
	}else {
		Utils::outMsg("SDL_ConvertSurfaceFormat", SDL_GetError());
	}

	SDL_Texture *textureText;
	{
		std::lock_guard<std::mutex> g(GV::renderGuard);
		textureText = SDL_CreateTextureFromSurface(GV::mainRenderer, surfaceText);
	}
	SDL_FreeSurface(surfaceText);

	if (!textureText) {
		Utils::outMsg("SDL_CreateTextureFromSurface", SDL_GetError());
		return;
	}

	SDL_Texture *texture = lineTextures.size() ? lineTextures[lineTextures.size() - 1] : nullptr;
	if (texture) {
		std::lock_guard<std::mutex> g(GV::renderGuard);

		SDL_SetRenderTarget(GV::mainRenderer, texture);
		if (SDL_RenderCopy(GV::mainRenderer, textureText, nullptr, &rect)) {
			Utils::outMsg("SDL_RenderCopy", SDL_GetError());
		}
		SDL_SetRenderTarget(GV::mainRenderer, nullptr);
	}
	SDL_DestroyTexture(textureText);
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


	if (hAlign == "left") {
		for (size_t i = 0; i < rects.size(); ++i) {
			rects[i].x = 0;
			rects[i].y = i * charHeight + indentY;
		}
	}else {
		int maxWidth = this->maxWidth;
		for (size_t i = 0; i < rects.size(); ++i) {
			if (rects[i].w > maxWidth) {
				maxWidth = rects[i].w;
			}
		}

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


	double sinA = Utils::getSin(getGlobalRotate());
	double cosA = Utils::getCos(getGlobalRotate());

	for (size_t i = 0; i < rects.size(); ++i) {
		int x = rects[i].x;
		int y = rects[i].y;

		int rotX = std::round(x * cosA - y * sinA);
		int rotY = std::round(x * sinA + y * cosA);

		rects[i].x = rotX;
		rects[i].y = rotY;
	}
}

bool Text::checkAlpha(int x, int y) const {
	for (const SDL_Rect &rect : rects) {
		if (rect.x < x && x < rect.x + rect.w && rect.y < y && y < rect.y + rect.h) {
			return true;
		}
	}
	return false;
}

void Text::draw() const {
	if (!enable || globalAlpha <= 0) return;

	for (size_t i = 0; i < lineTextures.size(); ++i) {
		SDL_Texture *texture = lineTextures[i];
		if (!texture) continue;

		Uint8 intAlpha = Utils::inBounds(int(globalAlpha * 255), 0, 255);
		if (SDL_SetTextureAlphaMod(texture, intAlpha)) {
			Utils::outMsg("SDL_SetTextureAlphaMod", SDL_GetError());
		}

		SDL_Rect t = rects[i];
		t.x += globalX;
		t.y += globalY;

		if (!globalRotate) {
			if (SDL_RenderCopy(GV::mainRenderer, texture, nullptr, &t)) {
				Utils::outMsg("SDL_RenderCopy", SDL_GetError());
			}
		}else {
			SDL_Point center = { int(xAnchor), int(yAnchor) };
			if (SDL_RenderCopyEx(GV::mainRenderer, texture, nullptr, &t, globalRotate, &center, SDL_FLIP_NONE)) {
				Utils::outMsg("SDL_RenderCopyEx", SDL_GetError());
			}
		}
	}
}

Text::~Text() {
	font = nullptr;

	for (size_t i = 0; i < lineTextures.size(); ++i) {
		SDL_Texture *textLine = lineTextures[i];
		SDL_DestroyTexture(textLine);
	}
}
