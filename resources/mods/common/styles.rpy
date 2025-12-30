init -999:
	style menu_text:
		text_size 0.03
		text_size_min 20
		color '#FFF'
	
	style menu_button:
		xsize 0.15
		ysize 0.045
		size_min 24
		
		font 'Fregat'
		text_size 0.03
		text_size_min 18
		
		hover_color 0
	
	style prefs_content:
		yalign 0.55

init -998:
	style page_button is menu_button
	style pages_vbox:
		xpos 0.17
		ypos 0.5
		yanchor 0.486
	
	style return_button:
		xpos 1 - 0.05 / get_from_hard_config('window_w_div_h', float)
		ypos 0.05
		anchor (1.0, 0)
		hover_color 0

init -997:
	style prefs_page_button is page_button
	
	style prefs_pages_vbox is pages_vbox:
		xpos 0.1
		yalign 0.55

init -998:
	style bar_button:
		text_size 0.04
		hover_color 0
	
	style bar:
		ysize style.menu_button.ysize



init -998:
	style slots_content:
		xpos 0.585
		ypos 0.16
		yanchor 0

init:
	style menu_title:
		alpha 0
	style mods_button:
		alpha 0
	
	$ style.slots_vbox.spacing /= 2
	$ style.slots_hbox.spacing /= 2
	
	style disabled_load_button is menu_button:
		alpha 0
	style disabled_delete_button is menu_button:
		alpha 0


init -998:
	style choice_button:
		ground 'images/gui/menu/choice/button.webp'
		hover  im.matrix_color('images/gui/menu/choice/button.webp', im.matrix.brightness(0.1))
		color 0
		hover_color '#00D'
	
	style choice_buttons_vbox:
		spacing 0.01


init -998:
	style quick_buttons_bg:
		xsize 0.283
		xsize_min 335
		ysize 0.031
		ysize_min 19
	
	style quick_buttons_hbox:
		yalign 0
		spacing 0.005

init -997:
	style quick_button:
		xsize 0.075
		xsize_min 90
		ysize 0.025
		ysize_min 16
		text_size 0.02
		text_size_min 14
		color 0
		outlinecolor None
		hover_color 0
		hover_outlinecolor None
		ground 'images/gui/dialogue/quick_menu_button.webp'
		hover  im.recolor('images/gui/dialogue/quick_menu_button.webp', 230, 230, 230)

init:
	style help_button:
		ground im.round_rect('#08F', 20, 20, 6)
		hover  im.round_rect('#F80', 20, 20, 6)
		font  'Calibri'
		color '#EEE'
		text_size 0.04
		text_size_min 18
	
	style help_slider_button is textbutton:
		ground im.round_rect('#CCC', 20, 20, 6)
		hover  im.round_rect('#AAA', 20, 20, 6)
		color       '#111'
		hover_color '#000'
	$ help.slider_button_style = 'help_slider_button'
	
	style history_title is text:
		color '#FFF'
		text_size 0.04
		text_size_min 16
		xpos 0.096
		xanchor 0.5
	
	style history_name_bg is image:
		xpos -0.015
		ypos 0.04
		xsize 0.06
		ysize gui.name_text_size
		ysize_min gui.name_text_size_min
		ysize_max gui.name_text_size_max
		corner_sizes -1
	
	style history_slider_button is textbutton:
		color       '#EEE'
		hover_color '#111'
	$ history.slider_button_style = 'history_slider_button'
	
	
	style input_button:
		ground 'images/gui/menu/input/button.webp'
		hover  im.matrix_color('images/gui/menu/input/button.webp', im.matrix.brightness(0.1))
		size (120, 25)
		font 'Fregat_Bold'
		text_size 20
		color '#FFF'
	
	style input_prompt:
		font 'Fregat'
