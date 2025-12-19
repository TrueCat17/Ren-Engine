#ifndef TEXT_H
#define TEXT_H

#include "container.h"

class TextField;

struct TextParams {
	const std::string *font = nullptr;

	Uint32 color = 0;
	Uint32 outlinecolor = 0;

	float halign;
	float valign;

	float size, size_min, size_max;
	bool size_is_float:1;
	bool size_min_is_float:1;
	bool size_max_is_float:1;

	bool set_font:1;
	bool set_color:1;
	bool set_outlinecolor:1;
	bool set_halign:1;
	bool set_valign:1;

	Uint8 font_style = 0;
	Uint8 set_font_style = 0;
};

class Text: public Container {
private:
	std::string prevText;
	uint32_t prevVisibleSymbols = 1e9;

public:
	bool prevParamsIsHover = false;
	bool curParamsIsHover = false;
	TextParams mainParams;
	TextParams hoverParams;

	TextField *tf;

	Text(Node *node, Screen *screen);
	virtual ~Text();

	virtual void updateSize();
	virtual void updateGlobal();
	virtual void updateTexture();
};

#endif // TEXT_H
