# test_1623_shake__

init python:
	test_1623_shake__direction_changed = False
	
	test_1623_shake__exit = False
	test_1623_shake__fail = False
	
	test_1623_shake__score = 0
	test_1623_shake__food_x = 0
	test_1623_shake__food_y = 0
	
	test_1623_shake__dx = 1
	test_1623_shake__dy = 0
	
	test_1623_shake__fps = 4
	
	
	def test_1623_shake__init():
		global test_1623__pause, test_1623_shake__exit, test_1623_shake__level
		
		set_fps(test_1623_shake__fps)
		
		test_1623__pause = False
		test_1623_shake__exit = False
		
		test_1623_shake__set_level(0)
	
	
	def test_1623_shake__set_level(level):
		global test_1623__width, test_1623__height, test_1623__color_field
		global test_1623_shake__field, test_1623_shake__fail
		global test_1623_shake__list, test_1623_shake__score, test_1623_shake__level, test_1623_shake__dx, test_1623_shake__dy
		
		test_1623_shake__score = 0
		test_1623_shake__level = level
		test_1623_shake__fail = False
		
		test_1623__width, test_1623__height = test_1623_shake__level_sizes[level]
		test_1623_shake__field = [False for i in xrange(test_1623__width * test_1623__height)]
		test_1623__color_field = ['white' for i in xrange(test_1623__width * test_1623__height)]
		const_level = test_1623_shake__levels[level]
		
		for y in xrange(test_1623__height):
			for x in xrange(test_1623__width):
				index = y * test_1623__width + x
				src = const_level[index]
				
				if src == test_1623_shake__WALL:
					test_1623_shake__field[index] = True
				elif src == test_1623_shake__PLAYER:
					shake_x, shake_y = x, y
		
		test_1623_shake__list = [(shake_x - 2, shake_y), (shake_x - 1, shake_y), (shake_x, shake_y)]
		test_1623_shake__dx, test_1623_shake__dy = 1, 0
		
		test_1623_shake__set_food_to_random()
		
		test_1623_shake__render()
	
	
	def test_1623_shake__update():
		global test_1623_shake__list, test_1623_shake__fail, test_1623_shake__score, test_1623_shake__direction_changed
		
		test_1623_shake__direction_changed = False
		
		if test_1623_shake__fail or save_screenshotting:
			return
		
		to_x, to_y = test_1623_shake__list[-1]
		to_x = (to_x + test_1623_shake__dx) % test_1623__width
		to_y = (to_y + test_1623_shake__dy) % test_1623__height
		
		if test_1623_shake__field[to_y * test_1623__width + to_x] or (to_x, to_y) in test_1623_shake__list[1:]:
			test_1623_shake__fail = True
			return
		
		if to_x == test_1623_shake__food_x and to_y == test_1623_shake__food_y:
			test_1623_shake__score += 1
			test_1623_shake__set_food_to_random()
		else:
			test_1623_shake__list = test_1623_shake__list[1:]
		
		t = (to_x, to_y)
		test_1623_shake__list.append(t)
		
		test_1623_shake__render()
	
	
	def test_1623_shake__render():
		for y in xrange(test_1623__height):
			for x in xrange(test_1623__width):
				index = y * test_1623__width + x
				if test_1623_shake__field[index]:
					test_1623__color_field[index] = 'black'
				else:
					test_1623__color_field[index] = 'white'
		
		for part_x, part_y in test_1623_shake__list:
			index = part_y * test_1623__width + part_x
			test_1623__color_field[index] = 'orange'
		
		index = test_1623_shake__food_y * test_1623__width + test_1623_shake__food_x
		test_1623__color_field[index] = 'green'
	
	
	def test_1623_shake__set_food_to_random():
		global test_1623_shake__food_x, test_1623_shake__food_y
		
		head_x, head_y = test_1623_shake__list[-1]
		
		while True:
			while True:
				to_y = renpy.random.randint(1, test_1623__height - 1)
				if to_y != head_y:
					break
			
			to_x = renpy.random.randint(1, test_1623__width - 1)
			if (to_x, to_y) not in test_1623_shake__list and not test_1623_shake__field[to_y * test_1623__width + to_x]:
				break
		
		test_1623_shake__food_x = to_x
		test_1623_shake__food_y = to_y
	
	
	def test_1623_shake__to_exit():
		global test_1623_shake__exit
		test_1623_shake__exit = True
	
	
	def test_1623_shake__on_left_press():
		global test_1623_shake__dx, test_1623_shake__dy, test_1623_shake__direction_changed
		if test_1623_shake__dx != 1 and not test_1623_shake__direction_changed:
			test_1623_shake__direction_changed = True
			test_1623_shake__dx = -1
			test_1623_shake__dy = 0
	def test_1623_shake__on_right_press():
		global test_1623_shake__dx, test_1623_shake__dy, test_1623_shake__direction_changed
		if test_1623_shake__dx != -1 and not test_1623_shake__direction_changed:
			test_1623_shake__direction_changed = True
			test_1623_shake__dx = 1
			test_1623_shake__dy = 0
	def test_1623_shake__on_up_press():
		global test_1623_shake__dx, test_1623_shake__dy, test_1623_shake__direction_changed
		if test_1623_shake__dy != 1 and not test_1623_shake__direction_changed:
			test_1623_shake__direction_changed = True
			test_1623_shake__dx = 0
			test_1623_shake__dy = -1
	def test_1623_shake__on_down_press():
		global test_1623_shake__dx, test_1623_shake__dy, test_1623_shake__direction_changed
		if test_1623_shake__dy != -1 and not test_1623_shake__direction_changed:
			test_1623_shake__direction_changed = True
			test_1623_shake__dx = 0
			test_1623_shake__dy = 1
	
	
	def test_1623_shake__restart_on_fail():
		if test_1623_shake__fail:
			test_1623_shake__set_level(0)


screen test_1623_shake__screen:
	key 'K_LEFT' action test_1623_shake__on_left_press
	key 'K_RIGHT' action test_1623_shake__on_right_press
	key 'K_UP' action test_1623_shake__on_up_press
	key 'K_DOWN' action test_1623_shake__on_down_press
	key 'K_SPACE' action test_1623_shake__restart_on_fail
	key 'K_RETURN' action test_1623_shake__restart_on_fail
	key 'a' action test_1623_shake__on_left_press
	key 'd' action test_1623_shake__on_right_press
	key 'w' action test_1623_shake__on_up_press
	key 's' action test_1623_shake__on_down_press
	
	if not test_1623__pause:
		$ test_1623_shake__update()
	
	use test_1623__main_screen
	
	hbox:
		align (0.5, 0.97)
		
		textbutton 'ReStart' action test_1623_shake__init
		
		textbutton ("Continue" if test_1623__pause else "Pause"):
			action test_1623__change_pause_state
		
		textbutton 'Exit' action test_1623_shake__to_exit
	
	
	text ('Score: ' + str(test_1623_shake__score) + ' / 5' + '\n' + 'Level: ' + str(test_1623_shake__level)):
		align    (0.05, 0.5)
		text_size 30
		color     0x00AA00
	
	if test_1623_shake__fail:
		text 'DIED':
			align    (0.5, 0.1)
			text_size 30
			color     0xFF0000


label test_1623_shake__start:
	$ test_1623_shake__init()
	
	window hide
	scene bg black
	show screen test_1623_shake__screen
	
	while not test_1623_shake__exit:
		if test_1623_shake__score == 5:
			$ test_1623_shake__set_level(test_1623_shake__level + 1)
		
		pause 0.1
	
	hide screen test_1623_shake__screen
	
	
	jump test_1623__main

