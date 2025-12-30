init:
	$ rpg_btn_ground = 'mods/rpg/gui/btn/ground.webp'
	$ rpg_btn_hover  = 'mods/rpg/gui/btn/hover.webp'
	
	style inventory_use_button:
		ground rpg_btn_ground
		hover  rpg_btn_hover
	style inventory_throw_button:
		ground rpg_btn_ground
		hover  rpg_btn_hover
	
	$ gui.inventory_cell_usual_over    = im.color('images/gui/save_load/selected.webp', '#888')
	$ gui.inventory_cell_selected_over = im.color('images/gui/save_load/selected.webp', '#08F')
	
	style choice_button:
		ground rpg_btn_ground
		hover  rpg_btn_hover
		color       '#FFF'
		hover_color '#FFF'
	
	style choice_buttons_vbox:
		spacing 0.03
	
	
	style quick_buttons_bg:
		xsize 0 # 0 = auto
		ysize 0.03
		ysize_min style.menu_button.text_size_min + 2
	
	style quick_buttons_hbox is hbox:
		align 0.5
	
	style quick_button:
		xsize 0 # 0 = auto
		xsize_min 0
		ysize style.quick_buttons_bg.ysize
		ysize_min style.quick_buttons_bg.ysize_min
		ysize_max style.quick_buttons_bg.ysize_max
		font 'Calibri'
		text_size 0.02
		text_size_min 16
		color        '#DDD'
		outlinecolor '#000'
		hover_color        '#DDD'
		hover_outlinecolor '#000'
		ground im.rect('#00000001')
		hover  im.rect('#00000001')
	
	
	style rpg_history_btn is textbutton:
		ground rpg_btn_ground
		hover  rpg_btn_hover



init python:
	gui.choice_button_text_font = 'FixEx3'
	
	gui.dialogue_menu_button_ground = 'mods/rpg/gui/dialogue/to_menu.webp'
	gui.dialogue_menu_button_width  = 50
	gui.dialogue_menu_button_height = 50
	
	gui.dialogue_box_bg = 'mods/rpg/gui/dialogue/voice.webp'
	gui.dialogue_box_width = None
	gui.dialogue_box_height = 0.2
	
	gui.dialogue_text_font = 'FixEx3'
	gui.dialogue_text_color = '#FF0'
	gui.dialogue_text_ypos = 10
	
	gui.name_box_bg = 'mods/rpg/gui/dialogue/name.webp'
	gui.name_box_width  = 0.2
	gui.name_box_height = 0.06
	gui.name_box_xanchor = 0.0
	gui.name_box_yanchor = 1.0
	gui.name_box_xpos = 0.1
	gui.name_box_ypos = 0
	
	gui.name_text_font = 'FixEx3'
	gui.name_text_yalign = 0.8
	
	gui.dialogue_prev_ground = 'mods/rpg/gui/dialogue/to_prev.webp'
	gui.dialogue_next_ground = 'mods/rpg/gui/dialogue/to_next.webp'
	gui.dialogue_button_width  = 50
	gui.dialogue_button_height = 50
	
	
	gui.quick_buttons_bg = im.rect('#0000')
	gui.quick_buttons_bg_without_window = im.rect('#0000')
	gui.quick_buttons_top_indent = 5
	quick_menu_screen.items = quick_menu_screen.old_items
	quick_menu_screen.prefixes.clear()
	
	
	history.pre_start_props, history.start_props, history.end_props = history.prev_params
	
	gui.history_bg = im.rect('#181818BB')
	history.title_text = ''
	history.xsize = None
	history.ysize = 0.95
	history.padding = [15, 10, 15, 10]
	
	history.slider_ground = im.rotozoom('mods/rpg/gui/bar/ground.webp', 90, 1)
	history.slider_hover  = im.rotozoom('mods/rpg/gui/bar/hover.webp',  90, 1)
	history.slider_button_style = 'rpg_history_btn'
	
	gui.history_name_bg = None
	gui.history_name_xpos = 0.125
	gui.history_name_ypos = 0
	gui.history_name_width = 0.125
	gui.history_name_xalign = 1.0
	
	gui.history_text_xpos = 0.13
	gui.history_text_width = 0.5
	
	gui.history_thought_xpos = 0.08
	gui.history_thought_width = 0.6
	
	gui.history_name_prefix = ''
	gui.history_name_suffix = ':'
	gui.history_text_prefix = ''
	gui.history_text_suffix = ''
	th.history_text_prefix = '~ '
	th.history_text_suffix = ' ~'
	
	
	tutorial = Character('', what_color = '#F80',
		what_prefix = '{color=#F00}[{/color}',
		what_suffix = '{color=#F00}]{/color}',
	)
	
	sm.name_text_color = '#DDA'
