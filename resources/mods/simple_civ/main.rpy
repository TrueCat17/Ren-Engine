init 10 python:
	set_fps(20)
	set_can_mouse_hide(False)
	
	start_screens = ['hotkeys', 'debug_screen', 'map', 'control', 'info']
	
	style.btn = Style(style.textbutton)
	style.btn.ground = im.rect('#08F')
	style.btn.hover  = im.rect('#09F')
	style.btn.text_size = 20
	style.btn.font = 'Arial'
	
	style.help_button = style.btn
	
	
	sc_map.bounds = {
		'food':  ( 4,  8),
		'wood':  ( 5, 20),
		'stone': (15, 40),
		'coal':  (10, 30),
		'metal': ( 5, 15),
	}
	sc_map.chances = {
		'food':    5,
		'wood':  200,
		'stone':   8,
		'coal':    5,
		'metal':   4,
	}
	
	sc_map.count_players = 1
	sc_map.count_start_builders = 1
	
	sc_map.generate(64, 64, block_size=8)
	
