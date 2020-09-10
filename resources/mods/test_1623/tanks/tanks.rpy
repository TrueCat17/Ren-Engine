# test_1623_tanks__

init python:
	test_1623_tanks__frame_for_bullets = False
	test_1623_tanks__direction_changed = False
	test_1623_tanks__simpled = False
	test_1623_tanks__exit = False
	
	test_1623_tanks__walls = []
	test_1623_tanks__price_walls = []
	
	test_1623_tanks__limit_bullets = True
	test_1623_tanks__limit_moves = True
	
	test_1623_tanks__player = None
	test_1623_tanks__players = []
	test_1623_tanks__bullets = []
	
	test_1623_tanks__time = time.time()
	
	test_1623_tanks__fps = 10
	
	
	test_1623_tanks__positions = {
		'left':  (0, 4, 6, 1, 3, 0, 0, 2, 5),
		'up':	 (0, 1, 0, 2, 3, 4, 5, 0, 6),
		'right': (5, 2, 0, 0, 3, 1, 6, 4, 0),
		'down':  (6, 0, 5, 4, 3, 2, 0, 1, 0)
	}
	
	test_1623_tanks__colors = ('green', 'orange', 'purple', 'yellow', 'blue', 'cyan', 'black')
	test_1623_tanks__hex_colors = {
		'green'  : 0x00AA00,
		'orange' : 0xFF8000,
		'purple' : 0x7F007F,
		'yellow' : 0xDDDD00,
		'blue'   : 0x0000FF,
		'cyan'   : 0x00FFFF,
		'black'  : 0x808080
	}
	
	
	def test_1623_tanks__init():
		global test_1623_tanks__direction_changed, test_1623__pause, test_1623_tanks__exit, test_1623_tanks__level
		global test_1623_tanks__frame_for_bullets, test_1623_tanks__player
		global test_1623_tanks__limit_bullets, test_1623_tanks__limit_moves
		
		test_1623_tanks__frame_for_bullets = False
		test_1623_tanks__direction_changed = False
		test_1623__pause = False
		test_1623_tanks__exit = False
		
		test_1623_tanks__limit_bullets = True
		test_1623_tanks__limit_moves = True
		
		test_1623_tanks__level = 0
		test_1623_tanks__player = None
	
	
	def test_1623_tanks__set_level(level):
		global test_1623_tanks__player, test_1623_tanks__players, test_1623_tanks__bullets
		global test_1623_tanks__walls, test_1623_tanks__price_walls, test_1623__color_field
		global test_1623__width, test_1623__height, test_1623_tanks__time
		
		bot_exp = 0
		player = test_1623_tanks__player
		if player is not None:
			bot_level = test_1623_tanks__player.level
			bot_exp = 2 ** bot_level
		
		test_1623_tanks__time = time.time()
		
		test_1623_tanks__player = None
		test_1623_tanks__players = []
		test_1623_tanks__bullets = []
		
		test_1623__width, test_1623__height = test_1623_tanks__level_sizes[level]
		const_level = test_1623_tanks__levels[level]
		test_1623_tanks__walls = [0 for i in xrange(test_1623__width * test_1623__height)]
		test_1623_tanks__price_walls = [0 for i in xrange(test_1623__width * test_1623__height)]
		test_1623__color_field = ['white' for i in xrange(test_1623__width * test_1623__height)]
		
		for y in xrange(test_1623__height):
			for x in xrange(test_1623__width):
				index = y * test_1623__width + x
				src = const_level[index]
				
				if src == 0:
					if test_1623_tanks__get_near_count(const_level, x, y) > 0:
						test_1623_tanks__walls[index] = -1
					continue
				
				if src == test_1623_tanks__WALL:
					if not test_1623_tanks__simpled:
						v = max(int(test_1623_tanks__get_near_count(const_level, x, y) / 3), 2)
					else:
						v = 1
					
					test_1623_tanks__walls[index] = v
					test_1623_tanks__price_walls[index] = 2
				else:
					if src == test_1623_tanks__PLAYER:
						tank = Tank(False, x, y)
						
						if player is not None:
							tank.exp = player.exp
							tank.add_exp(0)
							
							tank.count_shots = player.count_shots
							tank.count_steps = player.count_steps
							tank.last_shot_time = player.last_shot_time
							tank.last_step_time = player.last_step_time
							tank.last_shots_time = player.last_shots_time
							tank.last_steps_time = player.last_steps_time
						
						test_1623_tanks__player = tank
						test_1623_tanks__players.append(test_1623_tanks__player)
					else:
						tank = Tank(True, x, y)
						tank.add_exp(bot_exp)
						test_1623_tanks__players.append(tank)
						
						if not test_1623_tanks__simpled:
							tank.team = src % 10
						else:
							tank.team = renpy.random.randint(0, 1e6)
	
	
	def test_1623_tanks__update_walls():
		for y in xrange(test_1623__height):
			for x in xrange(test_1623__width):
				index = y * test_1623__width + x
				src = test_1623_tanks__walls[index]
				
				if src <= 0:
					if test_1623_tanks__get_near_count(test_1623_tanks__walls, x, y) == 0:
						test_1623_tanks__walls[index] = 0
					else:
						test_1623_tanks__walls[index] = -1
	
	
	def test_1623_tanks__get_near_count(array, _x, _y):
		count = 0
		
		for dy in xrange(-1, 2):
			for dx in xrange(-1, 2):
				x = (_x + dx) % test_1623__width
				y = (_y + dy) % test_1623__height
				
				index = y * test_1623__width + x
				v = array[index]
				if v >= test_1623_tanks__WALL and v < test_1623_tanks__PLAYER:
					count += 1
		
		return count
	
	
	def test_1623_tanks__update():
		global test_1623__color_field, test_1623_tanks__direction_changed, test_1623_tanks__time
		global test_1623_tanks__bullets, test_1623_tanks__players, test_1623_tanks__frame_for_bullets
		
		test_1623_tanks__direction_changed = False
		test_1623_tanks__frame_for_bullets = not test_1623_tanks__frame_for_bullets
		test_1623_tanks__time = time.time()
		
		for bullet in test_1623_tanks__bullets:
			bullet.update()
		if not test_1623_tanks__frame_for_bullets:
			for tank in test_1623_tanks__players:
				tank.update()
				tank.x, tank.y = tank.to_x, tank.to_y
		
		test_1623_tanks__bullets = [bullet for bullet in test_1623_tanks__bullets if not bullet.need_to_delete]
		test_1623_tanks__players = [player for player in test_1623_tanks__players if not player.need_to_delete]
		
		for bullet in test_1623_tanks__bullets:
			bullet.x, bullet.y = bullet.to_x, bullet.to_y
		
		test_1623_tanks__render()
	
	
	def test_1623_tanks__render():
		for y in xrange(test_1623__height):
			for x in xrange(test_1623__width):
				index = y * test_1623__width + x
				
				is_wall = test_1623_tanks__walls[index]
				if is_wall > 0:
					test_1623__color_field[index] = 'black'
				else:
					test_1623__color_field[index] = 'white'
		
		for tank in test_1623_tanks__players:
			_x = tank.x - 1
			_y = tank.y - 1
			
			position = test_1623_tanks__positions[tank.rotation]
			hp = tank.hp
			
			for dy in xrange(3):
				for dx in xrange(3):
					x = (_x + dx) % test_1623__width
					y = (_y + dy) % test_1623__height
					
					d_index = dy * 3 + dx
					place = position[d_index]
					
					if place > 0:
						if place <= hp or test_1623_tanks__simpled:
							color = tank.color
						else:
							color = 'gray'
						
						index = y * test_1623__width + x
						test_1623__color_field[index] = color
		
		for bullet in test_1623_tanks__bullets:
			x, y = bullet.x, bullet.y
			
			index = y * test_1623__width + x
			test_1623__color_field[index] = 'red' if bullet.hp != 1 else 'yellow'
	
	def test_1623_tanks__check_point(x, y):
		for dy in xrange(-1, 2):
			for dx in xrange(-1, 2):
				_x = (x + dx) % test_1623__width
				_y = (y + dy) % test_1623__height
				index = _y * test_1623__width + _x
				if test_1623_tanks__walls[index] > 0:
					return False
		for tank in test_1623_tanks__players:
			dx, dy = tank.x - x, tank.y - y
			if abs(dx) < 3 and abs(dy) < 3:
				return False
		
		return True
	
	def test_1623_tanks__set_player_to_random():
		global test_1623_tanks__player
		
		x, y = renpy.random.randint(0, test_1623__width - 1), renpy.random.randint(0, test_1623__height - 1)
		ok = test_1623_tanks__check_point(x, y)
		while not ok:
			x, y = renpy.random.randint(0, test_1623__width - 1), renpy.random.randint(0, test_1623__height - 1)
			ok = test_1623_tanks__check_point(x, y)
		test_1623_tanks__player = Tank(False, x, y)
		test_1623_tanks__player.start_time = time.time()
		test_1623_tanks__players.append(test_1623_tanks__player)
	
	
	def test_1623_tanks__to_exit():
		global test_1623_tanks__exit
		test_1623_tanks__exit = True
	
	
	def test_1623_tanks__failed():
		for tank in test_1623_tanks__players:
			if tank.team == (test_1623_tanks__PLAYER % 10):
				return False
		return True
	
	
	def test_1623_tanks__won():
		for tank in test_1623_tanks__players:
			if tank.team != (test_1623_tanks__PLAYER % 10):
				return False
		for bullet in test_1623_tanks__bullets:
			if bullet.team != (test_1623_tanks__PLAYER % 10):
				return False
		return True
	
	
	def test_1623_tanks__change_direction(rotation, dx, dy):
		global test_1623_tanks__direction_changed
		if test_1623_tanks__direction_changed:
			return
		
		player = test_1623_tanks__player
		if not player:
			return
		
		if player.rotation == rotation:
			player.need_to_forward = True
		else:
			test_1623_tanks__direction_changed = True
			player.rotation, player.dx, player.dy = rotation, dx, dy
	
	
	def test_1623_tanks__on_left_press():
		test_1623_tanks__change_direction('left', -1, 0)
	def test_1623_tanks__on_right_press():
		test_1623_tanks__change_direction('right', 1, 0)
	def test_1623_tanks__on_up_press():
		test_1623_tanks__change_direction('up', 0, -1)
	def test_1623_tanks__on_down_press():
		test_1623_tanks__change_direction('down', 0, 1)
	def test_1623_tanks__player_fair():
		if test_1623_tanks__player:
			test_1623_tanks__player.need_to_fair = True


screen test_1623_tanks__screen:
	key 'K_LEFT' action test_1623_tanks__on_left_press
	key 'K_RIGHT' action test_1623_tanks__on_right_press
	key 'K_UP' action test_1623_tanks__on_up_press
	key 'K_DOWN' action test_1623_tanks__on_down_press
	key 'K_RETURN' action test_1623_tanks__player_fair
	key 'K_SPACE' action test_1623_tanks__player_fair
	key 'a' action test_1623_tanks__on_left_press
	key 'd' action test_1623_tanks__on_right_press
	key 'w' action test_1623_tanks__on_up_press
	key 's' action test_1623_tanks__on_down_press
	
	if not test_1623__pause:
		$ test_1623_tanks__update()
	
	use test_1623__main_screen
	
	hbox:
		align (0.5, 0.97)
		
		textbutton ('Continue' if test_1623__pause else 'Pause'):
			action test_1623__change_pause_state
		
		textbutton 'Exit' action test_1623_tanks__to_exit
	if not test_1623_tanks__simpled:
		vbox:
			align (0.02, 0.5)
			
			$ player = test_1623_tanks__player
			if player is not None:
				if test_1623_tanks__limit_bullets:
					$ info = '  Level:' + str(player.level) + '\n' + '  HP: ' + str(player.hp) + '\n' + '  Exp: ' + str(player.exp) + '\n' + '  Патронов осталось: ' + str(player.bullets_to_shorts)
				else:
					$ info = '  HP: ' + str(player.hp)
				
				text ('Вы: ' + '\n' + info):
					color test_1623_tanks__hex_colors[player.color]
					text_size 25
				null ysize 25
			
			$ test_1623_tanks__was_friends = False
			for tank in test_1623_tanks__players:
				if tank.team == (test_1623_tanks__PLAYER % 10) and tank is not test_1623_tanks__player:
					if not test_1623_tanks__was_friends:
						$ test_1623_tanks__was_friends = True
						text 'Друзья: ':
							color     0x00AA00
							text_size 25
					
					if test_1623_tanks__limit_bullets:
						$ info = '  Level: ' + str(tank.level) + '\n' + '  HP: ' + str(tank.hp)
					else:
						$ info = '  HP: ' + str(tank.hp)
					
					text info:
						color test_1623_tanks__hex_colors[tank.color]
					null ysize 10
			
			if test_1623_tanks__was_friends:
				null ysize 15
			else:
				null ysize 25
			
			$ test_1623_tanks__was_enemies = False
			for tank in test_1623_tanks__players:
				if tank.team != test_1623_tanks__PLAYER % 10:
					if not test_1623_tanks__was_enemies:
						$ test_1623_tanks__was_enemies = True
						text 'Враги: ':
							color	 '#AA0000'
							text_size 25
					
					text ('  Level: ' + str(tank.level) + '\n' + '  HP: ' + str(tank.hp)):
						color test_1623_tanks__hex_colors[tank.color]
					null ysize 10



label test_1623_tanks__start_usual:
	$ set_fps(60)
	
	$ test_1623_tanks__simpled = False
	
	jump test_1623_tanks__start

label test_1623_tanks__start_simple:
	$ test_1623_tanks__simpled = True
	
	jump test_1623_tanks__start


label test_1623_tanks__start:
	$ set_fps(60)
	$ test_1623_tanks__init()
	jump expression 'test_1623_tanks__dialogue_' + str(test_1623_tanks__level)

label test_1623_tanks__start_next_level:
	$ set_fps(test_1623_tanks__fps)
	
	$ test_1623_tanks__start_level_time = time.time()
	
	if test_1623_tanks__simpled:
		$ test_1623_tanks__level = renpy.random.randint(0, len(test_1623_tanks__levels) - 1)
	$ test_1623_tanks__set_level(test_1623_tanks__level)
	
	window hide
	scene bg black
	show screen test_1623_tanks__screen
	
	while not test_1623_tanks__exit:
		if test_1623_tanks__failed():
			if test_1623_tanks__simpled:
				$ test_1623_tanks__set_player_to_random()
			else:
				hide screen test_1623_tanks__screen
				$ set_fps(60)
				jump test_1623_tanks__fail
		
		if test_1623_tanks__won():
			if test_1623_tanks__simpled:
				$ test_1623_tanks__start_level_time = time.time()
				$ test_1623_tanks__level = renpy.random.randint(0, len(test_1623_tanks__levels) - 1)
				$ test_1623_tanks__set_level(test_1623_tanks__level)
			else:
				hide screen test_1623_tanks__screen
				$ test_1623_tanks__level += 1
				$ set_fps(60)
				jump expression 'test_1623_tanks__dialogue_' + str(test_1623_tanks__level)
		
		pause 0.1
	
	hide screen test_1623_tanks__screen
	jump test_1623__main
