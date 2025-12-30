init -100 python:
	quick_menu = False
	gui.dialogue_menu_button_width = 0
	
	def tetris__update_sizes(xindent, yindent):
		xzoom = (get_stage_width()  - xindent) // tetris.width
		yzoom = (get_stage_height() - yindent) // tetris.height
		tetris.zoom = min(xzoom, yzoom)
	
	def tetris__normal_pos(x, y):
		return x % tetris.width, y % tetris.height
	
	def tetris__get_level(game, level):
		tetris[game + '_level'] = level
		
		levels = tetris[game + '_levels']
		tetris.is_last_level = (level + 1) not in levels
		
		level_data = levels[level]
		
		lines = [line.strip() for line in level_data.strip().split('\n')]
		
		tetris.height = len(lines)
		tetris.width = len(lines[0])
		for i, line in enumerate(lines):
			if len(line) != tetris.width:
				out_msg('tetris.get_level (%s)' % game, 'Level %s, line %s: expected %s symbols, got %s', level, i + 1, tetris.width, len(line))
		
		return lines


init python:
	tetris = SimpleObject()
	build_object('tetris')
	
	tetris.pause = False
	tetris.exit  = False
	
	tetris.indent = 10
	tetris.text_xsize = 170
	
	tetris.width = 1
	tetris.height = 1
	
	tetris.colors = ('green', 'orange', 'purple', 'yellow', 'blue', 'cyan', 'red')
	tetris.hex_colors = {
		'green' : '#0A0',
		'orange': '#F80',
		'purple': '#808',
		'yellow': '#DD0',
		'blue'  : '#00F',
		'cyan'  : '#0FF',
		'red'   : '#F00',
		'gray'  : '#888',
		'black' : '#000',
		'white' : '#FFF',
	}
	tetris.images = { color: im.rect(color) for color in tetris.hex_colors.values() }
	
	tetris.pixels = [['#0A0']]
	
	tetris.sides = (
		(-1, 0),
		(+1, 0),
		(0, -1),
		(0, +1),
	)


screen tetris_main_screen(xindent, yindent):
	key 'ESCAPE' action 'tetris.pause = True'
	
	$ tetris.update_sizes(screen.xindent, screen.yindent)
	xsize get_stage_width()  - screen.xindent
	ysize get_stage_height() - screen.yindent
	xalign 1.0
	ypos tetris.indent
	
	vbox:
		align 0.5
		zoom tetris.zoom
		
		for line in tetris.pixels:
			hbox:
				for image in line:
					image image:
						size 1


label start:
	jump tetris_main

label tetris_main:
	$ set_fps(60)
	
	scene bg room_screen
	
	'Choose a game! Management - WASD.'
	menu:
		'Life':
			jump tetris_life_start
		'Snake':
			jump tetris_snake_start
		'Tanks':
			jump tetris_tanks_start_usual
		'Tanks (campaign)':
			jump tetris_tanks_start_campaign
		''
		'Exit':
			'See you later!'
			
			$ start_mod('main_menu')
			while 1:
				pause 1
