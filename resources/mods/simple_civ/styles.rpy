init 10:
	style sc_main_menu_title is text:
		color '#FF0'
		outlinecolor 0
		text_size 0.07
		text_size_min 24
		xalign 0.5
	
	style sc_btn is textbutton:
		ground im.round_rect('#08F', 20, 20, 6)
		hover  im.round_rect('#F80', 20, 20, 6)
		font 'Arial'
		text_size 20
		color '#FFF'
		hover_color 0
	
	style sc_disabled_btn is sc_btn:
		ground im.round_rect('#048', 20, 20, 6)
		hover  im.round_rect('#048', 20, 20, 6)
		mouse False
	
	style sc_main_menu_btn is sc_btn:
		xsize 0.3
		xsize_min 300
		ysize 0.04
		ysize_min 24
		text_size 0.035
		text_size_min 20
		color '#FFF'
	
	style sc_main_menu_seed_btn is sc_main_menu_btn:
		ground im.round_rect('#F70', 20, 20, 6)
		hover  im.round_rect('#F80', 20, 20, 6)
		color 0
	
	style sc_main_menu_change_btn is sc_main_menu_seed_btn:
		xsize     style.sc_main_menu_btn.ysize / get_from_hard_config('window_w_div_h', float)
		xsize_min style.sc_main_menu_btn.ysize_min
	
	style sc_main_menu_text is sc_main_menu_change_btn:
		xsize -1
	
	
	style notification:
		ground im.rect('#333')
		hover  im.rect('#333')
		
		color       '#EE0'
		hover_color '#FF0'
