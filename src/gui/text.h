#ifndef TEXT_H
#define TEXT_H

#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "gv.h"

#include "gui/display_object.h"
#include "utils/string.h"


class Text: public DisplayObject {
private:
	static const int MIN_TEXT_SIZE = 10;
	static const int MAX_TEXT_SIZE = 48;

	int charOutNum = 0;

	int charX = 0;
	int charWidth;
	int charHeight;

	String fontName;
	int textSize;
	int originalTextSize;

	int prevDrawTextColor;
	TTF_Font *prevDrawFont = nullptr;
	String prevText;

	std::vector<SDL_Texture*> prevLineTextures;
	std::vector<SDL_Rect> prevRects;

	std::vector<SDL_Rect> rects;

	TTF_Font *font = nullptr;

	String align = "left";

	int maxWidth = -1;
	int maxHeight = -1;

	int getLineWidth(String text, bool resetPrevStyle = false);
	void addChars(String c, int color);

	int mainColor;

	int curColor;//Цвет
	int isBold = 0;//Жирный
	int isItalic = 0;//Курсив
	int isUnderline = 0;//Подчёркнутый
	int isStrike = 0;//Зачёркнутый

	std::vector<int> bolds;
	std::vector<int> italics;
	std::vector<int> underlines;
	std::vector<int> strikes;

	void pushStyle();
	void popStyle();
	void resetStyle();

	std::vector<SDL_Texture*> lineTextures;

	std::vector<String> lines;
	size_t numLine = 0;
	void addText();

public:
	static const char *defaultFontName;


	bool wordWrap = false;

	virtual int getMinX() const;
	virtual int getMinY() const;
	virtual int getMaxX() const;
	virtual int getMaxY() const;

	Text(String fontName = "", int textSize = 20);
	virtual ~Text();

	void setFont(String fontName = "", int textSize = 20, bool setAsOriginal = true);
	void setAlign(String align = "left");

	String getText() const;
	void setText(const String &text, int color = 0x000000);

	int getMaxWidth() const;
	int getMaxHeight() const;
	void setMaxWidth(int width);
	void setMaxHeight(int height);

	String getFontName() const { return fontName; }
	int getFontSize() const { return textSize; }
	int getOriginalFontSize() const { return originalTextSize; }

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // TEXT_H
