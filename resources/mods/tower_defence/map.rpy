init 1 python:
	
	# S - start
	# E - end
	# X - road
	# . - empty
	
	tower_defence.map = '''
		...............
		SXX.....XXX....
		..X.XXX.X.XXXX.
		..X.X.X.X....X.
		..XXX.X.X.XXXX.
		......XXX.X....
		..........XXXXE
	'''
	
	tower_defence.cell_size = 32
	
	tower_defence.map_bg = im.rect('#090')
	tower_defence.map_end_image = im.rect('#B00')


init -1 python:
	def tower_defence__init_map():
		level = [line.strip() for line in tower_defence.map.strip().split('\n')]
		tower_defence.map_h = len(level)
		tower_defence.map_w = len(level[0])
		
		start_cell = end_cell = None
		path_cells = set()
		for y, line in enumerate(level):
			for x, c in enumerate(line):
				if c == 'S':
					start_cell = (x, y)
				elif c == 'E':
					end_cell = (x, y)
				elif c == 'X':
					path_cells.add((x, y))
				elif c != '.':
					out_msg('tower_defence.init_map()', 'Line %s: expected symbols S, E, . or X, got %s', y + 1, c)
		
		if not start_cell or not end_cell:
			out_msg('tower_defence.init_map()', 'No start or/and end cell')
		
		path = tower_defence.path = [start_cell]
		cell = start_cell
		while cell != end_cell:
			for dx, dy in ((-1, 0), (+1, 0), (0, -1), (0, +1)):
				near_cell = cell[0] + dx, cell[1] + dy
				if near_cell in path: continue
				
				if near_cell in path_cells or near_cell == end_cell:
					cell = near_cell
					break
			else:
				out_msg('tower_defence.init_map()', 'Path to end cell not found')
				break
			
			path.append(cell)
		
		tower_defence.tank_path = [(absolute(x + 0.5), absolute(y + 0.5)) for x, y in path]
		
		
		cell_size = tower_defence.cell_size
		
		way_horizontal = im.scale('mods/tower_defence/images/way_horizontal.png', cell_size, cell_size)
		way_vertical   = im.rotozoom(way_horizontal, 90, 1)
		
		way_left_bottom  = im.scale('mods/tower_defence/images/way_corner.png', cell_size, cell_size)
		way_left_top     = im.flip(way_left_bottom, False, True)
		way_right_bottom = im.flip(way_left_bottom, True, False)
		way_right_top    = im.flip(way_left_bottom, True, True)
		
		w = tower_defence.map_w * cell_size
		h = tower_defence.map_h * cell_size
		back = im.scale(tower_defence.map_bg, w, h)
		
		args = [(w, h), (0, 0), back]
		
		sx, sy = start_cell
		args += [(sx * cell_size, sy * cell_size), way_horizontal]
		
		rotation = 90
		rotations = tower_defence.rotations = [rotation]
		for i in range(1, len(path) - 1):
			prev_rotation = rotation
			
			px, py = path[i - 1]
			x, y   = path[i]
			nx, ny = path[i + 1]
			
			if py == ny:
				image = way_horizontal
			elif px == nx:
				image = way_vertical
			else:
				if py < ny:
					if px < nx:
						image, rotation = (way_left_bottom,  180) if py == y else (way_right_top, 90)
					else:
						image, rotation = (way_right_bottom, 180) if py == y else (way_left_top, 270)
				else:
					if px < nx:
						image, rotation = (way_left_top,  0)      if py == y else (way_right_bottom, 90)
					else:
						image, rotation = (way_right_top, 0)      if py == y else (way_left_bottom, 270)
			
			# wrong:   270 -> 0   or 360 -> 0
			# correct: 270 -> 360 or 360 -> 360
			for v in (-90, 0, +90):
				v += prev_rotation
				if rotation % 360 == v % 360:
					rotation = v
					break
			
			rotations.append(rotation)
			args += [(x * cell_size, y * cell_size), image]
		tower_defence.rotations.append(rotation)
		
		ex, ey = end_cell
		end = im.scale(tower_defence.map_end_image, cell_size, cell_size)
		args += [(ex * cell_size, ey * cell_size), end]
		
		tower_defence.field = im.composite(*args)
