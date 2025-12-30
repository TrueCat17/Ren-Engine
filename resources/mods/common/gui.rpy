init -1000 python:
	# see Ren-Engine/rpy/gui.rpy for full list of gui params and descriptions
	
	
	gui.name_text_font = 'Fregat_Bold'
	gui.name_text_size = 0.036
	gui.name_text_xalign = 0.45
	gui.name_text_yalign = 0.25
	gui.name_box_bg = 'images/gui/dialogue/name.webp'
	gui.name_box_xpos = 0
	del gui.name_box_xpos_min
	gui.name_box_xanchor = 0
	gui.name_box_yanchor = 0
	
	gui.name_box_width = 236 / 2560
	del gui.name_box_width_min
	gui.name_box_height = 72 / 1440
	
	
	gui.dialogue_text_font = 'Fregat'
	gui.dialogue_box_bg = 'images/gui/dialogue/box.webp'
	gui.dialogue_box_width = 1701 / 2560
	gui.dialogue_box_height = 306 / 1440
	
	gui.dialogue_text_color = '#000'
	gui.dialogue_text_xpos = 35 / 2560
	gui.dialogue_text_ypos = 70 / 1440
	gui.dialogue_text_height = gui.dialogue_box_height - gui.dialogue_text_ypos
	
	
	# to disable dialogue prev/next or pause button, set their width/height to 0:
	gui.dialogue_button_width = 0
	gui.dialogue_menu_button_width = 0
	
	
	gui.quick_buttons_bg = 'images/gui/dialogue/quick_menu_bg.webp'
	gui.quick_buttons_top_indent = 0
	
	
	gui.file_slot_cols = 3
	gui.file_slot_rows = 4
	gui.slot_pages = 7
	
	gui.main_bg = 'images/bg/screen_stars.png'
	
	gui.back_button_text = '<'
	gui.next_button_text = '>'
	
	gui.slot_selected = im.recolor('images/gui/save_load/selected.webp', 255, 128, 0)
	
	def slot_image_processing(image):
		w, h = get_image_size(image)
		mask = im.scale_without_borders(gui.slot_mask, w, h, gui.slot_corner_sizes, need_scale = True)
		return im.mask(image, mask, 256, 'a', alpha_image = 2)
	gui.slot_image_processing = slot_image_processing
	gui.slot_mask = 'images/gui/save_load/mask.webp'
	
	
	gui.nvl_bg = im.rect('#0008')
	
	gui.nvl_name_xpos = 0.19
	gui.nvl_text_xpos = 0.2
	
	gui.nvl_name_prefix = '{color=#EEE}'
	gui.nvl_name_suffix = ':{/color}'
	gui.nvl_text_prefix = '{color=#EEE}'
	gui.nvl_text_suffix = '{/color}'
	
	th.nvl_text_prefix = gui.nvl_text_prefix + '~ '
	th.nvl_text_suffix = ' ~' + gui.nvl_text_suffix
	
	
	gui.history_fog_alpha = 0.3
	gui.history_bg = 'images/gui/menu/history/bg.webp'
	
	gui.history_name_bg = 'images/gui/menu/history/name_bg.webp'
	gui.history_name_bg_style = 'history_name_bg'
	
	gui.history_name_xpos = 0.028
	gui.history_name_ypos = -0.002
	gui.history_name_width = -1
	gui.history_name_xalign = 0.5
	
	gui.history_text_xpos = 0.06
	gui.history_text_width = 0.55
	
	gui.history_thought_xpos = 0.03
	gui.history_thought_width = 0.58
	
	gui.history_name_prefix = '{color=#EEE}'
	gui.history_name_suffix = ':{/color}'
	gui.history_text_prefix = '{color=#EEE}'
	gui.history_text_suffix = '{/color}'
	
	th.history_text_prefix = gui.history_text_prefix + '~ '
	th.history_text_suffix = ' ~' + gui.history_text_suffix

init python:
	history.prev_params = (history.pre_start_props, history.start_props, history.end_props)
	
	history.pre_start_props = {
		'xpos': 0.5,
		'ypos': 0.5,
		'xanchor': 0.5,
		'yanchor': 0.5,
		'alpha': 0,
	}
	history.start_props = {
		'xpos': 0.5,
		'ypos': 0.5,
		'xanchor': 0.5,
		'yanchor': 0.5,
		'alpha': 1,
	}
	history.end_props = history.pre_start_props
	
	history.title_text = 'History'
	history.xsize = history.ysize * (1478 / 1163) / get_from_hard_config('window_w_div_h', float)
	history.padding = [15 / 1200, 40 / 675, 15 / 1200, 15 / 675]
	
	history.hover = im.rotozoom('images/gui/std/bar/hover_without_border.webp', 90)
	
	help.slider_ground = im.recolor(gui.vbar_ground, 0, 0, 0)
	help.slider_hover  = im.recolor(history.hover, 128, 128, 128)
	
	
	gui.vbar_hover_spacing = 4
	
	
	input.bg  = 'images/gui/menu/input/bg.webp'
	input.tf_bg = 'images/gui/menu/input/text_bg.webp'
	
	input.bg_width = 1488 / 4450 * 2
	input.bg_height = 380 / 2500 * 2
	
	input.bg_width_min = 587
	input.bg_height_min = 150
	
	input.bg_corner_sizes = 0
	
	input.bg_border_size = 0
	input.tf_bg_border_size = 0
	
	console.background_alpha = 0.5
