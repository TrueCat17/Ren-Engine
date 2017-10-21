#include "screen_prop.h"


const char* screenPropNames[COUNT_PROPS + 1] = {
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
