init python:
	
	def physics__get_map_free():
		location_free = cur_location.free()
		
		size = 64
		half_size = size // 2 # > (radius + some_indent) * 2
		me_x = in_bounds(me.x, 0, get_image_width(location_free) - 1)
		me_y = in_bounds(me.y, 0, get_image_height(location_free) - 1)
		start_x = (round(me_x / half_size) - 1) * half_size
		start_y = (round(me_y / half_size) - 1) * half_size
		
		cs = me.xsize // 2
		def near(x, y, width, height):
			return (
				(x < me.x + cs and me.x - cs < x + width and y < me.y + cs and me.y - cs < y + height) and
				(x < start_x + size and x + width >= start_x and y < start_y + size and y + height >= start_y)
			)
		
		objs = [obj for obj in cur_location.objects if not isinstance(obj, Character)]
		characters = [obj for obj in cur_location.objects if isinstance(obj, Character) and obj is not me]
		
		# subtract 253.9/255 from each (rgb) channel: all colors, exceptly clear-white, become black
		matrix = im.matrix.identity()
		matrix[4] = matrix[9] = matrix[14] = -253.9/255.0
		matrix = im.matrix.invert() * matrix # invert colors before it
		
		to_draw = [(size, size)]
		if location_free:
			to_draw += [(-start_x, -start_y), location_free]
		else:
			to_draw += [(0, 0), im.rect('#000', size, size)]
		
		for obj in objs:
			if isinstance(obj, RpgLocation):
				continue
			
			obj_free = obj.free
			obj_free = obj_free() if callable(obj_free) else None
			if obj_free is None:
				continue
			if obj.frames and obj.frames > 1:
				obj_free = im.crop(obj_free, obj.crop)
			
			w, h = get_image_size(obj_free)
			x, y = obj.x + obj.xoffset - get_absolute(obj.xanchor, w), obj.y + obj.yoffset - get_absolute(obj.yanchor, h)
			if near(x, y, w, h):
				to_draw += [(x - start_x, y - start_y), im.matrix_color(obj_free, matrix)]
		
		for character in characters:
			if not character.invisible and near(character.x, character.y, 0, 0):
				to_draw += [(character.x - cs / 2 - start_x, character.y - cs / 2 - start_y), im.circle('#FFF', cs, cs)]
		
		if len(to_draw) == 3: # 3 - [size, pos0, image0]
			return location_free, 0, 0
		return im.composite(*to_draw), start_x, start_y
	
	
	def physics__get_side_coords(side, radius):
		side %= 8
		
		cache = physics__get_side_coords.__dict__
		key = (side, radius)
		if key in cache:
			return cache[key]
		
		if 'coords' not in cache:
			size = radius * 2
			img = im.circle('#000', size, size)
			
			# make sure that 4 parts of circle are equal
			img = im.crop(img, 0, 0, radius, radius)
			args = [
				(size, size),
				(0, 0), img,
				(radius, 0), im.flip(img, True, False),
				(0, radius), im.flip(img, False, True),
				(radius, radius), im.flip(img, True, True),
			]
			img = im.composite(*args)
			
			# RGBA32
			min_black = 128 # 0, 0, 0, 128
			max_black = 255 # 0, 0, 0, 255
			
			def is_black(x, y):
				if x < 0 or x >= size or y < 0 or y >= size:
					return False
				color = get_image_pixel(img, x, y)
				return color >= min_black and color <= max_black
			
			def get_coords(dx, dy, cond_left, cond_right):
				left_coords, right_coords = [], []
				for y in range(size):
					for x in range(size):
						if is_black(x, y):
							if not is_black(x + dx, y + dy):
								point = (x - radius, y - radius)
								if cond_left(x, y):
									left_coords.append(point)
								if cond_right(x, y):
									right_coords.append(point)
				return left_coords, right_coords
			
			corner_coords = get_coords(-1, -1, lambda x, y: x <= y, lambda x, y: x >= y)
			line_coords   = get_coords( 0, -1, lambda x, y: x < radius, lambda x, y: x >= radius)
			cache['coords'] = corner_coords, line_coords
		
		left_coords, right_coords = cache['coords'][side & 1]
		left_coords, right_coords = left_coords.copy(), right_coords.copy()
		
		angle = side // 2 * 90
		sina = int(_sin(angle))
		cosa = int(_cos(angle))
		for coords in (left_coords, right_coords):
			for i, (x, y) in enumerate(coords):
				rot_x = x * cosa - y * sina
				rot_y = x * sina + y * cosa
				coords[i] = (rot_x, rot_y)
		
		cache[key] = left_coords, right_coords
		return cache[key]
	
	
	def physics__check_side(is_black, from_x, from_y, side, radius):
		def get_free_part(coords):
			res = 0
			for x, y in coords:
				is_free = is_black(from_x + x, from_y + y)
				if is_free:
					res += 1
			return res / len(coords)
		
		left_coords, right_coords = physics.get_side_coords(side, radius)
		return get_free_part(left_coords), get_free_part(right_coords)
	
	
	def physics__get_end_point(from_x, from_y, dx, dy, length):
		radius = me.radius
		if radius % 2:
			radius += 1
		
		s2 = 1 / (2 ** 0.5)
		if dx and dy:
			dx, dy = dx * s2, dy * s2
		
		free, start_x, start_y = physics.get_map_free()
		if free is None:
			return from_x + dx * length, from_y + dy * length
		from_x -= start_x
		from_y -= start_y
		
		black_color = 255 # r, g, b, a = 0, 0, 0, 255
		map_width, map_height = get_image_size(free)
		def is_black(x, y):
			x, y = int(x), int(y)
			if x < 0 or x >= map_width or y < 0 or y >= map_height:
				return False
			return get_image_pixel(free, x, y) == black_color
		
		
		rotations = (
			(-s2, -s2), # left-up: x == -1, y == -1
			(  0, -1 ), # up
			( s2, -s2), # right-up
			(  1,  0 ), # ...
			( s2,  s2),
			(  0,  1 ),
			(-s2,  s2),
			( -1,  0 ),
		)
		forward = (dx, dy)
		side = rotations.index(forward)
		
		angle45 = 1, rotations[(side - 1) % 8], rotations[(side + 1) % 8]
		angle90 = 2, rotations[(side - 2) % 8], rotations[(side + 2) % 8]
		
		x, y = from_x, from_y
		while length:
			length_part = sign(length) if abs(length) > 1 else length
			pdx = dx * length_part
			pdy = dy * length_part
			
			left_free_part, right_free_part = physics.check_side(is_black, x + dx, y + dy, side, radius)
			if left_free_part == 1.0 and right_free_part == 1.0:
				dpoint = forward
			else:
				if left_free_part == right_free_part:
					break
				
				for dside, left, right in (angle45, angle90):
					if left_free_part > right_free_part:
						dside = -dside
						dpoint = left
					else:
						dpoint = right
				
					left_free_part, right_free_part = physics.check_side(is_black, x + dpoint[0], y + dpoint[1], side + dside, radius)
					if left_free_part == 1.0 and right_free_part == 1.0:
						break
					dpoint = None
			
			if not dpoint:
				break
			
			x += dpoint[0] * length_part
			y += dpoint[1] * length_part
			length -= length_part
		
		return start_x + x, start_y + y
	
	build_object('physics')

