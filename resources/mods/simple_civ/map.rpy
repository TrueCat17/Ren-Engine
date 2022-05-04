init -10 python:
	
	class Cell:
		__slots__ = ['x', 'y', 'resource', 'resource_count', 'road_level', 'enabled_level', 'building', 'building_level', 'workers']
		
		def __init__(self, x, y, resource, count):
			self.x = x
			self.y = y
			self.resource = resource
			self.resource_count = count
			self.road_level = 0
			self.enabled_level = 0
			self.building = None
			self.building_level = 0
			self.workers = []
		
		def __getstate__(self):
			return tuple(getattr(self, prop) for prop in self.__slots__)
		def __setstate__(self, state):
			for i in xrange(len(self.__slots__)):
				setattr(self, self.__slots__[i], state[i])
	
	
	def sc_map__generate(w, h, block_size):
		sc_map.x, sc_map.y = 0, 0
		sc_map.xsize, sc_map.ysize = w, h
		
		sc_map.block_size = block_size
		sc_map.block_xcount = sc_map.xsize / block_size
		sc_map.block_ycount = sc_map.ysize / block_size
		
		sum_chances = 0.0
		chances = []
		for k in sc_map.chances:
			sum_chances += sc_map.chances[k]
			chances.append((k, sum_chances))
		
		sc_map.map = []
		for y in xrange(sc_map.ysize):
			line = []
			sc_map.map.append(line)
			
			for x in xrange(sc_map.xsize):
				r = random.random() * sum_chances
				for resource, chance in chances:
					if r <= chance:
						break
				
				min, max = sc_map.bounds[resource]
				count = random.randint(min, max)
				line.append(Cell(x, y, resource, count))
		
		sc_map.players = []
		for i in xrange(sc_map.count_players):
			player = Player(is_ai = i>0)
			sc_map.players.append(player)
			
			start_x = random.randint(sc_map.xsize / 8, sc_map.xsize * 7 / 8)
			start_y = random.randint(sc_map.ysize / 8, sc_map.ysize * 7 / 8)
			for _ in xrange(sc_map.count_start_builders):
				player.add_builder(start_x, start_y)
			
			if i == 0:
				sc_map.player = player
				
				sc_map.center_cell = (start_x, start_y)
				sc_map.move()
				control.select_cell(start_x, start_y)
			
			player.calc_changing_resources()
		
		sc_map.blocks = {}
		for y in xrange(sc_map.block_ycount):
			for x in xrange(sc_map.block_xcount):
				sc_map.update_block(x, y)
	
	def sc_map__get_block(block_x, block_y):
		return sc_map.blocks[(block_x, block_y)]
	def sc_map__update_block(block_x, block_y):
		sc_map.blocks[(block_x, block_y)] = None
	def sc_map__real_update_block(block_x, block_y):
		image_dir = sc_map.image_dir
		image_size = sc_map.image_size
		res = [(image_size * sc_map.block_size, image_size * sc_map.block_size)]
		
		xstart = block_x * sc_map.block_size
		ystart = block_y * sc_map.block_size
		for y in xrange(sc_map.block_size):
			line  = sc_map.map[ystart + y]
			for x in xrange(sc_map.block_size):
				cell = line[xstart + x]
				cell_x, cell_y = x * image_size, y * image_size
				
				res.append((cell_x, cell_y))
				res.append(image_dir + cell.resource + '.png')
				
				if cell.road_level:
					res.append((cell_x, cell_y))
					res.append(image_dir + 'road-' + str(cell.road_level) + '.png')
				
				if cell.building:
					res.append((cell_x, cell_y))
					res.append(image_dir + cell.building.replace(' ', '_') + '-' + str(cell.building_level) + '.png')
		
		sc_map.blocks[(block_x, block_y)] = im.composite(*res)
	
	def sc_map__add_zoom(d):
		sc_map.zoom = in_bounds(sc_map.zoom + d * sc_map.zoom_change, sc_map.min_zoom, sc_map.max_zoom)
		sc_map.move()
	
	def sc_map__move(cell_x = 0, cell_y = 0, add = True):
		if add:
			if sc_map.ctrl:
				cell_x *= 1000
				cell_y *= 1000
			x, y = sc_map.center_cell
			cell_x += x
			cell_y += y
		
		sc_map.x = -cell_x * sc_map.image_size * sc_map.zoom + (get_stage_width() - control.xsize) / 2.0
		sc_map.y = -cell_y * sc_map.image_size * sc_map.zoom + (get_stage_height() - info.ysize) / 2.0
		sc_map.alignment()
		sc_map.update_center_cell()
	
	def sc_map__update_center_cell():
		size = sc_map.image_size * sc_map.zoom
		
		left  = -sc_map.x / size
		right = (get_stage_width() - control.xsize - sc_map.x) / size
		
		top    = -sc_map.y / size
		bottom = (get_stage_height() - info.ysize - sc_map.y) / size
		
		sc_map.center_cell = ((left + right) / 2, (top + bottom) / 2)
	
	def sc_map__alignment():
		xsize = sc_map.xsize * sc_map.image_size * sc_map.zoom
		stage_width = get_stage_width() - control.xsize
		if xsize < stage_width:
			sc_map.x = (stage_width - int(xsize)) / 2
		else:
			left = stage_width - int(xsize)
			right = 0
			sc_map.x = in_bounds(sc_map.x, left, right)
		
		ysize = sc_map.ysize * sc_map.image_size * sc_map.zoom
		stage_height = get_stage_height() - info.ysize
		if ysize < stage_height:
			sc_map.y = (stage_height - int(ysize)) / 2
		else:
			top = stage_height - int(ysize)
			bottom = 0
			sc_map.y = in_bounds(sc_map.y, top, bottom)
	
	
	def sc_map__to_draw():
		res = []
		
		zoom = sc_map.zoom
		block_size = sc_map.block_size * sc_map.image_size
		x, y = sc_map.x, sc_map.y
		
		for k in sc_map.blocks:
			block_x, block_y = k
			if not sc_map.blocks[k]:
				sc_map.real_update_block(block_x, block_y)
			block_image = sc_map.blocks[k]
			res.append({
				'image': block_image,
				'xpos': int(block_x * block_size * zoom + x),
				'ypos': int(block_y * block_size * zoom + y),
				'size': int(block_size * zoom),
			})
		
		size = int(sc_map.image_size * zoom)
		for player in sc_map.players:
			for UnitType in [Builder]:
				units = {} # units[pos] = is_free
				for unit in player.units:
					if not isinstance(unit, UnitType): continue
					
					pos = unit.x, unit.y
					is_free = unit.turns > 0
					units[pos] = units.get(pos) or is_free
				
				for (xpos, ypos), is_free in units.iteritems():
					res.append({
						'image': UnitType.image,
						'xpos': int(xpos * sc_map.image_size * zoom + x),
						'ypos': int(ypos * sc_map.image_size * zoom + y),
						'size': size,
					})
					if not is_free:
						res.append({
							'text': UnitType.no_turn_symbol,
							'xpos': int(xpos * sc_map.image_size * zoom + x + UnitType.no_turn_symbol_offset * size),
							'ypos': int(ypos * sc_map.image_size * zoom + y),
						})
		
		
		cell_time = 2
		cell_alpha_min = 0.20
		cell_alpha_max = 0.50
		
		cell_alpha = time.time() % cell_time # 0-2n
		if cell_alpha > cell_time * 0.5:
			cell_alpha = cell_time - cell_alpha # 0-n
		cell_alpha = cell_alpha * (cell_alpha_max - cell_alpha_min) + cell_alpha_min # min-max
		
		res.append({
			'image': im.rect('#F00'),
			'xpos': int(control.selected_cell.x * sc_map.image_size * zoom + x),
			'ypos': int(control.selected_cell.y * sc_map.image_size * zoom + y),
			'size': int(sc_map.image_size * zoom),
			'alpha': cell_alpha,
		})
		
		if sc_map.mark_unloaded:
			player = sc_map.player
			image = im.rect('#FF0')
			size = sc_map.image_size * zoom
			int_size = int(size)
			
			for building_cell in player.building_cells:
				mark = False
				building = building_cell.building
				if building == 'district':
					for worker in building_cell.workers:
						if not worker.work_cell:
							mark = True
							break
				elif building == 'storage':
					mark = building_cell.enabled_level == 0
				elif building:
					mark = building_cell.enabled_level != len(building_cell.workers) or building_cell.enabled_level != building_cell.building_level
				
				if mark:
					res.append({
						'image': image,
						'xpos': int(building_cell.x * size + x),
						'ypos': int(building_cell.y * size + y),
						'size': int_size,
						'alpha': cell_alpha,
					})
		
		return res
	
	
	def sc_map__outside(x, y):
		return x < 0 or y < 0 or x >= sc_map.xsize or y >= sc_map.ysize
	
	def sc_map__has_road(from_x, from_y, to_x, to_y):
		if sc_map.outside(from_x, from_y) or sc_map.outside(to_x, to_y):
			return False
		
		to = (to_x, to_y)
		m = sc_map.map
		
		closed = []
		opened = [(-1, from_x, from_y)]
		while opened:
			dist2, x, y = c = opened.pop(0)
			if (x, y) == to:
				return True
			closed.append(c)
			
			for dx, dy in ((-1, 0), (+1, 0), (0, -1), (0, +1)):
				nx, ny = x + dx, y + dy
				if sc_map.outside(nx, ny): continue
				
				dx2, dy2 = to_x - nx, to_y - ny
				c = (dx2*dx2 + dy2*dy2, nx, ny)
				if c in opened or c in closed: continue
				
				cell = m[ny][nx]
				if cell.road_level > 0:
					for i in xrange(len(opened)):
						if c < opened[i]:
							opened.insert(i, c)
							break
					else:
						opened.append(c)
				else:
					closed.append(c)
		return False
	
	
	def sc_map__next_step():
		sc_map.player.calc_changing_resources()
		for resource in info.resources:
			if sc_map.player[resource] + sc_map.player['change_' + resource] < 0:
				info.set_msg('Not enough resources')
				return
		
		for player in sc_map.players:
			player.update()
		
		control.step += 1
		
		cell = control.selected_cell
		control.select_cell(cell.x, cell.y)
	
	
	build_object('sc_map')
	
	sc_map.min_zoom = 0.25
	sc_map.max_zoom = 2.0
	sc_map.zoom_change = 0.25
	sc_map.zoom = 1.5
	
	sc_map.image_dir = 'mods/' + get_current_mod() + '/images/'
	sc_map.image_list = os.listdir(sc_map.image_dir)
	sc_map.image_size = get_image_height(sc_map.image_dir + 'food.png')
	
	sc_map.count_players = 1
	sc_map.count_start_builders = 1
	
	sc_map.ctrl = False
	sc_map.shift = False
	
	sc_map.mark_unloaded = True
	
	def on_resize():
		cell = control.selected_cell
		sc_map.move(cell.x, cell.y, False)
	signals.add('resized_stage', on_resize)
	
	def preload_images():
		for i in sc_map.image_list:
			load_image(sc_map.image_dir + i)
	preload_images()
	set_interval(preload_images, 10) # disallow to unload from image cache
