init -1 python:
	def tetris__tanks_init():
		set_fps(tetris.tanks_fps)
		
		tetris.tanks_frame_for_bullets = False
		tetris.tanks_direction_changed = False
		
		tetris.pause = False
		tetris.exit  = False
		tetris.tanks_state = ''
		
		tetris.tanks_level = -1
		tetris.tanks_set_next_level()
	
	
	def tetris__tanks_set_level(level):
		tetris.tanks_player = None
		tetris.tanks_players = []
		tetris.tanks_bullets = []
		
		lines = tetris.get_level('tanks', level)
		
		white = tetris.images['#FFF']
		
		tetris.tanks_walls = [[0]     * tetris.width for i in range(tetris.height)]
		tetris.pixels      = [[white] * tetris.width for i in range(tetris.height)]
		
		for y in range(tetris.height):
			for x in range(tetris.width):
				src = lines[y][x]
				
				if src == '.':
					continue
				
				if src == 'X':
					if tetris.tanks_campaign:
						v = 3 if tetris.tanks_get_near_count(lines, x, y, 'X') > 5 else 2
					else:
						v = 1
					tetris.tanks_walls[y][x] = v
					continue
				
				if src < 'A' or src > 'Z':
					out_msg('tetris.tanks_set_level', 'Unexpected symbol %s in level %s, pos (%s, %s)', src, level, x, y)
					continue
				
				if src == 'P':
					team = 0
				else:
					if tetris.tanks_campaign:
						if src == 'F':
							team = 0
						else:
							team = ord(src) - ord('A') + 1
					else:
						team = 1
				
				tank = TetrisTank(x, y, team)
				tetris.tanks_players.append(tank)
				
				if src == 'P':
					tank.is_bot = False
					tetris.tanks_player = tank
		
		tetris.tanks_update_walls_with_borders()
	
	
	def tetris__tanks_set_next_level():
		if tetris.tanks_campaign:
			num = tetris.tanks_level + 1
		else:
			num = random.randint(0, len(tetris.tanks_levels) - 1)
		tetris.tanks_set_level(num)
	
	
	def tetris__tanks_update_walls_with_borders():
		walls = tetris.tanks_walls
		tetris.tanks_walls_with_borders = []
		for y in range(tetris.height):
			line = [bool(tetris.tanks_get_near_count(walls, x, y, (1, 2, 3))) for x in range(tetris.width)]
			tetris.tanks_walls_with_borders.append(line)
	
	def tetris__tanks_get_near_count(lines, x, y, walls):
		count = 0
		
		for dy in (-1, 0, +1):
			for dx in (-1, 0, +1):
				tx, ty = tetris.normal_pos(x + dx, y + dy)
				
				v = lines[ty][tx]
				if v in walls:
					count += 1
		
		return count
	
	
	def tetris__tanks_update():
		if tetris.tanks_state:
			return
		
		tetris.tanks_frame_for_bullets = not tetris.tanks_frame_for_bullets
		
		for bullet in tetris.tanks_bullets:
			bullet.update()
		
		if not tetris.tanks_frame_for_bullets:
			tetris.tanks_direction_changed = False
			for tank in tetris.tanks_players:
				tank.update()
				tank.x, tank.y = tank.to_x, tank.to_y
		
		tetris.tanks_bullets = [bullet for bullet in tetris.tanks_bullets if not bullet.need_to_delete]
		tetris.tanks_players = [player for player in tetris.tanks_players if not player.need_to_delete]
		
		for bullet in tetris.tanks_bullets:
			bullet.x, bullet.y = bullet.to_x, bullet.to_y
		
		
		if tetris.tanks_failed():
			if tetris.tanks_campaign:
				tetris.tanks_state = '{color=#FF0000}' + _('FAIL')
				tetris.tanks_state_changed_time = get_game_time()
			else:
				tetris.tanks_set_next_level()
		
		if tetris.tanks_won():
			if tetris.tanks_campaign and tetris.is_last_level:
				tetris.tanks_state = '{color=#00FF00}' + _('WIN')
				tetris.tanks_state_changed_time = get_game_time()
			else:
				tetris.tanks_set_next_level()
	
	
	def tetris__tanks_render():
		pixels = tetris.pixels
		
		white = tetris.images['#FFF']
		black = tetris.images['#000']
		
		for y in range(tetris.height):
			pixels[y] = [(black if v else white) for v in tetris.tanks_walls[y]]
		
		for tank in tetris.tanks_players:
			tank_x = tank.x - 1
			tank_y = tank.y - 1
			
			rotation = tetris.tanks_rotations[tank.rotation]
			hp = tank.hp
			
			for dy in (0, 1, 2):
				for dx in (0, 1, 2):
					x, y = tetris.normal_pos(tank_x + dx, tank_y + dy)
					
					d_index = dy * 3 + dx
					place = rotation[d_index]
					
					if place > 0:
						color_name = tank.color if place <= hp else 'gray'
						color_hex = tetris.hex_colors[color_name]
						pixels[y][x] = tetris.images[color_hex]
		
		for bullet in tetris.tanks_bullets:
			color_name = 'red' if bullet.hp != 1 else 'yellow'
			color_hex = tetris.hex_colors[color_name]
			pixels[bullet.y][bullet.x] = tetris.images[color_hex]
	
	
	
	def tetris__tanks_failed():
		for tank in tetris.tanks_players:
			if tank.team == 0:
				return False
		return True
	
	
	def tetris__tanks_won():
		for tank in tetris.tanks_players:
			if tank.team != 0:
				return False
		for bullet in tetris.tanks_bullets:
			if bullet.team != 0:
				return False
		return True
	
	
	def tetris__tanks_change_direction(rotation, dx, dy):
		if tetris.tanks_direction_changed:
			return
		
		player = tetris.tanks_player
		if not player:
			return
		
		if player.rotation == rotation:
			player.need_to_forward = True
		else:
			tetris.tanks_direction_changed = True
			player.rotation, player.dx, player.dy = rotation, dx, dy
	
	
	def tetris__tanks_on_up_press():
		tetris.tanks_change_direction('up',     0, -1)
	def tetris__tanks_on_down_press():
		tetris.tanks_change_direction('down',   0, +1)
	def tetris__tanks_on_left_press():
		tetris.tanks_change_direction('left',  -1,  0)
	def tetris__tanks_on_right_press():
		tetris.tanks_change_direction('right', +1,  0)
	def tetris__tanks_player_fire():
		if tetris.tanks_state:
			if get_game_time() - tetris.tanks_state_changed_time > 1:
				tetris.tanks_init()
		elif tetris.tanks_player:
			tetris.tanks_player.need_to_fire = True
	
	def tetris__tanks_get_friends():
		res = []
		for tank in tetris.tanks_players:
			if tank.team == 0 and tank is not tetris.tanks_player:
				res.append(tank)
		return res
	def tetris__tanks_get_enemies():
		res = []
		for tank in tetris.tanks_players:
			if tank.team != 0:
				res.append(tank)
		return res


init 1 python:
	
	# times
	tetris.tanks_WAIT_FOR_HP_START = 3.0
	tetris.tanks_WAIT_FOR_HP_NEXT  = 1.0
	tetris.tanks_NO_DAMAGE = 1.5
	
	tetris.tanks_frame_for_bullets = False
	tetris.tanks_direction_changed = False
	tetris.tanks_campaign = False
	
	tetris.tanks_walls = []
	
	tetris.tanks_player = None
	tetris.tanks_players = []
	tetris.tanks_bullets = []
	
	tetris.tanks_fps = 10
	
	
	tetris.tanks_rotations = {
		'left': (
			0, 4, 6,
			1, 3, 0,
			0, 2, 5,
		),
		'up': (
			0, 1, 0,
			2, 3, 4,
			5, 0, 6,
		),
		'right': (
			5, 2, 0,
			0, 3, 1,
			6, 4, 0,
		),
		'down': (
			6, 0, 5,
			4, 3, 2,
			0, 1, 0,
		),
	}


screen tetris_tanks_screen:
	key 'W' action tetris.tanks_on_up_press
	key 'S' action tetris.tanks_on_down_press
	key 'A' action tetris.tanks_on_left_press
	key 'D' action tetris.tanks_on_right_press
	
	key 'RETURN' action tetris.tanks_player_fire
	key 'SPACE'  action tetris.tanks_player_fire
	
	if not tetris.pause and not tetris.tanks_state:
		$ tetris.tanks_update()
		$ tetris.tanks_render()
	
	use tetris_main_screen(tetris.text_xsize if tetris.tanks_campaign else 0, style.tetris_btn.ysize + tetris.indent * 3)
	
	if tetris.tanks_state:
		text tetris.tanks_state:
			style 'tetris_state'
	
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
	
	
	if tetris.tanks_campaign:
		vbox:
			xpos 0.05
			yalign 0.5
			spacing 25
			
			text ('%s: %s ♥' % (_('You'), tetris.tanks_player.hp)):
				style 'tetris_tanks_text'
				color tetris.hex_colors[tetris.tanks_player.color]
			
			$ screen_tmp = SimpleObject()
			
			$ screen_tmp.friends = tetris.tanks_get_friends()
			if screen_tmp.friends:
				vbox:
					xalign 1.0
					spacing 15
					
					text (_('Friends') + ':'):
						style 'tetris_tanks_text'
						color '#0A0'
					for tank in screen_tmp.friends:
						text ('%s ♥' % tank.hp):
							style 'tetris_tanks_text'
							color tetris.hex_colors[tank.color]
			
			$ screen_tmp.enemies = tetris.tanks_get_enemies()
			if screen_tmp.enemies:
				vbox:
					xalign 1.0
					spacing 15
					
					text (_('Enemies') + ':'):
						style 'tetris_tanks_text'
						color '#A00'
					for tank in screen_tmp.enemies:
						text ('%s ♥' % tank.hp):
							style 'tetris_tanks_text'
							color tetris.hex_colors[tank.color]


label tetris_tanks_start_usual:
	$ tetris.tanks_campaign = False
	jump tetris_tanks_start

label tetris_tanks_start_campaign:
	$ tetris.tanks_campaign = True
	jump tetris_tanks_start


label tetris_tanks_start:
	$ tetris.tanks_init()
	
	window hide
	scene
	show screen tetris_tanks_screen
	
	while not tetris.exit:
		pause 0.1
	
	hide screen tetris_tanks_screen
	jump tetris_main
