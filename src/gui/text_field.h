#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include <vector>

#include "gui/display_object.h"

typedef struct _TTF_Font TTF_Font;


struct TextStyle {
	std::string tag;

	TTF_Font *font = nullptr;
	std::string fontName;
	float fontSize = 0;

	Uint8 fontStyle = 0;//TTF_STYLE_NORMAL

	bool enableOutline = false;
	Uint32 outlineColor = 0;

	Uint32 color = 0;
	float alpha = 1.0;
};


class TextField: public DisplayObject {
private:
	std::string hAlign = "left";
	std::string vAlign = "top";

	std::vector<std::string> lines;
	std::vector<SDL_Rect> lineRects;

	std::vector<SDL_Rect> rects;
	std::vector<SurfacePtr> lineSurfaces;

	void setLine(size_t numLine, const std::string &line, std::vector<TextStyle> &stackStyles);

public:
	TextStyle mainStyle;

	bool wordwrap = false;
	int maxWidth  = -1;
	int maxHeight = -1;

	const std::string& getHAlign() const { return hAlign; }
	const std::string& getVAlign() const { return vAlign; }
	void setAlign(std::string hAlign = "left", std::string vAlign = "top");

	void setFont(std::string fontName, float fontSize);
	void setText(const std::string &text);

	virtual bool checkAlpha(int x, int y) const;
	virtual void draw() const;
};

#endif // TEXT_FIELD_H
