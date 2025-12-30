init -10:
	style clicker_text is text:
		font 'Fregat_Bold'
		text_size 0.037
		color '#FFF'
		outlinecolor 0
	
	style clicker_text_error is clicker_text:
		color '#F00'
	
	
	style clicker_upgrade_button is textbutton:
		font 'Corbel'
		text_size 0.032
		color '#0F0'
		outlinecolor 0
		xsize 0.04 / get_from_hard_config('window_w_div_h', float)
		ysize 0.04
	
	style clicker_button is textbutton:
		font 'Fregat_Bold'
		text_size 0.032
		color '#FFF'
		outlinecolor 0
		size (0.2, 0.04)
