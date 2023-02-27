init python:
	gravity = cell_size / 4
	friction = 0.85
	
	me_radius = 6
	me_jump = 6 * cell_size
	me_xspeed_run = 80
	me_xspeed = 20
	me_yspeed = 20
	
	
	me_vx = me_vy = 0
	pltf_lost_dxy = (0, 0)
	def update_physics():
		global me_vx, me_vy
		
		global next_level
		next_level = None
		
		def sign(x):
			return -1 if x < 0 else 1 if x > 0 else 0
		def part(x):
			return 0 if abs(x) < 1 else sign(x)
		def to_zero(x):
			return x if abs(x) < 1 else x + 1 if x < 0 else x - 1
		def floor(x):
			return int(math.floor(x))
		
		
		level = levels[cur_level]
		xcell, ycell = floor(me.x / cell_size), floor(me.y / cell_size)
		x, y = me.x - xcell * cell_size, me.y - ycell * cell_size
		
		symbol = get_symbol(level, floor(xcell + float(x) / cell_size + 0.5), floor(ycell + float(y - 1) / cell_size + 1))
		obj_type = level_classes[symbol]
		
		
		if left == right:
			me.move_kind = 'stay'
			if left: # and right
				me.set_direction(to_back)
		else:
			speed = (me_xspeed if shift_is_down else me_xspeed_run) * cell_size
			
			if left:
				me_vx -= speed * physics_step
				if not obj_type.can_vertical:
					me.move_kind = 'stance'
					me.set_direction(to_left)
			else: # right
				me_vx += speed * physics_step
				if not obj_type.can_vertical:
					me.move_kind = 'stance'
					me.set_direction(to_right)
		
		
		no_gravity = False
		if obj_type.can_vertical:
			if up or down:
				me.move_kind = 'stay'
				me.set_direction(to_forward)
				
				speed = me_yspeed * cell_size
				if up:
					me_vy -= speed * physics_step
				else:
					me_vy += speed * physics_step
			me_vx *= friction
			me_vy *= friction
			
			if me.direction == to_forward:
				no_gravity = True
		else:
			if on_ground((xcell, ycell), (floor(x + cell_size / 2), floor(y + cell_size - 1))):
				me_vy = gravity * 0
				if space:
					me_vy -= me_jump
		
		if not no_gravity:
			me_vy += gravity
		
		
		global pltf_lost_dxy
		full_dx = me_vx * physics_step + pltf_lost_dxy[0]
		full_dy = me_vy * physics_step + pltf_lost_dxy[1]
		me_vx *= friction
		if abs(me_vx) < 0.15:
			me_vx = 0
		
		i = 0
		while int(full_dx) or int(full_dy):
			ax, ay = cell_size / 2, cell_size - 1 # anchor; default - down border, point (0.5, 1.0)
			dx = dy = 0
			
			i += 1
			if i % 2:
				dx = part(full_dx)
				full_dx = to_zero(full_dx)
				if not dx:
					continue
				
				x += ax
				y += ay
				
				xcell += floor(float(x) / cell_size)
				x %= cell_size
				ycell += floor(float(y) / cell_size)
				y %= cell_size
				
				if dx < 0:
					if floor(x) == 0:
						x += cell_size
						xcell -= 1
				else:
					if floor(x) == cell_size - 1:
						x -= cell_size
						xcell += 1
				
				symbol = get_symbol(level, xcell, ycell)
				obj_type = level_classes[symbol]
			else:
				dy = part(full_dy)
				full_dy = to_zero(full_dy)
				if not dy:
					continue
				
				if dy < 0:
					ay = 0 # up border
					
				x += ax
				y += ay
				
				xcell += floor(float(x) / cell_size)
				x %= cell_size
				ycell += floor(float(y) / cell_size)
				y %= cell_size
				
				if dy < 0:
					if floor(y) == 0:
						y += cell_size
						ycell -= 1
				else:
					if floor(y) == cell_size - 1:
						y -= cell_size
						ycell += 1
				
				symbol = get_symbol(level, xcell, ycell)
				obj_type = level_classes[symbol]
			
			if 'physics' not in level_class_props[symbol]:
				x += dx
				y += dy
			else:
				lx, ly = x - floor(x), y - floor(y)
				x, y = obj_type.physics((xcell, ycell), (floor(x), floor(y)), dx, dy)
				x, y = x + lx, y + ly
			
			x -= ax
			y -= ay
		
		pltf_lost_dxy = (full_dx, full_dy)
		
		me.x = xcell * cell_size + x
		me.y = ycell * cell_size + y
		
		moving_dtime = get_game_time() - last_stay_time
		fps = me.walk_fps if shift_is_down else me.run_fps
		me.set_frame(int(moving_dtime * fps))
		
		me.update_crop()
		
		if next_level is not None:
			set_level(next_level)
	
	
	
	def get_symbol(level, xcell, ycell):
		if xcell < 0 or xcell >= level_w or ycell < 0 or ycell >= level_h:
			return ' '
		return level[ycell][xcell]
	
	def on_ground(cell, pixel):
		xcell, ycell = cell
		x, y = pixel
		
		xcell += int(math.floor(float(x) / cell_size))
		x %= cell_size
		ycell += int(math.floor(float(y) / cell_size))
		y %= cell_size
		
		if y == cell_size - 1:
			y = -1
			ycell += 1
		
		symbol = get_symbol(levels[cur_level], xcell, ycell)
		
		if 'physics' not in level_class_props[symbol]:
			return False
		
		obj_type = level_classes[symbol]
		tx, ty = obj_type.physics((xcell, ycell), (x, y), 0, 1)
		free_to_down = tx == x and ty == y + 1
		return not free_to_down

