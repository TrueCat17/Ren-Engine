init python:
	
	dont_save.sc_nature_blocks = {}
	dont_save.sc_extra_blocks = {}
	
	
	def sc_map__generate(map_width, map_height, seed):
		sc_map.step = 1
		
		sc_map.x, sc_map.y = 0, 0
		sc_map.xsize, sc_map.ysize = map_width, map_height
		
		block_size = sc_map.block_size
		sc_map.block_xcount = map_width // block_size
		sc_map.block_ycount = map_height // block_size
		
		if seed < 0:
			seed = random.randint(0, 9999)
		sc_map.seed = seed
		sc_map.random = random.Random(seed)
		randint = sc_map.random.randint
		
		spec_resources = {}
		for y in range(sc_map.block_ycount):
			block_y = y * block_size
			for x in range(sc_map.block_xcount):
				block_x = x * block_size
				
				sc_map.update_block(x, y)
				
				for resource, count in sc_map.resource_count:
					for i in range(count):
						while True:
							cell_x = block_x + randint(0, block_size - 1)
							cell_y = block_y + randint(0, block_size - 1)
							if (cell_x, cell_y) not in spec_resources:
								spec_resources[(cell_x, cell_y)] = resource
								break
		
		
		map = sc_map.map = []
		for y in range(map_height):
			line = []
			map.append(line)
			
			for x in range(map_width):
				resource = spec_resources.get((x, y), 'wood')
				
				min_count, max_count = sc_map.resource_bounds[resource]
				count = randint(min_count, max_count)
				
				line.append(Cell(x, y, resource, count))
		
		sc_map.count_of_players = in_bounds(sc_map.count_of_players, 1, 4)
		sc_map.players = []
		for i in range(sc_map.count_of_players):
			player = SC_Player(i)
			sc_map.players.append(player)
			
			px, py = ((0, 0), (2, 0), (0, 2), (2, 2))[i]
			
			cx = sc_map.xsize // 6 + sc_map.xsize // 3 * px
			cy = sc_map.ysize // 6 + sc_map.ysize // 3 * py
			
			stone_cell = None
			max_score = None
			r = 4
			for dy in range(-r, r + 1):
				y = cy + dy
				if y < 0 or y >= map_height: continue
				
				line = map[y]
				
				for dx in range(-r, r + 1):
					x = cx + dx
					if x < 0 or x >= map_width: continue
					
					cell = line[x]
					if cell.resource != 'stone': continue
					
					dist = max(abs(dx), abs(dy))
					score = cell.resource_count - dist * 3
					
					if max_score is None or max_score < score:
						max_score = score
						stone_cell = cell
			
			if not stone_cell:
				stone_cell = map[cy][cx]
				stone_cell.resource = 'stone'
				stone_cell.resource_count = 20
			
			stone_cell.player = stone_cell.next_player = player
			player.build(stone_cell.x, stone_cell.y, 'career', 1, free = True)
		
		
		for i in range(1, sc_map.count_of_bots + 1):
			player = sc_map.players[-i]
			player.bot = SC_Bot(player)
		
		sc_map.player = sc_map.players[0]
		start_cell = sc_map.player.building_cells[0]
		sc_map.central_pos = (start_cell.x, start_cell.y)
		sc_map.move()
		
		dont_save.sc_nature_blocks.clear()
		dont_save.sc_extra_blocks.clear()
		
		resource_cells = sc_map.resource_cells = { resource: [] for resource in sc_info.simple_resources }
		for line in map:
			for cell in line:
				resource = cell.resource
				if resource != 'wood' or cell.resource_count > 15:
					resource_cells[resource].append(cell)
		
		sc_map.update_borders()
		
		for player in sc_map.players:
			if player.bot:
				player.bot.prepare()
			player.calc_changing_resources()
		
		sc_ach.check()
		sc_map.game_ended = False
	
	
	def sc_map__make_nature_block(block_x, block_y):
		image_dir = sc_map.image_dir
		image_size = sc_map.image_size
		block_size = sc_map.block_size
		
		block_image_size = block_size * image_size
		res = [(block_image_size, block_image_size)]
		
		start_x = block_x * block_size
		start_y = block_y * block_size
		for y in range(block_size):
			real_y = start_y + y
			cell_y = y * image_size
			
			line = sc_map.map[real_y]
			
			for x in range(block_size):
				real_x = start_x + x
				cell_x = x * image_size
				
				cell = line[real_x]
				image = cell.resource
				if image == 'wood' and cell.resource_count > 17:
					image = 'wood_best'
				
				res.append((cell_x, cell_y))
				res.append(image_dir + image + '.webp')
		
		dont_save.sc_nature_blocks[(block_x, block_y)] = im.composite(*res)
	
	
	def sc_map__get_block(block_x, block_y):
		return dont_save.sc_extra_blocks[(block_x, block_y)]
	
	def sc_map__update_block(block_x, block_y):
		dont_save.sc_extra_blocks[(block_x, block_y)] = None
	
	def sc_map__real_update_block(block_x, block_y):
		image_dir = sc_map.image_dir
		image_size = sc_map.image_size
		block_size = sc_map.block_size
		
		block_image_size = block_size * image_size
		res = [(block_image_size, block_image_size)]
		
		last_x = sc_map.xsize - 1
		last_y = sc_map.ysize - 1
		map = sc_map.map
		
		border_h = get_image_height(image_dir + 'border.webp')
		
		start_x = block_x * block_size
		start_y = block_y * block_size
		for y in range(block_size):
			real_y = start_y + y
			cell_y = y * image_size
			
			line = map[real_y]
			top_line    = map[real_y - 1] if real_y else None
			bottom_line = map[real_y + 1] if real_y != last_y else None
			
			for x in range(block_size):
				real_x = start_x + x
				cell_x = x * image_size
				
				cell = line[real_x]
				
				building = cell.building
				if building is not None:
					res.append((cell_x, cell_y))
					res.append(image_dir + building.replace(' ', '_') + '-%s.webp' % cell.building_level)
				
				player = cell.player
				if player is None: continue
				
				left = line[real_x - 1] if real_x else None
				if left and left.player is not player:
					res.append((cell_x, cell_y))
					res.append(player.border_v)
				
				right = line[real_x + 1] if real_x != last_x else None
				if right and right.player is not player:
					res.append((cell_x + image_size - border_h, cell_y))
					res.append(player.border_v)
				
				top = top_line and top_line[real_x]
				if top and top.player is not player:
					res.append((cell_x, cell_y))
					res.append(player.border_h)
				
				bottom = bottom_line and bottom_line[real_x]
				if bottom and bottom.player is not player:
					res.append((cell_x, cell_y + image_size - border_h))
					res.append(player.border_h)
		
		dont_save.sc_extra_blocks[(block_x, block_y)] = im.composite(*res)
	
	
	def sc_map__add_zoom(d):
		sc_map.zoom = in_bounds(sc_map.zoom + d * sc_map.zoom_change, sc_map.min_zoom, sc_map.max_zoom)
		sc_map.move()
	
	def sc_map__move(cell_x = 0, cell_y = 0, add = True):
		if add:
			if hotkeys.ctrl:
				cell_x *= 1000
				cell_y *= 1000
			x, y = sc_map.central_pos
			cell_x += x
			cell_y += y
		
		sc_map.x = -cell_x * sc_map.image_size * sc_map.zoom + (get_stage_width() - sc_control.get_xsize()) / 2
		sc_map.y = -cell_y * sc_map.image_size * sc_map.zoom + (get_stage_height() - sc_info.get_ysize()) / 2
		sc_map.alignment()
		sc_map.update_central_pos()
	
	def sc_map__update_central_pos():
		size = sc_map.image_size * sc_map.zoom
		
		left  = -sc_map.x / size
		right = (get_stage_width() - sc_control.get_xsize() - sc_map.x) / size
		
		top    = -sc_map.y / size
		bottom = (get_stage_height() - sc_info.get_ysize() - sc_map.y) / size
		
		sc_map.central_pos = ((left + right) / 2, (top + bottom) / 2)
	
	def sc_map__alignment():
		xsize = int(sc_map.xsize * sc_map.image_size * sc_map.zoom)
		stage_width = get_stage_width() - sc_control.get_xsize()
		if xsize < stage_width:
			sc_map.x = (stage_width - xsize) // 2
		else:
			left = stage_width - xsize
			right = 0
			sc_map.x = in_bounds(sc_map.x, left, right)
		
		ysize = int(sc_map.ysize * sc_map.image_size * sc_map.zoom)
		stage_height = get_stage_height() - sc_info.get_ysize()
		if ysize < stage_height:
			sc_map.y = (stage_height - ysize) // 2
		else:
			top = stage_height - ysize
			bottom = 0
			sc_map.y = in_bounds(sc_map.y, top, bottom)
	
	
	def sc_map__get_pos_under_mouse():
		x, y = get_mouse()
		if x < 0 or y < 0:
			return None
		if x >= get_stage_width() - sc_control.get_xsize():
			return None
		if y >= get_stage_height() - sc_info.get_ysize():
			return None
		
		cell_x = (x - sc_map.x) / sc_map.zoom / sc_map.image_size
		cell_y = (y - sc_map.y) / sc_map.zoom / sc_map.image_size
		
		cell_x = in_bounds(int(cell_x), 0, sc_map.xsize - 1)
		cell_y = in_bounds(int(cell_y), 0, sc_map.ysize - 1)
		
		return (cell_x, cell_y)
	
	
	def sc_map__to_draw():
		res = []
		
		zoom = sc_map.zoom
		block_size = int(sc_map.block_size * sc_map.image_size * zoom)
		x, y = int(sc_map.x), int(sc_map.y)
		map_width  = get_stage_width() - sc_control.get_xsize()
		map_height = get_stage_height() - sc_info.get_ysize()
		
		nature_blocks = dont_save.sc_nature_blocks
		extra_blocks  = dont_save.sc_extra_blocks
		
		if not nature_blocks:
			for block_y in range(sc_map.block_ycount):
				for block_x in range(sc_map.block_xcount):
					sc_map.make_nature_block(block_x, block_y)
					sc_map.update_block(block_x, block_y)
		
		for k in extra_blocks:
			block_x, block_y = k
			
			image_x = block_x * block_size + x
			if image_x >= map_width: continue
			if image_x < -block_size: continue
			
			image_y = block_y * block_size + y
			if image_y >= map_height: continue
			if image_y < -block_size: continue
			
			nature_image = nature_blocks[k]
			
			if not extra_blocks[k]:
				sc_map.real_update_block(block_x, block_y)
			extra_image = extra_blocks[k]
			
			obj = {
				'image': nature_image,
				'xpos': image_x,
				'ypos': image_y,
				'size': block_size,
				'alpha': 1,
			}
			res.append(obj)
			res.append(dict(obj, image = extra_image))
		
		
		cell_time = 2
		cell_alpha_min = 0.20
		cell_alpha_max = 0.50
		
		cell_alpha = time.time() % cell_time # 0-2n
		if cell_alpha > cell_time * 0.5:
			cell_alpha = cell_time - cell_alpha # 0-n
		cell_alpha = cell_alpha * (cell_alpha_max - cell_alpha_min) + cell_alpha_min # min-max
		
		size = int(sc_map.image_size * zoom)
		
		if sc_control.selected_cell:
			res.append({
				'image': sc_map.selected_cell_marker,
				'xpos': sc_control.selected_cell.x * size + x,
				'ypos': sc_control.selected_cell.y * size + y,
				'size': size,
				'alpha': cell_alpha,
			})
		
		if sc_map.mark_disabled:
			image = sc_map.disabled_cell_marker
			
			players = sc_map.players if sc_map.player.bot else [sc_map.player]
			for player in players:
				for cell in player.building_cells:
					if cell.building is None: continue
					if not cell.disabled: continue
					
					res.append({
						'image': image,
						'xpos': cell.x * size + x,
						'ypos': cell.y * size + y,
						'size': size,
						'alpha': cell_alpha,
					})
		
		if sc_map.mark_spending:
			spending_cell_marker = sc_map.spending_cell_marker
			taking_cell_marker   = sc_map.taking_cell_marker
			player = sc_map.player
			
			for cell in sc_map.host_changed_cells:
				is_cur  = cell.player is player
				is_next = cell.next_player is player
				
				if       is_cur and not is_next: image = spending_cell_marker
				elif not is_cur and     is_next: image = taking_cell_marker
				else: continue
				
				res.append({
					'image': image,
					'xpos': cell.x * size + x,
					'ypos': cell.y * size + y,
					'size': size,
					'alpha': cell_alpha,
				})
		
		return res
	
	
	def sc_map__next_step():
		sc_map.next_step_ready = True
	
	def sc_map__real_next_step():
		sc_map.next_step_ready = False
		
		if not sc_map.player.bot:
			sc_map.player.calc_changing_resources()
			for resource in sc_info.resources:
				if sc_map.player[resource] + sc_map.player['change_' + resource] < 0:
					sc_info.set_msg('Not enough resources')
					return
		
		for player in sc_map.players:
			player.update()
		
		sc_map.update_borders()
		cell = sc_control.selected_cell
		if cell:
			sc_control.select_cell(cell.x, cell.y)
		
		if not sc_map.game_ended and sc_map.count_of_players > 1 and sc_map.count_of_bots == sc_map.count_of_players - 1:
			sc_map.check_win()
			sc_map.check_fail()
		sc_ach.check()
		
		sc_map.step += 1
		for player in sc_map.players:
			if player.bot:
				player.bot.prepare()
		sc_map.step_time = time.time()
	
	
	def sc_map__check_win():
		if sc_ach.winner():
			sc_map.game_ended = True
			if 'winner' in persistent.sc_ach:
				notification.out('Win')
	def sc_map__check_fail():
		if sc_ach.loser():
			sc_map.game_ended = True
			if 'loser' in persistent.sc_ach:
				notification.out('Fail')
	
	
	def sc_map__execute_bots():
		sc_map.executed_bots = True
		for player in sc_map.players:
			bot = player.bot
			if bot is None or bot.executed: continue
			
			for i in range(3):
				bot.execute()
			if not bot.executed:
				sc_map.executed_bots = False
		
		if sc_map.executed_bots and sc_map.next_step_ready:
			sc_map.real_next_step()
	
	
	def sc_map__update_forces():
		sc_map.need_update_forces = True
	
	def sc_map__check_update_forces():
		if sc_map.need_update_forces:
			sc_map.real_update_forces()
	
	def sc_map__real_update_forces():
		sc_map.need_update_forces = False
		
		forces_template = [0] * sc_map.count_of_players
		for line in sc_map.map:
			for cell in line:
				cell.player_forces = forces_template.copy()
		
		for line in sc_map.map:
			for cell in line:
				cell.update_force()
		
		max = __builtins__.max
		players = sc_map.players
		for line in sc_map.map:
			for cell in line:
				player_forces = cell.player_forces
				max_force = max(player_forces)
				if not max_force: continue
				
				player = cell.player
				if player is not None and player_forces[player.index] == max_force:
					cell.next_player = player
				else:
					cell.next_player = players[player_forces.index(max_force)]
		
		host_changed_cells = sc_map.host_changed_cells = []
		for line in sc_map.map:
			for cell in line:
				player = cell.player
				if player is not None and player is not cell.next_player:
					host_changed_cells.append(cell)
	
	
	def sc_map__update_borders():
		sc_map.real_update_forces()
		
		blocks = dont_save.sc_extra_blocks
		block_size = sc_map.block_size
		last_pos = block_size - 1
		
		for y, line in enumerate(sc_map.map):
			block_y, cell_y = divmod(y, block_size)
			
			for cell in line:
				old_player = cell.player
				next_player = cell.next_player
				if old_player is next_player: continue
				
				block_x, cell_x = divmod(cell.x, block_size)
				
				blocks[(block_x, block_y)] = None
				if cell_x == 0:
					pos = (block_x - 1, block_y)
					if pos in blocks:
						blocks[pos] = None
				elif cell_x == last_pos:
					pos = (block_x + 1, block_y)
					if pos in blocks:
						blocks[pos] = None
				if cell_y == 0:
					pos = (block_x, block_y - 1)
					if pos in blocks:
						blocks[pos] = None
				elif cell_y == last_pos:
					pos = (block_x, block_y + 1)
					if pos in blocks:
						blocks[pos] = None
				
				cell.player = next_player
				if old_player is not None and cell.building is not None:
					cell.building = None
					cell.building_level = 0
					cell.disabled = False
					cell.update_nears_for_force()
					old_player.building_cells.remove(cell)
		
		sc_map.update_forces()
		
		for player in sc_map.players:
			player.calc_changing_resources()
	
	
	sc_map = SimpleObject()
	build_object('sc_map')
	
	sc_map.resource_bounds = {
		'food':  ( 4,  6),
		'wood':  ( 5, 20),
		'stone': (15, 40),
		'coal':  (10, 20),
		'metal': ( 8, 16),
	}
	sc_map.resource_count = (
		('food',  2),
		('stone', 2),
		('coal',  2),
		('metal', 1),
	)
	
	sc_map.bg = im.rect('#222')
	sc_map.hovered = False
	
	sc_map.selected_cell_marker = im.rect('#F00')
	sc_map.disabled_cell_marker = im.rect('#FF0')
	sc_map.spending_cell_marker = im.rect('#F80')
	sc_map.taking_cell_marker   = im.rect('#08B')
	
	sc_map.host_changed_cells = []
	
	sc_map.block_size = 8
	
	sc_map.min_zoom = 0.5
	sc_map.max_zoom = 2.0
	sc_map.zoom_change = sc_map.min_zoom
	sc_map.zoom = 1.0
	
	sc_map.image_dir = os.path.dirname(get_filename(0)) + '/images/'
	sc_map.image_list = os.listdir(sc_map.image_dir)
	sc_map.image_size = get_image_height(sc_map.image_dir + 'food.webp')
	
	sc_map.count_of_players = 4
	sc_map.count_of_bots = 3
	sc_map.player_colors = ['#F00', '#00F', '#FF0', '#0EE']
	sc_map.player = None
	
	sc_map.mark_disabled = True
	sc_map.mark_spending = True
	
	sc_map.central_pos = None
	sc_map.game_ended = False
	sc_map.executed_bots = False
	sc_map.need_update_forces = False
	sc_map.next_step_ready = False
	sc_map.step_time = 0
	
	def sc_on_resize():
		pos = sc_map.central_pos
		if pos:
			sc_map.move(pos[0], pos[1], False)
	signals.add('resized_stage', sc_on_resize)
	
	def preload_images():
		for file_name in sc_map.image_list:
			load_image(sc_map.image_dir + file_name)
	preload_images()
	set_interval(preload_images, 10) # disallow to unload from image cache
