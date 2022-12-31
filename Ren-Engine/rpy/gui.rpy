init -10000 python:
	def gui__min(a, b):
		if a is None: return b
		if b is None: return a
		return a if a < b else b
	def gui__max(a, b):
		if a is None: return b
		if b is None: return a
		return a if a > b else b
	
	def gui__get_int(name, **props):
		obj = props.get('obj', gui)
		default = props.get('default', None)
		
		if name.endswith('color'):
			res = color_to_int(obj[name])
			return res if res is not None else default
		
		
		is_x_coord = name.endswith('_xpos') or name.endswith('_width')
		max_value = props.get('max_value', get_stage_width() if is_x_coord else get_stage_height())
		
		res = get_absolute(obj[name], max_value) if obj[name] is not None else default
		min_res = obj[name + '_min'] and get_absolute(obj[name + '_min'], max_value)
		max_res = obj[name + '_max'] and get_absolute(obj[name + '_max'], max_value)
		
		res = gui.max(res, min_res)
		res = gui.min(res, max_res)
		return res
	
	
	def gui__bg(prop):
		res = gui[prop]
		if callable(res):
			res = res()
		return res
	
	
	build_object('gui')


init -1000 python:
	
	#    ------------
	#    |3 |4   |  |
	# |--------------------------------------------------|
	# |1 ----------------------------------------------  |
	# |  | 2                                          |  |
	# |  ----------------------------------------------  |
	# |--------------------------------------------------|
	#
	# 1. dialogue_box
	# 2. dialogue_text in dialogue_box
	# 3. name_box [in] dialogue_box (with default [yanchor = 1.0])
	# 4. name_text in name_box
	
	
	# if <text_size> is float, then multiplication by height of window will be applied
	# for example, 0.03 -> 0.03 * get_stage_height()
	# (of course, params <xpos> and <width> uses width of window, not height)
	
	# you can use suffixes *_min and *_max for positions and sizes
	
	# you can use function (not lambda!) instead of value for *_bg (background), *_ground and *_hover props
	#   (for example, to separate backgrounds for each time)
	
	
	# dialogue_box
	gui.dialogue_box_xalign = 0.5
	gui.dialogue_box_yalign = 0.99
	gui.dialogue_box_width  = None # None = auto
	gui.dialogue_box_height = 0.2
	gui.dialogue_box_height_min = 80
	gui.dialogue_box_bg = 'images/gui/dialogue/voice.png'
	
	# default prefix and suffix for text of all characters
	gui.dialogue_text_prefix = ''
	gui.dialogue_text_suffix = ''
	
	# dialogue text
	gui.dialogue_text_font = 'Calibri'
	gui.dialogue_text_size = 0.035
	gui.dialogue_text_size_min = 16
	gui.dialogue_text_size_max = 34
	gui.dialogue_text_color        = '#FF0'
	gui.dialogue_text_outlinecolor = None
	gui.dialogue_text_align = 'left'
	
	gui.dialogue_text_xpos = 15
	gui.dialogue_text_ypos = 10
	gui.dialogue_text_width  = None # None = auto: dialogue_box_width - 2 * dialogue_text_xpos
	gui.dialogue_text_height = None # None = auto: dialogue_box_height - 2 * dialogue_text_ypos
	
	
	# name_box
	# pos - relatively dialogue_box
	gui.name_box_xpos = 0.1
	gui.name_box_xpos_min = 50
	gui.name_box_ypos = 0
	gui.name_box_xanchor = 0.0
	gui.name_box_yanchor = 1.0
	gui.name_box_width = 0.2 # None = auto for each name
	gui.name_box_width_min = 150
	gui.name_box_height = 0.06
	gui.name_box_height_min = 24
	gui.name_box_bg = 'images/gui/dialogue/name.png'
	
	# default prefix and suffix for name of all characters
	gui.name_text_prefix = ''
	gui.name_text_suffix = ''
	
	# name_text
	gui.name_text_font = 'Calibri'
	gui.name_text_size = 0.04
	gui.name_text_size_min = 16
	gui.name_text_size_max = 40
	gui.name_text_color        = '#F00'
	gui.name_text_outlinecolor = None
	gui.name_text_xalign = 0.5
	gui.name_text_yalign = 0.8
	
	
	# dialogue prev and next buttons
	gui.dialogue_button_spacing = 5
	gui.dialogue_button_yalign = 0.5
	gui.dialogue_button_width  = 50
	gui.dialogue_button_height = 50
	gui.dialogue_prev_ground = 'images/gui/dialogue/to_prev.png'
	gui.dialogue_prev_hover  = None # None = auto
	gui.dialogue_next_ground = 'images/gui/dialogue/to_next.png'
	gui.dialogue_next_hover  = None # None = auto
	
	# dialogue pause button
	gui.dialogue_menu_button_xpos = 1.0
	gui.dialogue_menu_button_ypos = 0.0
	gui.dialogue_menu_button_xanchor =  70
	gui.dialogue_menu_button_yanchor = -20
	gui.dialogue_menu_button_width  = 50
	gui.dialogue_menu_button_height = 50
	gui.dialogue_menu_button_ground = 'images/gui/dialogue/to_menu.png'
	gui.dialogue_menu_button_hover  = None # None = auto
	
	# to disable dialogue prev/next or pause button, set their width/height to 0:
	#gui.dialogue_button_width = 0
	#gui.dialogue_menu_button_width = 0
	
	
	# props for nvl-mode
	gui.nvl_height = None # None = auto
	gui.nvl_spacing = 10 # enable if nvl_height is None
	
	gui.nvl_name_xpos = 0.25
	gui.nvl_name_ypos = 0
	gui.nvl_name_width = 150
	gui.nvl_name_xalign = 1.0
	
	gui.nvl_text_xpos = 0.27
	gui.nvl_text_ypos = 0
	gui.nvl_text_width = 0.6
	gui.nvl_text_xalign = 0.0
	
	# for text of narrator (with empty name)
	gui.nvl_thought_xpos = 0.1
	gui.nvl_thought_ypos = 0
	gui.nvl_thought_width = 0.8
	gui.nvl_thought_xalign = 0.0
	
	# NOT IMPLEMENTED NOW - uses usual (adv) choice menu in nvl-mode
	# choice menu button (see below) in nvl-mode
	gui.nvl_button_xpos = 0.5
	gui.nvl_button_xanchor = 0.5
	
	
	# history
	config.history_length = 50
	gui.history_height = None # None = auto
	gui.history_spacing = 5 # enable if history_height is None
	
	gui.history_name_xpos = 150
	gui.history_name_ypos = 0
	gui.history_name_width = 150
	gui.history_name_xalign = 1.0
	
	gui.history_text_xpos = 170
	gui.history_text_ypos = 0
	gui.history_text_width = 0.5
	gui.history_text_xalign = 0.0
	
	# for text of narrator (with empty name)
	gui.history_thought_xpos = 0.1
	gui.history_thought_ypos = 0
	gui.history_thought_width = 0.6
	gui.history_thought_xalign = 0.0
	
	
	# menu title
	gui.title_text_font = 'Calibri'
	gui.title_text_size = 0.1
	gui.title_text_color        = '#FFF'
	gui.title_text_outlinecolor = '#000'
	
	# interface in menus
	gui.interface_text_font = 'Calibri'
	gui.interface_text_size = 25
	gui.interface_text_color        = '#FFF'
	gui.interface_text_outlinecolor = '#000'
	gui.interface_text_xalign = 'left'
	
	
	# slots
	gui.file_slot_cols = 4
	gui.file_slot_rows = 3
	gui.slot_pages = 10
	
	gui.load_bg = 'images/gui/menu/main/back.png'
	gui.save_bg = 'images/gui/menu/main/back.png'
	
	gui.slot_hover    = 'images/gui/save_load/hover.png'
	gui.slot_selected = 'images/gui/save_load/selected.png'
	gui.slot_spacing = 0.01
	
	gui.slot_text_xpos = 5
	gui.slot_text_ypos = 5
	gui.slot_text_font = 'Calibri'
	gui.slot_text_size = 0.04
	gui.slot_text_color        = '#FFF'
	gui.slot_text_outlinecolor = '#000'
	
	
	# default button in save/load/preferences screens
	gui.button_width = 175
	gui.button_height = 25
	gui.button_ground = style.textbutton.ground
	gui.button_hover  = style.textbutton.hover
	gui.button_text_font = 'Calibri'
	gui.button_text_size = 15
	gui.button_text_color        = '#FFF'
	gui.button_text_outlinecolor = None
	gui.button_text_xalign = 0.5
	
	
	# choice menu button
	gui.choice_button_width = None # None = auto
	gui.choice_button_width_min = 150
	gui.choice_button_height = 34
	gui.choice_button_ground = style.textbutton.ground
	gui.choice_button_hover  = style.textbutton.hover
	gui.choice_button_text_font = 'Calibri'
	gui.choice_button_text_size = 20
	gui.choice_button_text_color        = '#FFF'
	gui.choice_button_text_outlinecolor = None
	gui.choice_button_text_xalign = 0.5
	
	gui.choice_spacing = 10
	
	
	# quick menu button
	gui.quick_button_width  = None # None = auto
	gui.quick_button_height = 20
	gui.quick_button_ground = im.rect('#00000002')
	gui.quick_button_hover  = im.rect('#00000002')
	gui.quick_button_text_font = 'Calibri'
	gui.quick_button_text_size = 16
	gui.quick_button_text_color        = '#DDD'
	gui.quick_button_text_outlinecolor = '#000'
	gui.quick_button_text_xalign = 0.5
	
	
	# page button in save/load/preferences screens
	gui.page_button_width = 0.10
	gui.page_button_height = 0.06
	gui.page_button_ground = style.textbutton.ground
	gui.page_button_hover  = style.textbutton.hover
	gui.page_button_text_font = 'Calibri'
	gui.page_button_text_size = 0.035
	gui.page_button_text_color        = '#FFF'
	gui.page_button_text_outlinecolor = None
	gui.page_button_text_xalign = 0.5
	gui.page_spacing = 0.007
	
	
	# prefs
	gui.prefs_bg = 'images/gui/menu/main/back.png'
	
	# props for bool btn
	gui.prefs_bool_width = 0.6
	gui.prefs_bool_width_max = 400
	gui.prefs_bool_height = 25
	gui.prefs_bool_ground = im.rect('#00000002')
	gui.prefs_bool_hover  = im.rect('#00000010')
	gui.prefs_bool_text_font = 'Calibri'
	gui.prefs_bool_text_size = 20
	gui.prefs_bool_text_color        = '#FFF'
	gui.prefs_bool_text_outlinecolor = '#000'
	gui.prefs_bool_text_xalign = 0.5
	
	gui.prefs_bar_width = 0.3
	gui.prefs_bar_width_max = 350
	gui.prefs_bar_height = 25
	gui.prefs_xindent = 0.1
	# see  text  props in gui.interface_text_*
	# see button props in gui.button_*
	
	
	gui.checkbox_yes = 'images/gui/std/checkbox/yes.png'
	gui.checkbox_no  = 'images/gui/std/checkbox/no.png'
	
	gui.bar_ground = 'images/gui/std/bar/ground.png'
	gui.bar_hover  = 'images/gui/std/bar/hover.png'
	
	gui.vbar_ground = im.rotozoom(gui.bar_ground, 90, 1)
	gui.vbar_hover  = im.rotozoom(gui.bar_hover , 90, 1)
