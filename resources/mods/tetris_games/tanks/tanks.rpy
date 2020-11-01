# tg_tanks_

init python:
	
	# times
	tg_tanks_WAIT_FOR_HP_START = 3.0
	tg_tanks_WAIT_FOR_HP_NEXT  = 1.0
	tg_tanks_NO_DAMAGE = 1.5
	
	tg_tanks_frame_for_bullets = False
	tg_tanks_direction_changed = False
	tg_tanks_campaign = False
	tg_tanks_exit = False
	
	tg_tanks_walls = []
	
	tg_tanks_player = None
	tg_tanks_players = []
	tg_tanks_bullets = []
	
	tg_tanks_fps = 10
	
	
	tg_tanks_rotations = {
		'left': (
			0, 4, 6,
			1, 3, 0,
			0, 2, 5
		),
		'up': (
			0, 1, 0,
			2, 3, 4,
			5, 0, 6
		),
		'right': (
			5, 2, 0,
			0, 3, 1,
			6, 4, 0
		),
		'down': (
			6, 0, 5,
			4, 3, 2,
			0, 1, 0
		),
	}
	
	
	def tg_tanks_init():
		global tg_tanks_frame_for_bullets, tg_tanks_direction_changed
		global tg_pause, tg_tanks_exit, tg_tanks_level, tg_tanks_state
		
		tg_tanks_frame_for_bullets = False
		tg_tanks_direction_changed = False
		
		tg_pause = False
		tg_tanks_exit = False
		tg_tanks_level = -1
		tg_tanks_state = ''
	
	
	def tg_tanks_set_level(level):
		global tg_tanks_player, tg_tanks_players, tg_tanks_bullets
		global tg_tanks_walls, tg_color_field
		global tg_tanks_level, tg_width, tg_height, tg_tanks_start_level_time
		
		tg_tanks_start_level_time = time.time()
		
		tg_tanks_player = None
		tg_tanks_players = []
		tg_tanks_bullets = []
		
		tg_tanks_level = level
		tg_width, tg_height = tg_tanks_level_sizes[level]
		size = tg_width * tg_height
		const_level = tg_tanks_levels[level]
		tg_tanks_walls = [0] * size
		tg_color_field = ['#FFF'] * size
		
		for y in xrange(tg_height):
			for x in xrange(tg_width):
				index = y * tg_width + x
				src = const_level[index]
				
				if src == 0:
					if tg_tanks_get_near_count(const_level, x, y) > 0:
						tg_tanks_walls[index] = -1
					continue
				
				if src == tg_tanks_WALL:
					if tg_tanks_campaign:
						v = 3 if tg_tanks_get_near_count(const_level, x, y) > 5 else 2
					else:
						v = 1
					tg_tanks_walls[index] = v
					continue
				
				team = src % 10 if tg_tanks_campaign else len(tg_tanks_players)
				tank = Tank(x, y, team)
				tg_tanks_players.append(tank)
				
				if src == tg_tanks_PLAYER:
					tank.is_bot = False
					tg_tanks_player = tank
	
	
	def tg_tanks_set_level_next():
		if tg_tanks_campaign:
			num = tg_tanks_level + 1
		else:
			num = random.randint(0, len(tg_tanks_levels) - 1)
		tg_tanks_set_level(num)
	
	
	def tg_tanks_update_walls():
		for y in xrange(tg_height):
			for x in xrange(tg_width):
				index = y * tg_width + x
				src = tg_tanks_walls[index]
				
				if src <= 0:
					if tg_tanks_get_near_count(tg_tanks_walls, x, y) == 0:
						tg_tanks_walls[index] = 0
					else:
						tg_tanks_walls[index] = -1
	
	
	def tg_tanks_get_near_count(array, _x, _y):
		count = 0
		
		for dy in xrange(-1, 2):
			for dx in xrange(-1, 2):
				x = (_x + dx) % tg_width
				y = (_y + dy) % tg_height
				
				index = y * tg_width + x
				v = array[index]
				if v >= tg_tanks_WALL and v < tg_tanks_PLAYER:
					count += 1
		
		return count
	
	
	def tg_tanks_update():
		global tg_tanks_frame_for_bullets, tg_tanks_direction_changed
		global tg_tanks_bullets, tg_tanks_players
		
		tg_tanks_frame_for_bullets = not tg_tanks_frame_for_bullets
		
		for bullet in tg_tanks_bullets:
			bullet.update()
		
		if not tg_tanks_frame_for_bullets:
			tg_tanks_direction_changed = False
			for tank in tg_tanks_players:
				tank.update()
				tank.x, tank.y = tank.to_x, tank.to_y
		
		tg_tanks_bullets = filter(lambda bullet: not bullet.need_to_delete, tg_tanks_bullets)
		tg_tanks_players = filter(lambda player: not player.need_to_delete, tg_tanks_players)
		
		for bullet in tg_tanks_bullets:
			bullet.x, bullet.y = bullet.to_x, bullet.to_y
		
		tg_tanks_render()
	
	
	def tg_tanks_render():
		for i in xrange(tg_width * tg_height):
			is_wall = tg_tanks_walls[i]
			if is_wall > 0:
				tg_color_field[i] = '#000'
			else:
				tg_color_field[i] = '#FFF'
		
		for tank in tg_tanks_players:
			_x = tank.x - 1
			_y = tank.y - 1
			
			rotation = tg_tanks_rotations[tank.rotation]
			hp = tank.hp
			
			for dy in xrange(3):
				for dx in xrange(3):
					x = (_x + dx) % tg_width
					y = (_y + dy) % tg_height
					
					d_index = dy * 3 + dx
					place = rotation[d_index]
					
					if place > 0:
						if place <= hp:
							color = tg_hex_colors[tank.color]
						else:
							color = tg_hex_colors['gray']
						
						index = y * tg_width + x
						tg_color_field[index] = color
		
		for bullet in tg_tanks_bullets:
			x, y = bullet.x, bullet.y
			index = y * tg_width + x
			
			color = 'red' if bullet.hp != 1 else 'yellow'
			tg_color_field[index] = tg_hex_colors[color]
	
	def tg_tanks_check_point(x, y):
		for dy in xrange(-1, 2):
			for dx in xrange(-1, 2):
				_x = (x + dx) % tg_width
				_y = (y + dy) % tg_height
				index = _y * tg_width + _x
				if tg_tanks_walls[index] > 0:
					return False
		for tank in tg_tanks_players:
			dx, dy = tank.x - x, tank.y - y
			if abs(dx) < 3 and abs(dy) < 3:
				return False
		
		return True
	
	def tg_tanks_set_player_to_random():
		ok = False
		while not ok:
			x, y = random.randint(0, tg_width - 1), random.randint(0, tg_height - 1)
			ok = tg_tanks_check_point(x, y)
		
		global tg_tanks_player
		tg_tanks_player = Tank(x, y, tg_tanks_PLAYER)
		tg_tanks_players.append(tg_tanks_player)
	
	
	
	def tg_tanks_failed():
		for tank in tg_tanks_players:
			if tank.team == (tg_tanks_PLAYER % 10):
				return False
		return True
	
	
	def tg_tanks_won():
		for tank in tg_tanks_players:
			if tank.team != (tg_tanks_PLAYER % 10):
				return False
		for bullet in tg_tanks_bullets:
			if bullet.team != (tg_tanks_PLAYER % 10):
				return False
		return True
	
	
	def tg_tanks_change_direction(rotation, dx, dy):
		global tg_tanks_direction_changed
		if tg_tanks_direction_changed:
			return
		
		player = tg_tanks_player
		if not player:
			return
		
		if player.rotation == rotation:
			player.need_to_forward = True
		else:
			tg_tanks_direction_changed = True
			player.rotation, player.dx, player.dy = rotation, dx, dy
	
	
	def tg_tanks_on_left_press():
		tg_tanks_change_direction('left',  -1,  0)
	def tg_tanks_on_right_press():
		tg_tanks_change_direction('right', +1,  0)
	def tg_tanks_on_up_press():
		tg_tanks_change_direction('up',     0, -1)
	def tg_tanks_on_down_press():
		tg_tanks_change_direction('down',   0, +1)
	def tg_tanks_player_fire():
		if tg_tanks_state:
			tg_tanks_init()
			tg_tanks_set_level_next()
		elif tg_tanks_player:
			tg_tanks_player.need_to_fire = True
	
	def tg_tanks_get_friends():
		res = []
		for tank in tg_tanks_players:
			if tank.team == (tg_tanks_PLAYER % 10) and tank is not tg_tanks_player:
				res.append(tank)
		return res
	def tg_tanks_get_enemies():
		res = []
		for tank in tg_tanks_players:
			if tank.team != (tg_tanks_PLAYER % 10):
				res.append(tank)
		return res


screen tg_tanks_screen:
	key 'K_LEFT'  action tg_tanks_on_left_press
	key 'K_RIGHT' action tg_tanks_on_right_press
	key 'K_UP'    action tg_tanks_on_up_press
	key 'K_DOWN'  action tg_tanks_on_down_press
	key 'a' action tg_tanks_on_left_press
	key 'd' action tg_tanks_on_right_press
	key 'w' action tg_tanks_on_up_press
	key 's' action tg_tanks_on_down_press
	
	key 'K_RETURN' action tg_tanks_player_fire
	key 'K_SPACE'  action tg_tanks_player_fire
	
	if not tg_pause and not tg_tanks_state:
		$ tg_tanks_update()
	
	use tg_main_screen
	
	if tg_tanks_state:
		text tg_tanks_state:
			text_size 30
			align (0.5, 0.05)
	
	hbox:
		align (0.5, 0.97)
		
		textbutton _('Continue' if tg_pause else 'Pause') action tg_change_pause_state
		textbutton _('Exit')                              action SetVariable('tg_tanks_exit', True)
	
	if tg_tanks_campaign:
		vbox:
			align (0.02, 0.5)
			spacing 15
			
			if tg_tanks_player is not None:
				text (_('You') + ': ' + str(tg_tanks_player.hp) + ' ♥'):
					font  'Arial'
					color int(tg_hex_colors[tg_tanks_player.color][1:], 16)
					text_size 25
				null ysize 10
			
			$ tg_tanks_friends = tg_tanks_get_friends()
			if tg_tanks_friends:
				text (_('Friends') + ': '):
					color     0x00AA00
					text_size 25
				for tank in tg_tanks_friends:
					text ('  ' + str(tank.hp) + ' ♥'):
						font  'Arial'
						color int(tg_hex_colors[tank.color][1:], 16)
				null ysize 10
			
			$ tg_tanks_enemies = tg_tanks_get_enemies()
			if tg_tanks_enemies:
				text (_('Enemies') + ': '):
					color	  0xAA0000
					text_size 25
				for tank in tg_tanks_enemies:
					text ('  ' + str(tank.hp) + ' ♥'):
						font  'Arial'
						color int(tg_hex_colors[tank.color][1:], 16)



label tg_tanks_start_usual:
	$ tg_tanks_campaign = False
	jump tg_tanks_start

label tg_tanks_start_campaign:
	$ tg_tanks_campaign = True
	jump tg_tanks_start


label tg_tanks_start:
	python:
		set_fps(tg_tanks_fps)
		tg_tanks_init()
		tg_tanks_set_level_next()
	
	window hide
	scene
	show screen tg_tanks_screen
	
	while not tg_tanks_exit:
		python:
			if tg_tanks_failed():
				if tg_tanks_campaign:
					tg_tanks_state = '{color=#FF0000}' + _('FAIL')
				else:
					tg_tanks_set_level_next()
			
			if tg_tanks_won():
				if tg_tanks_campaign and tg_tanks_level + 1 == len(tg_tanks_levels):
					tg_tanks_state = '{color=#00FF00}' + _('WIN')
				else:
					tg_tanks_set_level_next()
		
		pause 0.1
	
	hide screen tg_tanks_screen
	jump tg_main
