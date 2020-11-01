# tg_snake_

init python:
	tg_snake_direction_changed = False
	
	tg_snake_exit = False
	tg_snake_fail = False
	
	tg_snake_score = 0
	tg_snake_food_x = 0
	tg_snake_food_y = 0
	
	tg_snake_dx = 1
	tg_snake_dy = 0
	
	tg_snake_fps = 60
	tg_snake_fps_update = 4
	
	tg_snake_color_empty = tg_hex_colors['white']
	tg_snake_color_wall  = tg_hex_colors['black']
	tg_snake_color_body  = tg_hex_colors['orange']
	tg_snake_color_food  = tg_hex_colors['green']
	
	
	def tg_snake_init():
		global tg_pause, tg_snake_exit, tg_snake_last_update, tg_snake_level
		
		set_fps(tg_snake_fps)
		
		tg_pause = False
		tg_snake_exit = False
		tg_snake_last_update = time.time()
		
		tg_snake_set_level(0)
	
	
	def tg_snake_set_level(level):
		global tg_width, tg_height, tg_color_field
		global tg_snake_field, tg_snake_fail
		global tg_snake_list, tg_snake_score, tg_snake_level, tg_snake_dx, tg_snake_dy
		
		tg_snake_score = 0
		tg_snake_level = level
		tg_snake_fail = False
		
		tg_width, tg_height = tg_snake_level_sizes[level]
		tg_snake_field = [False for i in xrange(tg_width * tg_height)]
		tg_color_field = [tg_snake_color_empty for i in xrange(tg_width * tg_height)]
		const_level = tg_snake_levels[level]
		
		for y in xrange(tg_height):
			for x in xrange(tg_width):
				index = y * tg_width + x
				src = const_level[index]
				
				if src == tg_snake_WALL:
					tg_snake_field[index] = True
				elif src == tg_snake_PLAYER:
					snake_x, snake_y = x, y
		
		tg_snake_list = [(snake_x - 2, snake_y), (snake_x - 1, snake_y), (snake_x, snake_y)]
		tg_snake_dx, tg_snake_dy = 1, 0
		
		tg_snake_set_food_to_random()
		
		tg_snake_render()
	
	
	def tg_snake_update():
		global tg_snake_last_update
		if time.time() - tg_snake_last_update < 1.0 / tg_snake_fps_update:
			return
		tg_snake_last_update = time.time()
		
		global tg_snake_fail, tg_snake_score, tg_snake_direction_changed
		tg_snake_direction_changed = False
		
		if tg_snake_fail or save_screenshotting:
			return
		
		to_x, to_y = tg_snake_list[-1]
		to_x = (to_x + tg_snake_dx) % tg_width
		to_y = (to_y + tg_snake_dy) % tg_height
		
		if tg_snake_field[to_y * tg_width + to_x] or (to_x, to_y) in tg_snake_list[1:]:
			tg_snake_fail = True
			return
		
		if to_x == tg_snake_food_x and to_y == tg_snake_food_y:
			tg_snake_score += 1
			tg_snake_set_food_to_random()
		else:
			tg_snake_list.pop(0)
		
		tg_snake_list.append((to_x, to_y))
		
		tg_snake_render()
	
	
	def tg_snake_render():
		for y in xrange(tg_height):
			for x in xrange(tg_width):
				index = y * tg_width + x
				if tg_snake_field[index]:
					tg_color_field[index] = tg_snake_color_wall
				else:
					tg_color_field[index] = tg_snake_color_empty
		
		for part_x, part_y in tg_snake_list:
			index = part_y * tg_width + part_x
			tg_color_field[index] = tg_snake_color_body
		
		index = tg_snake_food_y * tg_width + tg_snake_food_x
		tg_color_field[index] = tg_snake_color_food
	
	
	def tg_snake_set_food_to_random():
		global tg_snake_food_x, tg_snake_food_y
		
		head_x, head_y = tg_snake_list[-1]
		
		while True:
			while True:
				to_y = random.randint(1, tg_height - 1)
				if to_y != head_y:
					break
			
			to_x = random.randint(1, tg_width - 1)
			if (to_x, to_y) not in tg_snake_list and not tg_snake_field[to_y * tg_width + to_x]:
				break
		
		tg_snake_food_x = to_x
		tg_snake_food_y = to_y
	
	
	tg_snake_to_exit = SetVariable('tg_snake_exit', True)
	
	def tg_snake_on_left_press():
		global tg_snake_dx, tg_snake_dy, tg_snake_direction_changed
		if tg_snake_dx != 1 and not tg_snake_direction_changed:
			tg_snake_direction_changed = True
			tg_snake_dx = -1
			tg_snake_dy = 0
	def tg_snake_on_right_press():
		global tg_snake_dx, tg_snake_dy, tg_snake_direction_changed
		if tg_snake_dx != -1 and not tg_snake_direction_changed:
			tg_snake_direction_changed = True
			tg_snake_dx = 1
			tg_snake_dy = 0
	def tg_snake_on_up_press():
		global tg_snake_dx, tg_snake_dy, tg_snake_direction_changed
		if tg_snake_dy != 1 and not tg_snake_direction_changed:
			tg_snake_direction_changed = True
			tg_snake_dx = 0
			tg_snake_dy = -1
	def tg_snake_on_down_press():
		global tg_snake_dx, tg_snake_dy, tg_snake_direction_changed
		if tg_snake_dy != -1 and not tg_snake_direction_changed:
			tg_snake_direction_changed = True
			tg_snake_dx = 0
			tg_snake_dy = 1
	
	
	def tg_snake_restart_on_fail():
		if tg_snake_fail:
			tg_snake_set_level(0)


screen tg_snake_screen:
	key 'K_LEFT'  action tg_snake_on_left_press
	key 'K_RIGHT' action tg_snake_on_right_press
	key 'K_UP'    action tg_snake_on_up_press
	key 'K_DOWN'  action tg_snake_on_down_press
	key 'a' action tg_snake_on_left_press
	key 'd' action tg_snake_on_right_press
	key 'w' action tg_snake_on_up_press
	key 's' action tg_snake_on_down_press
	
	key 'K_SPACE'  action tg_snake_restart_on_fail
	key 'K_RETURN' action tg_snake_restart_on_fail
	
	if not tg_pause:
		$ tg_snake_update()
	
	use tg_main_screen
	
	hbox:
		align (0.5, 0.97)
		
		textbutton _('Restart')                           action tg_snake_init
		textbutton _('Continue' if tg_pause else 'Pause') action tg_change_pause_state
		textbutton _('Exit')                              action tg_snake_to_exit
	
	text (_('Score') + ': ' + str(tg_snake_score) + ' / 5' + '\n' + _('Level') + ': ' + str(tg_snake_level)):
		align    (0.05, 0.5)
		text_size 30
		color     0x00AA00
	
	if tg_snake_fail:
		text _('FAIL'):
			align    (0.5, 0.1)
			text_size 30
			color     0xFF0000


label tg_snake_start:
	$ tg_snake_init()
	
	window hide
	scene
	show screen tg_snake_screen
	
	while not tg_snake_exit:
		if tg_snake_score == 5:
			$ tg_snake_set_level(tg_snake_level + 1)
		pause 0.1
	
	hide screen tg_snake_screen
	
	jump tg_main

