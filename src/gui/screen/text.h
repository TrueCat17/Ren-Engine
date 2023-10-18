#ifndef TEXT_H
#define TEXT_H

#include "gui/screen/child.h"

class TextField;

struct TextParams {
	std::string font;

	Uint32 color = 0;
	Uint32 outlinecolor = 0;

	float halign;
	float valign;

	float size;
	bool sizeIsFloat;

	bool set_font = false;
	bool set_color = false;
	bool set_outlinecolor = false;
	bool set_size = false;
	bool set_halign = false;
	bool set_valign = false;

	Uint8 font_style = 0;
	Uint8 set_font_style = 0;
};

class Text: public Child {
private:
	std::string prevText;

	const bool hasOutlineColor;
	const bool hasHoverOutlineColor;

public:

	bool curParamsIsHover = false;
	TextParams mainParams;
	TextParams hoverParams;

	TextField *tf;

	Text(Node *node, Screen *screen);
	virtual ~Text();

	virtual void updateRect(bool needUpdatePos = true);
	virtual void updateGlobal();
};

#endif // TEXT_H
