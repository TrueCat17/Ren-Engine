
# styles for menus (load, save, preferences)
# corner_sizes: None (auto), tuple (left, top, right, bottom) or one number for all sides

init -999:
	style menu_title is text:
		align (0.5, 0.02)
		text_size 0.1
		color '#FFF'
		outlinecolor '#000'
	
	# usual text in menu (used only in <preferences> screen)
	style menu_text is text:
		text_size 0.04
		text_size_min 20
		color '#000'
		text_align 'left'
	
	# usual button in menu
	style menu_button is textbutton:
		xsize 0.15
		ysize 0.055
		ysize_min 22
		corner_sizes None
		text_size 0.04
		text_size_min 16
	
	# vbox conteiner for all lines (rows) of prefs
	style prefs_content is vbox:
		spacing 0.015
		pos (0.62, 0.5)
		anchor 0.5
		xsize 0.6
	
	# hbox container for all elements in one line of prefs
	style prefs_line is hbox:
		spacing style.prefs_content.spacing
		xalign 0.5

init -998:
	style mods_button is menu_button:
		align (0.05, 0.95)
	style return_button is menu_button:
		align (0.95, 0.95)
	
	# common buttons on the left for load/save/prefs screens
	style page_button is menu_button:
		xsize 0.15
		text_size 0.04
		corner_sizes None
	
	# vbox container for page buttons
	style pages_vbox is vbox:
		spacing 0.01
		xpos 0.05
		xanchor 0
		yalign 0.5

init -997:
	# spec. for prefs screen
	style prefs_page_button is page_button:
		xsize 0.17
	
	# spec. for prefs screen
	style prefs_pages_vbox is pages_vbox:
		xpos 0.07

init -998:
	# button for bar actions (<minus> and <plus>)
	style bar_menu_button is menu_button:
		xsize style.menu_button.ysize / get_from_hard_config('window_w_div_h', float)
	
	# image params for <bar> element
	style bar is menu_button:
		xsize 0.3
		xsize_max 400
		ysize style.menu_button.ysize / 1.5
		corner_sizes None
	
	# button over checkbox and text description
	style bool_button is menu_button:
		xsize 0.4
		ysize style.menu_button.ysize * 1.2
		ysize_min style.menu_button.ysize_min + 20
		ground im.rect('#00000002')
		hover  im.rect('#00000010')
		corner_sizes 0
	
	# checkbox in bool_button
	style checkbox is image:
		xsize style.menu_button.ysize / get_from_hard_config('window_w_div_h', float)
		ysize style.menu_button.ysize
		size_min style.menu_button.ysize_min
		corner_sizes None
	
	# text params for description in <bool> element
	style bool_text is menu_text
	
	# hbox with checkbox and text, in (under) button
	style bool_hbox is hbox:
		spacing 0.01
		align 0.5



init -998:
	# all slots (save/load)
	style slots_content is vbox:
		pos (0.6, 0.5)
		anchor 0.5
		spacing 0.01 * get_from_hard_config('window_w_div_h', float)
	
	style slots_vbox is vbox:
		spacing 0.01 * get_from_hard_config('window_w_div_h', float)
	style slots_hbox is hbox:
		spacing 0.01
	
	# slot description (date/time)
	style slot_text is menu_text:
		xalign 0.5
		yalign 0.97
		color '#FFF'
		outlinecolor '#000'
		text_size 0.03
		text_size_min 15
	
	# transparent button over slot image
	style slot_button is menu_button:
		ground im.rect('#00000001')
		hover  im.rect('#00000001')

init -997:
	# hbox with buttons <load>/<save> and <delete>
	style slots_buttons is slots_hbox:
		xalign 0.5



init -997:
	# vbox container for mod buttons
	style mods_vbox is vbox:
		spacing 0.01
		align 0.5
	
	# button to start mod
	style mod_button is menu_button:
		xsize 0.27
		ysize 0.06
		ysize_min 24
		text_size 0.03
	
	# prev/next page of mods (enable if count of mods > gui.prefs_mods_in_page)
	style change_mod_page_button is mod_button:
		xsize 0.06 / get_from_hard_config('window_w_div_h', float)
		ysize 0.06
		size_min 24
	
	# text for "[current page] / [count of pages]"
	style mod_page_text is menu_text
	
	# hbox container for prev elements
	style change_mod_page_button_hbox is hbox:
		spacing 0.01
		xalign 0.5



init -998:
	# default bg for choice menu - almost transparent black
	# (see gui.choice_buttons_bg)
	style choice_buttons_bg is image:
		size 1.0
	
	# choice menu button
	style choice_button is menu_button:
		xsize 0 # 0 = auto
		xsize_min 150
		ysize 0.07
		text_size 0.05
	
	# vbox container for choice buttons
	style choice_buttons_vbox is vbox:
		spacing 0.03
		align 0.5



init -998:
	# default bg for quick menu - transparent
	# (see gui.quick_buttons_bg)
	style quick_buttons_bg is image:
		xsize 0 # 0 = auto
		ysize 0.03
		ysize_min style.menu_button.text_size_min + 2
		align (0.5, 1.0)
	
	style quick_buttons_hbox is hbox:
		align 0.5

init -997:
	# quick menu button
	style quick_button is menu_button:
		xsize 0 # 0 = auto
		ysize style.quick_buttons_bg.ysize
		ysize_min style.quick_buttons_bg.ysize_min
		ysize_max style.quick_buttons_bg.ysize_max
		text_size 0.02
		color '#DDD'
		outlinecolor '#000'
		hover_color '#DDD'
		hover_outlinecolor '#000'
		ground im.rect('#00000001')
		hover  im.rect('#00000001')
