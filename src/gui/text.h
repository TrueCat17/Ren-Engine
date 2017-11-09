#ifndef TEXT_H
#define TEXT_H

#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "gui/display_object.h"
#include "utils/string.h"


class Text: public DisplayObject {
private:
	static const int MIN_TEXT_SIZE = 10;
	static const int MAX_TEXT_SIZE = 72;

	int charOutNum = 0;

	int charX = 0;
	int charWidth = 0;
	int charHeight = 0;

	String fontName;
	int textSize = 0;
	int originalTextSize = 0;

	int prevDrawTextColor = 0;
	TTF_Font *prevDrawFont = nullptr;
	String prevText;

	std::vector<SDL_Rect> rects;

	TTF_Font *font = nullptr;

	String hAlign = "left";
	String vAlign = "top";

	int maxWidth = -1;
	int maxHeight = -1;

	int getLineWidth(String text, bool resetPrevStyle = false);
	void addChars(String c, int color);

	int mainColor = 0;

	int curColor = 0;//Цвет
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

	std::vector<SurfacePtr> lineSurfaces;

	std::vector<String> lines;
	size_t numLine = 0;
	void addText();

public:
	static const char *defaultFontName;


	bool wordWrap = false;

	Text();
	virtual ~Text();

	String getHAlign() const { return hAlign; }
	String getVAlign() const { return vAlign; }
	void setAlign(String hAlign = "left", String vAlign = "top");

	String getText() const;
	void setText(const String &text, int color = 0x000000);

	int getMaxWidth() const;
	int getMaxHeight() const;
	void setMaxWidth(int width);
	void setMaxHeight(int height);

	void setFont(String fontName = "", int textSize = 20, bool setAsOriginal = true);
	String getFontName() const { return fontName; }
	int getFontSize() const { return textSize; }
	int getOriginalFontSize() const { return originalTextSize; }

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // TEXT_H
