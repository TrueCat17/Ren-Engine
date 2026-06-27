init -1 python:
	def tetris__tetris_init():
		set_fps(tetris.tetris_fps)
		
		tetris.pause = False
		tetris.exit  = False
		tetris.tetris_last_update = get_game_time()
		
		tetris.tetris_init_time = get_game_time()
		
		tetris.tetris_fail = False
		tetris.tetris_score = 0
		tetris.tetris_fps_update = tetris.tetris_fps_update_start
		
		tetris.width = 10
		tetris.height = 20
		
		tetris.tetris_field = [[tetris.tetris_bg_cell] * tetris.width for i in range(tetris.height)]
		
		tetris.tetris_holded_figure = ''
		
		tetris.tetris_next_figures = []
		for i in range(tetris.tetris_count_next_figures):
			tetris.tetris_add_random_figure()
		tetris.tetris_set_next_figure()
		
		tetris.tetris_render()
	
	
	def tetris__tetris_add_random_figure():
		last = tetris.tetris_next_figures[-1] if tetris.tetris_next_figures else None
		res = last
		while res == last:
			res = random.choice(list(tetris.tetris_figures.keys()))
		tetris.tetris_next_figures.append(res)
	
	def tetris__tetris_check_lines():
		lines = 0
		for line in tetris.tetris_field.copy():
			if tetris.tetris_bg_cell not in line:
				tetris.tetris_field.remove(line)
				lines += 1
		
		tetris.tetris_score += tetris.tetris_award_for_lines[lines]
		tetris.tetris_fps_update += lines * tetris.tetris_acceleration_per_line
		
		while len(tetris.tetris_field) < tetris.height:
			tetris.tetris_field.insert(0, [tetris.tetris_bg_cell] * tetris.width)
	
	def tetris__tetris_set_next_figure():
		tetris.tetris_check_lines()
		
		tetris.tetris_holded_on_cur_figure = False
		
		tetris.tetris_cur_figure = tetris.tetris_next_figures.pop(0)
		tetris.tetris_set_start_figure_params()
		if not tetris.tetris_check_place_for_figure(
			tetris.tetris_cur_figure,
			tetris.tetris_cur_figure_x,
			tetris.tetris_cur_figure_y,
			tetris.tetris_cur_figure_rotation
		):
			tetris.tetris_fail = True
			tetris.tetris_fail_time = get_game_time()
		
		tetris.tetris_add_random_figure()
	
	def tetris__tetris_set_start_figure_params():
		tetris.tetris_cur_figure_rotation = 0
		figure = tetris.tetris_figures[tetris.tetris_cur_figure][0]
		figure_width = len(figure[0])
		tetris.tetris_cur_figure_x = round((tetris.width - figure_width) / 2)
		tetris.tetris_cur_figure_y = 0
		tetris.tetris_cur_figure_max_y = tetris.tetris_get_figure_max_y()
	
	def tetris__tetris_get_figure_max_y():
		y = tetris.tetris_cur_figure_y
		while tetris.tetris_check_place_for_figure(
			tetris.tetris_cur_figure,
			tetris.tetris_cur_figure_x, y + 1,
			tetris.tetris_cur_figure_rotation
		):
			y += 1
		return y
	
	def tetris__tetris_check_place_for_figure(name, x, y, rotation):
		rotations = tetris.tetris_figures[name]
		figure = rotations[rotation % len(rotations)]
		
		for dy, line in enumerate(figure):
			ty = y + dy
			
			for dx, point in enumerate(line):
				if point != '1': continue
				
				tx = x + dx
				if tx < 0 or tx >= tetris.width: return False
				if ty < 0 or ty >= tetris.height: return False
				
				if tetris.tetris_field[ty][tx] != tetris.tetris_bg_cell: return False
		
		return True
	
	
	def tetris__tetris_draw_cur_figure(pixels, hint):
		name = tetris.tetris_cur_figure
		
		x = tetris.tetris_cur_figure_x
		
		if hint:
			y = tetris.tetris_cur_figure_max_y
			image = tetris.tetris_hint_cell
		else:
			y = tetris.tetris_cur_figure_y
			color = tetris.tetris_figure_colors[name]
			image = tetris.images[color]
		
		rotations = tetris.tetris_figures[name]
		figure = rotations[tetris.tetris_cur_figure_rotation % len(rotations)]
		
		for dy, line in enumerate(figure):
			for dx, point in enumerate(line):
				if point == '1':
					pixels[y + dy][x + dx] = image
	
	
	def tetris__tetris_move_if_can(dx, dy):
		x = tetris.tetris_cur_figure_x
		y = tetris.tetris_cur_figure_y
		rotation = tetris.tetris_cur_figure_rotation
		
		def set_prev_params():
			tetris.tetris_cur_figure_x = x
			tetris.tetris_cur_figure_y = y
			tetris.tetris_cur_figure_rotation = rotation
		
		def correct():
			return tetris.tetris_check_place_for_figure(
				tetris.tetris_cur_figure,
				tetris.tetris_cur_figure_x,
				tetris.tetris_cur_figure_y,
				tetris.tetris_cur_figure_rotation
			)
		
		
		if dy == -1:
			tetris.tetris_cur_figure_rotation += tetris.tetris_add_rotation
			
			if not correct():
				min_dist = None
				min_dist_x = min_dist_y = None
				
				for ty in range(+2, -3, -1): # from bottom to up
					tetris.tetris_cur_figure_y = y + ty
					
					for tx in range(-2, +3): # from left to right
						tetris.tetris_cur_figure_x = x + tx
						
						if correct():
							dist = tx * tx + ty * ty
							if min_dist is None or dist < min_dist:
								min_dist = dist
								min_dist_x = tetris.tetris_cur_figure_x
								min_dist_y = tetris.tetris_cur_figure_y
				
				if min_dist:
					tetris.tetris_cur_figure_x = min_dist_x
					tetris.tetris_cur_figure_y = min_dist_y
				else:
					set_prev_params()
					return False
			
			tetris.tetris_cur_figure_max_y = tetris.tetris_get_figure_max_y()
		
		else:
			tetris.tetris_cur_figure_x += dx
			tetris.tetris_cur_figure_y += dy
			
			if not correct():
				set_prev_params()
				return False
			
			if dx:
				tetris.tetris_cur_figure_max_y = tetris.tetris_get_figure_max_y()
		
		return True
	
	
	def tetris__tetris_update():
		if not tetris.tetris_move_if_can(0, +1):
			tetris.tetris_draw_cur_figure(tetris.tetris_field, hint = False)
			tetris.tetris_set_next_figure()
		tetris.tetris_render()
	
	
	def tetris__tetris_render():
		tetris.pixels = [line.copy() for line in tetris.tetris_field]
		tetris.tetris_draw_cur_figure(tetris.pixels, hint = True)
		tetris.tetris_draw_cur_figure(tetris.pixels, hint = False)
	
	
	def tetris__tetris_move_from_keyboard(dx, dy):
		if tetris.pause:
			tetris.pause = False
		
		if tetris.tetris_move_if_can(dx, dy):
			tetris.tetris_render()
			if dy == +1:
				tetris.tetris_last_update = get_game_time()
	
	def tetris__tetris_full_down():
		tetris.tetris_cur_figure_y = tetris.tetris_cur_figure_max_y
		tetris.tetris_last_update = get_game_time()
		tetris.tetris_update()
	
	def tetris__tetris_hold_figure():
		if tetris.tetris_holded_on_cur_figure:
			return
		
		tetris.tetris_cur_figure, tetris.tetris_holded_figure = tetris.tetris_holded_figure, tetris.tetris_cur_figure
		
		if tetris.tetris_cur_figure:
			tetris.tetris_set_start_figure_params()
			tetris.tetris_holded_on_cur_figure = True
		else:
			tetris.tetris_set_next_figure()
		
		tetris.tetris_last_update = get_game_time()
		tetris.tetris_render()


init 1 python:
	tetris.tetris_fps = 20
	tetris.tetris_acceleration_per_line = 0.1
	
	# -1 for counterclockwise rotation
	# +1 for clockwise
	tetris.tetris_add_rotation = +1
	
	tetris.tetris_count_next_figures = 5
	tetris.tetris_award_for_lines = [0, 100, 300, 700, 1500]


screen tetris_tetris_figure(figure):
	$ screen.color = tetris.tetris_figure_colors[screen.figure]
	$ screen.image = tetris.images[screen.color]
	$ screen.rotations = tetris.tetris_figures[screen.figure]
	
	zoom tetris.zoom
	size (4, 2)
	
	vbox:
		align 0.5
		
		for line in screen.rotations[0]:
			if '1' not in line:
				continue
			
			hbox:
				for point in line:
					image screen.image:
						size 1
						alpha 1 if point == '1' else 0


screen tetris_tetris_screen:
	key 'W' action tetris.tetris_move_from_keyboard(0, -1)
	key 'S' action tetris.tetris_move_from_keyboard(0, +1)
	key 'A' action tetris.tetris_move_from_keyboard(-1, 0)
	key 'D' action tetris.tetris_move_from_keyboard(+1, 0)
	
	if not (tetris.pause or tetris.tetris_fail):
		if get_game_time() - tetris.tetris_init_time > 0.5:
			key 'RETURN' action tetris.tetris_full_down
			key 'SPACE'  action tetris.tetris_full_down
		
		key 'LEFT SHIFT'  action tetris.tetris_hold_figure
		key 'RIGHT SHIFT' action tetris.tetris_hold_figure
		
		if get_game_time() - tetris.tetris_last_update > 1 / tetris.tetris_fps_update:
			$ tetris.tetris_last_update = get_game_time()
			$ tetris.tetris_update()
	else:
		$ tetris.tetris_last_update = get_game_time()
	
	if tetris.tetris_fail and get_game_time() - tetris.tetris_fail_time > 0.5:
		key 'RETURN' action tetris.tetris_init
		key 'SPACE'  action tetris.tetris_init
	
	use tetris_main_screen(0, style.tetris_btn.ysize + tetris.indent * 3, xalign = 0.5)
	
	hbox:
		spacing tetris.zoom
		xalign 0.5
		ypos tetris.indent
		
		vbox:
			spacing tetris.zoom
			xsize tetris.zoom * 4
			
			text 'Shift':
				style 'tetris_tetris_figure_text'
			
			if tetris.tetris_holded_figure:
				use tetris_tetris_figure(tetris.tetris_holded_figure)
		
		null:
			xsize tetris.width * tetris.zoom
		
		vbox:
			spacing tetris.zoom
			xsize tetris.zoom * 4
			
			for figure in tetris.tetris_next_figures:
				use tetris_tetris_figure(figure)
			
			text str(tetris.tetris_score):
				style 'tetris_tetris_figure_text'
	
	
	if tetris.tetris_fail:
		text _('FAIL'):
			style 'tetris_state'
	
	vbox:
		align (0.5, 1.0)
		
		hbox:
			spacing tetris.indent
			
			if tetris.tetris_fail:
				textbutton _('New game'):
					style 'tetris_btn'
					action tetris.tetris_init
			else:
				textbutton _('Continue' if tetris.pause else 'Pause'):
					style 'tetris_btn'
					action ToggleVariable('tetris.pause')
			
			textbutton _('Exit'):
				style 'tetris_btn'
				action 'tetris.exit = True'
		
		null ysize tetris.indent


label tetris_tetris_start:
	menu:
		'Easy':
			$ tetris.tetris_fps_update_start = 1
		'Medium':
			$ tetris.tetris_fps_update_start = 2
		'Hard':
			$ tetris.tetris_fps_update_start = 4
	
	$ tetris.tetris_init()
	
	window hide
	scene
	show screen tetris_tetris_screen
	
	while not tetris.exit:
		pause 0.1
	
	hide screen tetris_tetris_screen
	jump tetris_main
