init python:
	
	def get_map_free():
		location_free = cur_location.free()
		
		size = 64
		half_size = size // 2 # > (radius + some_indent) * 2
		me_x = in_bounds(me.x, 0, get_image_width(location_free) - 1)
		me_y = in_bounds(me.y, 0, get_image_height(location_free) - 1)
		start_x = int(round(me_x / half_size) - 1) * half_size
		start_y = int(round(me_y / half_size) - 1) * half_size
		
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
			obj_free = obj.free() if callable(obj.free) else None
			if obj_free is None or isinstance(obj, RpgLocation):
				continue
			if obj.frames and obj.frames > 1:
				obj_free = im.crop(obj_free, obj.crop)
			
			w, h = get_image_size(obj_free)
			x, y = obj.x + obj.xoffset - get_absolute(obj.xanchor, w), obj.y + obj.yoffset - get_absolute(obj.yanchor, h)
			if near(x, y, w, h):
				to_draw += [(x - start_x, y - start_y), im.matrix_color(obj_free, matrix)]
		
		for character in characters:
			if not character.invisible and near(character.x, character.y, 0, 0):
				to_draw += [(int(character.x - cs // 2 - start_x), int(character.y - cs // 2 - start_y)), im.rect('#FFF', cs, cs)]
		
		if len(to_draw) == 3: # 3 - [size, pos0, image0]
			return location_free, 0, 0
		return im.composite(*to_draw), start_x, start_y
	
	def get_end_point(from_x, from_y, dx, dy):
		to_x = in_bounds(from_x + dx, 0, cur_location.xsize)
		to_y = in_bounds(from_y + dy, 0, cur_location.ysize)
		dx, dy = to_x - from_x, to_y - from_y
		if dx == 0 and dy == 0:
			return to_x, to_y
		
		free, start_x, start_y = get_map_free()
		if free is None:
			return to_x, to_y
		
		from_x -= start_x
		from_y -= start_y
		to_x -= start_x
		to_y -= start_y
		
		black_color = 255 # r, g, b, a = 0, 0, 0, 255
		map_width, map_height = get_image_size(free)
		
		def is_black(x, y):
			x, y = int(x), int(y)
			if x < 0 or x >= map_width or y < 0 or y >= map_height:
				return False
			return get_image_pixel(free, x, y) == black_color
		
		s2 = 1 / (2 ** 0.5)
		rotations = (
			(-s2, -s2), # left-up: x == -1, y == -1
			(  0, -1 ), # up
			( s2, -s2), # right-up
			(  1,  0 ), # ...
			( s2,  s2),
			(  0,  1 ),
			(-s2,  s2),
			( -1,  0 )
		)
		
		
		def to_zero(x):
			return 0 if abs(x) < 1 else x + 1 if x < 0 else x - 1
		
		def part(x):
			return sign(x) if abs(x) > 1 else x
		
		sdx, sdy = sign(dx), sign(dy)
		if sdx and sdy:
			sdx, sdy = sdx * s2, sdy * s2
		
		rot_index = rotations.index( (sdx, sdy) )
		left    = rotations[(rot_index - 3) % len(rotations)]
		forward = rotations[(rot_index    ) % len(rotations)]
		right   = rotations[(rot_index + 3) % len(rotations)]
		
		left1, right1 = rotations[(rot_index - 1) % len(rotations)], rotations[(rot_index + 1) % len(rotations)]
		left2, right2 = rotations[(rot_index - 2) % len(rotations)], rotations[(rot_index + 2) % len(rotations)]
		
		radius = me.radius
		x, y = from_x, from_y
		fx, fy = x + radius * forward[0], y + radius * forward[1]
		while int(x + dx) != int(x) or int(y + dy) != int(y):
			pdx, pdy = part(dx), part(dy)
			
			left_last_block = 0
			left_first_block = 0
			left_first_free = radius
			for dist in range(1, radius):
				is_free = is_black(fx + pdx + dist * left[0] + left1[0], fy + pdy + dist * left[1] + left1[1])
				
				if not is_free:
					left_last_block = dist
					if not left_first_block:
						left_first_block = dist
				else:
					if left_first_free == radius:
						left_first_free = dist
			
			right_last_block = 0
			right_first_block = 0
			right_first_free = radius
			for dist in range(1, radius):
				is_free = is_black(fx + pdx + right1[0] + dist * right[0], fy + pdy + right1[1] + dist * right[1])
				
				if not is_free:
					right_last_block = dist
					if not right_first_block:
						right_first_block = dist
				else:
					if right_first_free == radius:
						right_first_free = dist
			
			changed = True
			if left_first_block or right_first_block:
				side = 'no'
				
				if not left_first_block and right_first_block:
					side = 'left'
				elif not right_first_block and left_first_block:
					side = 'right'
				else: # left_first_block and right_first_block
					if left_first_free < right_first_free:
						side = 'left'
					elif left_first_free > right_first_free:
						side = 'right'
					else:
						for dist in range(radius):
							left_free = is_black(fx + pdx + left1[0] + dist * left2[0], fy + pdy + left1[1] + dist * left2[1])
							right_free = is_black(fx + pdx + right1[0] + dist * right2[0], fy + pdy + right1[1] + dist * right2[1])
							
							if left_free or right_free:
								if left_free and not right_free and left_last_block <= 1:
									side = 'left'
								elif right_free and not left_free and right_last_block <= 1:
									side = 'right'
								break
				
				if side == 'left':
					extra_free = is_black(fx + pdx + left1[0], fy + pdy + left1[1])
					dpoint = left1 if extra_free else left2
				elif side == 'right':
					extra_free = is_black(fx + pdx + right1[0], fy + pdy + right1[1])
					dpoint = right1 if extra_free else right2
				else:
					changed = False
			else:
				dpoint = (pdx, pdy)
			
			
			if not changed:
				dx = dy = 0
			else:
				dx, dy = to_zero(dx), to_zero(dy)
				x, y = x + dpoint[0], y + dpoint[1]
				fx, fy = x + radius * forward[0], y + radius * forward[1]
		
		return start_x + x + dx, start_y + y + dy

