#include "screen_prop.h"


std::vector<String> screenPropNames = {
	"xpos",
	"ypos",
	"xanchor",
	"yanchor",
	"xalign",
	"yalign",
	"xsize",
	"ysize",
	"crop",
	"rotate",
	"alpha",

	"image_path",
	"ground_path",
	"hover_path",

	"rectangle_hotspot",

	"mouse",

	"box_spacing",
	"screen_modal",
	"screen_zorder",

	"key_name",
	"key_delay",
	"key_first_delay",

	"text_str",
	"text_font",
	"text_size",
	"text_align",
	"text_valign",
	"text_color",


	//not use, must be ALWAYS last
	"LAST"
};


std::vector<boost::python::object> screenPropObjects;
void initScreenPropObjects() {
	screenPropObjects.clear();
	for (size_t i = 0; i < COUNT_PROPS; ++i) {
		screenPropObjects.push_back(boost::python::object(screenPropNames[i].c_str()));
	}
}
