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

	float alpha = 1.0;
	Uint32 color = 0;

	Uint8 fontStyle = 0;//TTF_STYLE_NORMAL

	bool enableOutline = false;
	Uint32 outlineColor = 0;

	bool operator==(const TextStyle &o) const {
		auto a = std::tie(  tag,   font,   fontName,   fontSize,   alpha,   color,   fontStyle,   enableOutline,   outlineColor);
		auto b = std::tie(o.tag, o.font, o.fontName, o.fontSize, o.alpha, o.color, o.fontStyle, o.enableOutline, o.outlineColor);
		return a == b;
	}
};


class TextField: public DisplayObject {
private:
	float hAlign = 0;
	float vAlign = 0;

	TextStyle prevMainStyle;
	std::vector<std::string> lines;
	std::vector<SDL_Rect> lineRects;

	std::vector<SDL_Rect> rects;
	std::vector<SurfacePtr> lineSurfaces;

	void setLine(size_t numLine, const std::string &line, std::vector<TextStyle> &styleStack);

public:
	TextStyle mainStyle;

	bool wordwrap = false;
	int maxWidth  = -1;
	int maxHeight = -1;

	float getHAlign() const { return hAlign; }
	float getVAlign() const { return vAlign; }
	void setAlign(float hAlign = 0, float vAlign = 0);

	void setFont(std::string fontName, float fontSize);
	void setText(const std::string &text);

	virtual bool transparentForMouse(int x, int y) const;
	virtual void draw() const;
};

#endif // TEXT_FIELD_H
