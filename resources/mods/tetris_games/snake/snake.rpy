init -1 python:
	def tetris__snake_init():
		set_fps(tetris.snake_fps)
		
		tetris.pause = False
		tetris.exit  = False
		tetris.snake_last_update = 0
		
		tetris.snake_set_level(0)
	
	
	def tetris__snake_set_level(level):
		tetris.snake_score = 0
		tetris.snake_fail = False
		
		lines = tetris.get_level('snake', level)
		
		white = tetris.images['#FFF']
		black = tetris.images['#000']
		
		tetris.snake_walls = [[False] * tetris.width for i in range(tetris.height)]
		tetris.pixels      = [[white] * tetris.width for i in range(tetris.height)]
		
		for y in range(tetris.height):
			for x in range(tetris.width):
				src = lines[y][x]
				
				if src == 'X':
					tetris.snake_walls[y][x] = True
				elif src == 'P':
					snake_x, snake_y = x, y
				elif src != '.':
					out_msg('tetris.snake_set_level', 'Unexpected symbol %s in level %s, pos (%s, %s)', src, level, x, y)
		
		snake_start_len = 3
		tetris.snake_list = [(snake_x - snake_start_len + 1 + i, snake_y) for i in range(snake_start_len)]
		tetris.snake_dx, tetris.snake_dy = (+1, 0)
		tetris.next_snake_dx, tetris.next_snake_dy = tetris.snake_dx, tetris.snake_dy
		
		tetris.snake_set_food_to_random()
		
		tetris.snake_render()
	
	
	def tetris__snake_update():
		if tetris.snake_fail:
			return
		
		tetris.snake_dx, tetris.snake_dy = tetris.next_snake_dx, tetris.next_snake_dy
		
		to_x, to_y = tetris.snake_list[-1]
		to_x, to_y = tetris.normal_pos(to_x + tetris.snake_dx, to_y + tetris.snake_dy)
		
		if tetris.snake_walls[to_y][to_x] or (to_x, to_y) in tetris.snake_list[1:]:
			tetris.snake_fail = True
			return
		
		if to_x == tetris.snake_food_x and to_y == tetris.snake_food_y:
			tetris.snake_score += 1
			tetris.snake_set_food_to_random()
		else:
			tetris.snake_list.pop(0)
		
		tetris.snake_list.append((to_x, to_y))
		
		if tetris.snake_score == tetris.snake_score_for_next_level and not tetris.is_last_level:
			tetris.snake_set_level(tetris.snake_level + 1)
	
	
	def tetris__snake_render():
		pixels = tetris.pixels
		
		wall  = tetris.images['#000']
		empty = tetris.images['#FFF']
		body  = tetris.images['#F80']
		food  = tetris.images['#0A0']
		
		for y in range(tetris.height):
			line = tetris.snake_walls[y]
			pixels[y] = [(wall if line[x] else empty) for x in range(tetris.width)]
		
		for x, y in tetris.snake_list:
			pixels[y][x] = body
		
		if tetris.snake_food_x >= 0:
			pixels[tetris.snake_food_y][tetris.snake_food_x] = food
	
	
	def tetris__snake_set_food_to_random():
		free = []
		for y in range(tetris.height):
			for x in range(tetris.width):
				if tetris.snake_walls[y][x]:
					continue
				
				if (x, y) in tetris.snake_list:
					continue
				
				free.append((x, y))
		
		if not free:
			tetris.snake_food_x = tetris.snake_food_y = -1
		else:
			tetris.snake_food_x, tetris.snake_food_y = random.choice(free)
	
	
	def tetris__snake_on_up_press():
		if tetris.snake_dy != +1:
			tetris.next_snake_dx, tetris.next_snake_dy = 0, -1
	def tetris__snake_on_down_press():
		if tetris.snake_dy != -1:
			tetris.next_snake_dx, tetris.next_snake_dy = 0, +1
	def tetris__snake_on_left_press():
		if tetris.snake_dx != +1:
			tetris.next_snake_dx, tetris.next_snake_dy = -1, 0
	def tetris__snake_on_right_press():
		if tetris.snake_dx != -1:
			tetris.next_snake_dx, tetris.next_snake_dy = +1, 0
	
	
	def tetris__snake_restart_on_fail():
		if tetris.snake_fail:
			tetris.snake_set_level(0)


init 1 python:
	tetris.snake_fail = False
	
	tetris.snake_score = 0
	tetris.snake_score_for_next_level = 5
	
	tetris.snake_food_x = 0
	tetris.snake_food_y = 0
	
	tetris.snake_dx = 1
	tetris.snake_dy = 0
	
	tetris.snake_fps = 60
	tetris.snake_fps_update = 4


screen tetris_snake_screen:
	key 'W' action tetris.snake_on_up_press
	key 'S' action tetris.snake_on_down_press
	key 'A' action tetris.snake_on_left_press
	key 'D' action tetris.snake_on_right_press
	
	key 'SPACE'  action tetris.snake_restart_on_fail
	key 'RETURN' action tetris.snake_restart_on_fail
	
	if not tetris.pause and get_game_time() - tetris.snake_last_update > 1 / tetris.snake_fps_update:
		$ tetris.snake_last_update = get_game_time()
		$ tetris.snake_update()
		$ tetris.snake_render()
	
	use tetris_main_screen(tetris.text_xsize, style.tetris_btn.ysize + tetris.indent * 3)
	
	vbox:
		align (0.5, 1.0)
		
		hbox:
			spacing tetris.indent
			
			textbutton _('Continue' if tetris.pause else 'Pause'):
				style 'tetris_btn'
				action ToggleVariable('tetris.pause')
			textbutton _('Exit'):
				style 'tetris_btn'
				action 'tetris.exit = True'
		
		null ysize tetris.indent
	
	python:
		if tetris.is_last_level:
			screen_tmp = '%s: %s\n%s: %s' % (_('Score'), tetris.snake_score, _('Level'), tetris.snake_level)
		else:
			screen_tmp = '%s: %s / %s\n%s: %s' % (_('Score'), tetris.snake_score, tetris.snake_score_for_next_level, _('Level'), tetris.snake_level)
	text screen_tmp:
		font     'Arial'
		text_size 30
		color    '#0A0'
		text_align 'center'
		xsize      tetris.text_xsize
		yalign 0.5
	
	if tetris.snake_fail:
		text _('FAIL'):
			style 'tetris_state'


label tetris_snake_start:
	$ tetris.snake_init()
	
	window hide
	scene
	show screen tetris_snake_screen
	
	while not tetris.exit:
		pause 0.1
	
	hide screen tetris_snake_screen
	
	jump tetris_main
