init -1 python:
	def tetris__life_copy_wiki_link():
		link = _('https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life')
		set_clipboard_text(link)
		notification.out(link.replace('%', '%%'))
	
	
	def tetris__life_init():
		set_fps(tetris.life_fps)
		
		tetris.width = 150
		tetris.height = 75
		
		tetris.pause = False
		tetris.exit = False
		
		tetris.life_alives = set()
		for i in range(tetris.width * tetris.height // 4):
			x = random.randint(0, tetris.width  - 1)
			y = random.randint(0, tetris.height - 1)
			tetris.life_alives.add((x, y))
		
		tetris.life_nears = {}
		for y in range(tetris.height):
			for x in range(tetris.width):
				nears = []
				for dy in (-1, 0, 1):
					for dx in (-1, 0, 1):
						if dx != 0 or dy != 0:
							tx, ty = tetris.normal_pos(x + dx, y + dy)
							nears.append((tx, ty))
				tetris.life_nears[(x, y)] = nears
		
		tetris.pixels = [['#FFF'] * tetris.width for i in range(tetris.height)]
		tetris.life_render()
	
	
	def tetris__life_update():
		next_alifes = set()
		
		life_nears  = tetris.life_nears
		life_alives = tetris.life_alives
		
		can_alife = life_alives.copy()
		for cell in life_alives:
			can_alife.update(life_nears[cell])
		
		for cell in can_alife:
			nears = life_nears[cell]
			
			count = 0
			for near in nears:
				if near in life_alives:
					count += 1
					if count == 4:
						break
			
			if count == 3 or (count == 2 and cell in life_alives):
				next_alifes.add(cell)
		
		tetris.life_alives = next_alifes
	
	def tetris__life_render():
		white = tetris.images['#FFF']
		black = tetris.images['#000']
		
		pixels = tetris.pixels = [[white] * tetris.width for i in range(tetris.height)]
		
		for x, y in tetris.life_alives:
			pixels[y][x] = black
		
		x = tetris.life_rect_x
		y = tetris.life_rect_y
		pixels[y][x] = tetris['life_black_rect' if pixels[y][x] == black else 'life_white_rect']
	
	
	def tetris__life_change_cell_state():
		selected_pos = (tetris.life_rect_x, tetris.life_rect_y)
		if selected_pos in tetris.life_alives:
			tetris.life_alives.remove(selected_pos)
		else:
			tetris.life_alives.add(selected_pos)
	
	
	def tetris__life_clear():
		tetris.life_alives.clear()
	
	
	def tetris__life_make_planer():
		tetris.life_clear()
		
		start_x = tetris.width // 2 - 1
		start_y = tetris.height // 2 - 1
		
		for x, y in ((0, 0), (1, 0), (2, 0), (2, 1), (1, 2)):
			tetris.life_alives.add((start_x + x, start_y + y))
	
	
	def tetris__life_make_r_pentamino():
		tetris.life_clear()
		
		start_x = tetris.width // 2 - 1
		start_y = tetris.height // 2 - 1
		
		for x, y in ((1, 0), (2, 0), (0, 1), (1, 1), (1, 2)):
			tetris.life_alives.add((start_x + x, start_y + y))
	
	
	def tetris__life_make_space_ship():
		tetris.life_clear()
		
		start_x = tetris.width // 2 - 1
		start_y = tetris.height // 2 - 1
		
		for x, y in ((1, 0), (2, 0), (3, 0), (4, 0), (0, 1), (4, 1), (4, 2), (0, 3), (3, 3)):
			tetris.life_alives.add((start_x + x, start_y + y))
	
	def tetris__life_make_taxi():
		tetris.life_clear()
		
		start_x = tetris.width // 2 - 10
		start_y = tetris.height // 2 - 1
		
		for i in range(9):
			tetris.life_alives.add((start_x + i * 2 + 1, start_y + 0))
		for i in range(10):
			tetris.life_alives.add((start_x + i * 2 + 0, start_y + 1))
	
	
	def tetris__life_on_up_press():
		tetris.life_rect_y = (tetris.life_rect_y - 1) % tetris.height
	def tetris__life_on_down_press():
		tetris.life_rect_y = (tetris.life_rect_y + 1) % tetris.height
	def tetris__life_on_left_press():
		tetris.life_rect_x = (tetris.life_rect_x - 1) % tetris.width
	def tetris__life_on_right_press():
		tetris.life_rect_x = (tetris.life_rect_x + 1) % tetris.width

init 1 python:
	tetris.life_rect_x = 0
	tetris.life_rect_y = 0
	
	tetris.life_fps = 20
	
	tetris.life_black_rect = im.composite((32, 32),
		(0, 0), im.rect('#F00', 32, 32),
		(3, 3), im.rect('#000', 32 - 3 * 2, 32 - 3 * 2),
	)
	tetris.life_white_rect = im.composite((32, 32),
		(0, 0), im.rect('#F00', 32, 32),
		(3, 3), im.rect('#FFF', 32 - 3 * 2, 32 - 3 * 2),
	)


screen tetris_life_screen:
	key 'W' action tetris.life_on_up_press
	key 'S' action tetris.life_on_down_press
	key 'A' action tetris.life_on_left_press
	key 'D' action tetris.life_on_right_press
	
	key 'SPACE'  action tetris.life_change_cell_state
	key 'RETURN' action tetris.life_change_cell_state
	
	key 'C' action tetris.life_clear
	key 'R' action tetris.life_init
	
	if not tetris.pause:
		$ tetris.life_update()
	$ tetris.life_render()
	
	use tetris_main_screen(0, style.tetris_life_btn.ysize * 2 + tetris.indent * 4)
	
	vbox:
		align (0.5, 1.0)
		
		hbox:
			xalign 0.5
			spacing tetris.indent
			
			$ btn_params = (
				('Wiki (copy)',                           tetris.life_copy_wiki_link),
				('Restart (random)',                      tetris.life_init),
				('Clear',                                 tetris.life_clear),
				('Continue' if tetris.pause else 'Pause', ToggleVariable('tetris.pause')),
				('Exit',                                 'tetris.exit = True'),
			)
			for text, action in btn_params:
				textbutton _(text):
					style 'tetris_life_btn'
					action action
		
		null ysize tetris.indent
		
		hbox:
			xalign 0.5
			spacing tetris.indent
			
			text (_('Make') + ': '):
				style 'tetris_life_btn'
				text_size style.tetris_life_btn.ysize - 4
				color '#FFF'
			
			$ btn_params = (
				('Planer',      tetris.life_make_planer),
				('SpaceShip',   tetris.life_make_space_ship),
				('R-Pentamino', tetris.life_make_r_pentamino),
				('Taxi',        tetris.life_make_taxi),
			)
			for text, action in btn_params:
				textbutton _(text):
					style 'tetris_life_btn'
					action action
		
		null ysize tetris.indent


label tetris_life_start:
	$ tetris.life_init()
	
	window hide
	scene
	show screen tetris_life_screen
	
	while not tetris.exit:
		pause 0.1
	
	hide screen tetris_life_screen
	$ tetris.life_nears.clear() # clear big data, needed only in <life> game
	
	jump tetris_main
