init:
	style tetris_btn is textbutton:
		ground im.round_rect('#EEE', 20, 20, 4)
		hover  im.round_rect('#08F', 20, 20, 4)
		color '#111'
	
	style tetris_life_btn is tetris_btn:
		xsize_max 0.17
	
	style tetris_state is text:
		align (0.5, 0.1)
		font     'Arial'
		text_size 30
		color    '#F00'
		outlinecolor 0
	
	style tetris_tanks_text is text:
		font     'Arial'
		text_size 25
		xalign 1.0
