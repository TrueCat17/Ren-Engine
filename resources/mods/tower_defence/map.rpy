init -2 python:
	
	S, E, W = 1, 2, 3
	
	td_map = (
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		S, W, W, 0, 0, 0, 0, 0, W, W, W, 0, 0, 0, 0,
		0, 0, W, 0, W, W, W, 0, W, 0, W, W, W, W, 0,
		0, 0, W, 0, W, 0, W, 0, W, 0, 0, 0, 0, W, 0,
		0, 0, W, W, W, 0, W, 0, W, 0, W, W, W, W, 0,
		0, 0, 0, 0, 0, 0, W, W, W, 0, W, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, W, W, W, W, E
	)
	
	td_cell_size = 32
	td_map_w, td_map_h = 15, 7
	
	def td_get_pos(v):
		i = td_map.index(v)
		return i % td_map_w, i / td_map_w
	
	def td_from_map(x, y = None):
		if y is None:
			x, y = x # x - list, 1 arg
		return td_map[y * td_map_w + x]
	
	def td_init_back():
		global td_path, td_rotations, td_back
		td_path = []
		td_rotations = []
		
		nears = ((-1, 0), (+1, 0), (0, -1), (0, +1))
		cur = td_get_pos(S)
		while True:
			td_path.append(cur)
			if td_from_map(cur) == E:
				break
			
			for dx, dy in nears:
				x, y = cur[0] + dx, cur[1] + dy
				if (x, y) in td_path: continue
				
				if td_from_map(x, y):
					cur = x, y
					break
			else:
				break
		
		way_horizontal = im.Scale('mods/tower_defence/images/way_horizontal.png', td_cell_size, td_cell_size)
		way_vertical   = im.Rotozoom(way_horizontal, 90, 1)
		
		way_left_down  = im.Scale('mods/tower_defence/images/way_corner.png', td_cell_size, td_cell_size)
		way_left_up    = im.Flip(way_left_down, False, True)
		way_right_down = im.Flip(way_left_down, True, False)
		way_right_up   = im.Flip(way_left_down, True, True)
		
		end  = im.Rect('#B00', td_cell_size, td_cell_size)
		back = im.Rect('#090')
		
		w, h = td_map_w * td_cell_size, td_map_h * td_cell_size
		args = [(w, h), (0, 0), im.Scale(back, w, h)]
		
		sx, sy = td_path[0]
		args += [(sx * td_cell_size, sy * td_cell_size), way_horizontal]
		
		rotation = 90
		td_rotations.append(rotation)
		for i in xrange(1, len(td_path) - 1):
			px, py = td_path[i - 1]
			x, y   = td_path[i]
			nx, ny = td_path[i + 1]
			
			if py == ny:
				image = way_horizontal
			elif px == nx:
				image = way_vertical
			else:
				if py < ny:
					if px < nx:
						image, rotation = (way_left_down,  180) if py == y else (way_right_up, 90)
					else:
						image, rotation = (way_right_down, 180) if py == y else (way_left_up, 270)
				else:
					if px < nx:
						image, rotation = (way_left_up,  0)     if py == y else (way_right_down, 90)
					else:
						image, rotation = (way_right_up, 0)     if py == y else (way_left_down, 270)
			
			td_rotations.append(rotation)
			args += [(x * td_cell_size, y * td_cell_size), image]
		td_rotations.append(rotation)
		
		ex, ey = td_path[-1]
		args += [(ex * td_cell_size, ey * td_cell_size), end]
		
		td_back = im.Composite(*args)
	td_init_back()

