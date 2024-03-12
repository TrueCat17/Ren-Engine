# tg_life_

init python:
	tg_life_rect_x = 0
	tg_life_rect_y = 0
	
	tg_life_fps = 20
	
	tg_life_black_rect = im.composite((32, 32),
		(0, 0), im.rect('#F00', 32, 32),
		(3, 3), im.rect('#000', 32 - 3 * 2, 32 - 3 * 2),
	)
	tg_life_white_rect = im.composite((32, 32),
		(0, 0), im.rect('#F00', 32, 32),
		(3, 3), im.rect('#FFF', 32 - 3 * 2, 32 - 3 * 2),
	)
	
	
	def tg_life_open_wiki():
		import webbrowser
		webbrowser.open(_('https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life'))
	
	
	def tg_life_init():
		global tg_width, tg_height
		global tg_pause, tg_life_exit
		global tg_life_alives, tg_color_field
		global tg_life_nears
		
		set_fps(tg_life_fps)
		
		tg_width = 80
		tg_height = 40
		
		tg_pause = False
		tg_life_exit = False
		
		tg_life_alives = set()
		for i in range(tg_width * tg_height // 3):
			x = random.randint(0, tg_width  - 1)
			y = random.randint(0, tg_height - 1)
			tg_life_alives.add((x, y))
		
		tg_life_nears = {}
		for y in range(tg_height):
			for x in range(tg_width):
				nears = []
				for dy in (-1, 0, 1):
					for dx in (-1, 0, 1):
						if dx != 0 or dy != 0:
							cell = ((x + dx) % tg_width, (y + dy) % tg_height)
							nears.append(cell)
				tg_life_nears[(x, y)] = nears
		
		tg_color_field = ['#FFF'] * (tg_width * tg_height)
		tg_life_render()
	
	
	def tg_life_update():
		global tg_life_alives
		
		next_alifes = set()
		
		can_alife = set()
		for cell in tg_life_alives:
			can_alife.add(cell)
			
			nears = tg_life_nears[cell]
			for near in nears:
				can_alife.add(near)
		
		for cell in can_alife:
			nears = tg_life_nears[cell]
			
			life_nears = 0
			for near in nears:
				if near in tg_life_alives:
					life_nears += 1
					if life_nears == 4:
						break
			
			if life_nears == 3 or (life_nears == 2 and cell in tg_life_alives):
				next_alifes.add(cell)
		
		tg_life_alives = next_alifes
	
	def tg_life_render():
		for i in range(tg_width * tg_height):
			tg_color_field[i] = '#FFF'
		
		for x, y in tg_life_alives:
			index = y * tg_width + x
			tg_color_field[index] = '#000'
		
		rect_index = tg_life_rect_y * tg_width + tg_life_rect_x
		if tg_color_field[rect_index] == '#000':
			tg_color_field[rect_index] = tg_life_black_rect
		else:
			tg_color_field[rect_index] = tg_life_white_rect
	
	
	def tg_life_change_cell_state():
		rect = (tg_life_rect_x, tg_life_rect_y)
		if rect in tg_life_alives:
			tg_life_alives.remove(rect)
		else:
			tg_life_alives.add(rect) 
	
	
	def tg_life_clear():
		global tg_life_alives
		tg_life_alives = set()
	
	
	def tg_life_make_planer():
		tg_life_clear()
		
		start_x = tg_width // 2 - 1
		start_y = tg_height // 2 - 1
		
		tg_life_alives.add((start_x + 0, start_y + 0))
		tg_life_alives.add((start_x + 1, start_y + 0))
		tg_life_alives.add((start_x + 2, start_y + 0))
		tg_life_alives.add((start_x + 2, start_y + 1))
		tg_life_alives.add((start_x + 1, start_y + 2))
	
	
	def tg_life_make_r_pentamino():
		tg_life_clear()
		
		start_x = tg_width // 2 - 1
		start_y = tg_height // 2 - 1
		
		tg_life_alives.add((start_x + 1, start_y + 0))
		tg_life_alives.add((start_x + 2, start_y + 0))
		tg_life_alives.add((start_x + 0, start_y + 1))
		tg_life_alives.add((start_x + 1, start_y + 1))
		tg_life_alives.add((start_x + 1, start_y + 2))
	
	
	def tg_life_make_space_ship():
		tg_life_clear()
		
		start_x = tg_width // 2 - 1
		start_y = tg_height // 2 - 1
		
		tg_life_alives.add((start_x + 1, start_y + 0))
		tg_life_alives.add((start_x + 2, start_y + 0))
		tg_life_alives.add((start_x + 3, start_y + 0))
		tg_life_alives.add((start_x + 4, start_y + 0))
		tg_life_alives.add((start_x + 0, start_y + 1))
		tg_life_alives.add((start_x + 4, start_y + 1))
		tg_life_alives.add((start_x + 4, start_y + 2))
		tg_life_alives.add((start_x + 0, start_y + 3))
		tg_life_alives.add((start_x + 3, start_y + 3))
	
	def tg_life_make_taxi():
		tg_life_clear()
		
		start_x = tg_width // 2 - 10
		start_y = tg_height // 2 - 1
		
		for i in range(9):
			tg_life_alives.add((start_x + i * 2 + 1, start_y + 0))
		for i in range(10):
			tg_life_alives.add((start_x + i * 2 + 0, start_y + 1))
	
	
	def tg_life_on_left_press():
		global tg_life_rect_x
		tg_life_rect_x = (tg_life_rect_x - 1) % tg_width
	def tg_life_on_right_press():
		global tg_life_rect_x
		tg_life_rect_x = (tg_life_rect_x + 1) % tg_width
	def tg_life_on_up_press():
		global tg_life_rect_y
		tg_life_rect_y = (tg_life_rect_y - 1) % tg_height
	def tg_life_on_down_press():
		global tg_life_rect_y
		tg_life_rect_y = (tg_life_rect_y + 1) % tg_height


init:
	style tg_life_btn is textbutton:
		xsize_max 0.17


screen tg_life_screen:
	key 'K_LEFT'  action tg_life_on_left_press
	key 'K_RIGHT' action tg_life_on_right_press
	key 'K_UP'    action tg_life_on_up_press
	key 'K_DOWN'  action tg_life_on_down_press
	key 'a' action tg_life_on_left_press
	key 'd' action tg_life_on_right_press
	key 'w' action tg_life_on_up_press
	key 's' action tg_life_on_down_press
	
	key 'K_SPACE'  action tg_life_change_cell_state
	key 'K_RETURN' action tg_life_change_cell_state
	
	key 'c' action tg_life_clear
	key 'r' action tg_life_init
	
	if not tg_pause:
		$ tg_life_update()
	$ tg_life_render()
	
	$ tg_xindent = 0
	$ tg_yindent = style.textbutton.ysize * 2 + 17
	use tg_main_screen
	
	vbox:
		align (0.5, 1.0)
		spacing 5
		
		null size 1
		
		$ tmp_style = style.tg_life_btn
		
		hbox:
			xalign 0.5
			spacing 5
			
			$ btn_params = (
				('Wiki', tg_life_open_wiki),
				('Restart (random)', tg_life_init),
				('Clear', tg_life_clear),
				('Continue' if tg_pause else 'Pause', tg_change_pause_state),
				('Exit', SetVariable('tg_life_exit', True)),
			)
			for text, action in btn_params:
				textbutton _(text):
					style tmp_style
					ground tmp_style.get_ground()
					hover  tmp_style.get_hover()
					action action
		
		hbox:
			xalign 0.5
			spacing 5
			
			text (_('Make') + ': '):
				text_size 20
				outlinecolor 0
				text_align 'center'
				xsize tmp_style.xsize
				xsize_max tmp_style.xsize_max
				yalign 0.5
			
			$ btn_params = (
				('Planer', tg_life_make_planer),
				('SpaceShip', tg_life_make_space_ship),
				('R-Pentamino', tg_life_make_r_pentamino),
				('Taxi', tg_life_make_taxi),
			)
			for text, action in btn_params:
				textbutton _(text):
					style tmp_style
					ground tmp_style.get_ground()
					hover  tmp_style.get_hover()
					action action
		
		null size 1


label tg_life_start:
	$ tg_life_init()
	
	window hide
	scene
	show screen tg_life_screen
	
	while not tg_life_exit:
		pause 0.1
	
	hide screen tg_life_screen
	
	jump tg_main

