#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include <vector>

#include "gui/display_object.h"

typedef struct _TTF_Font TTF_Font;


class TextField: public DisplayObject {
private:
	static const int MIN_TEXT_SIZE = 10;
	static const int MAX_TEXT_SIZE = 72;

	size_t charOutNum = 0;

	int charX = 0;
	int charWidth = 0;
	int charHeight = 0;

	std::string fontName;
	int textSize = 0;
	int originalTextSize = 0;

	Uint32 prevDrawTextColor = 0;
	TTF_Font *prevDrawFont = nullptr;
	std::string prevText;

	std::vector<SDL_Rect> rects;

	TTF_Font *font = nullptr;

	std::string hAlign = "left";
	std::string vAlign = "top";

	int maxWidth = -1;
	int maxHeight = -1;

	int getLineWidth(std::string text, bool resetPrevStyle = false);
	void addChars(std::string c, Uint32 color);

	Uint32 mainColor = 0;

	Uint32 curColor = 0;//Цвет
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

	std::vector<std::string> lines;
	size_t numLine = 0;
	void addText();

public:
	bool wordWrap = false;

	TextField();
	virtual ~TextField();

	std::string getHAlign() const { return hAlign; }
	std::string getVAlign() const { return vAlign; }
	void setAlign(std::string hAlign = "left", std::string vAlign = "top");

	std::string getText() const;
	void setText(const std::string &text, Uint32 color = 0x000000);

	int getMaxWidth() const;
	int getMaxHeight() const;
	void setMaxWidth(int width);
	void setMaxHeight(int height);

	void setFont(std::string fontName = "", int textSize = 20, bool setAsOriginal = true);
	std::string getFontName() const { return fontName; }
	int getFontSize() const { return textSize; }
	int getOriginalFontSize() const { return originalTextSize; }

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // TEXT_FIELD_H
