# tg_

init -100 python:
	quick_menu = False
	gui.dialogue_menu_button_width = 0
	
	tg_pause = False
	
	tg_text_xsize = 170
	
	tg_width = 1
	tg_height = 1
	def tg_update_sizes():
		global tg_zoom, tg_btns_ysize
		xzoom = (get_stage_width()  - tg_xindent) // tg_width
		yzoom = (get_stage_height() - tg_yindent) // tg_height
		tg_zoom = min(xzoom, yzoom)
	
	
	tg_colors = ('green', 'orange', 'purple', 'yellow', 'blue', 'cyan', 'black')
	tg_hex_colors = {
		'green'  : '#00AA00',
		'orange' : '#FF8000',
		'purple' : '#7F007F',
		'yellow' : '#DDDD00',
		'blue'   : '#0000FF',
		'cyan'   : '#00FFFF',
		'gray'   : '#808080',
		'black'  : '#000000',
		'white'  : '#FFFFFF',
		'red'    : '#FF0000',
	}
	
	tg_color_field = ['#0A0']
	
	def tg_change_pause_state():
		global tg_pause
		tg_pause = not tg_pause


screen tg_main_screen:
	key 'ESCAPE' action SetVariable('tg_pause', True)
	
	$ tg_update_sizes()
	xsize get_stage_width()  - tg_xindent
	ysize get_stage_height() - tg_yindent
	xalign 1.0
	
	image im.rect('#FFF'):
		align 0.5
		zoom tg_zoom
		size (tg_width, tg_height)
	
	vbox:
		align 0.5
		zoom tg_zoom
		
		for i in range(tg_height):
			hbox:
				python:
					tg_line = []
					
					tg_count = 0
					for j in range(tg_width):
						tg_count += 1
						
						tg_color = tg_color_field[i * tg_width + j]
						
						tg_is_end = j == tg_width - 1
						if tg_is_end:
							tg_next_color = '#0000'
						else:
							tg_next_color = tg_color_field[i * tg_width + j + 1]
					
						tg_need_draw = tg_color != tg_next_color
						if tg_need_draw:
							tg_line.append((tg_color, tg_count))
							tg_count = 0
				
				for tg_color, tg_count in tg_line:
					$ cell_image = im.rect(tg_color) if tg_color.startswith('#') else tg_color
					image cell_image:
						size (tg_count, 1)


label start:
	jump tg_main

label tg_main:
	$ set_fps(20)
	
	scene bg room
	
	'Choose a game!'
	menu:
		'Life':
			jump tg_life_start
		'Snake':
			jump tg_snake_start
		'Tanks':
			jump tg_tanks_start_usual
		'Tanks (campaign)':
			jump tg_tanks_start_campaign
		''
		'Exit':
			'See you later!'

