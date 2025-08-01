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
		
		level = levels[cur_level]
		xcell, ycell = me.x // cell_size, me.y // cell_size
		x, y = me.x % cell_size, me.y % cell_size
		
		symbol = get_symbol(level, xcell, ycell)
		obj_type = level_classes[symbol]
		
		on_ground = get_on_ground(xcell, ycell, x, y)
		
		
		if left == right:
			if left: # and right
				me.set_direction(to_back)
		else:
			speed = (me_xspeed if hotkeys.shift else me_xspeed_run) * cell_size * physics_step
			
			me_vx += speed * (-1 if left else 1)
			
			if on_ground or not obj_type.can_vertical:
				me.set_direction(to_left if left else to_right)
		
		
		no_gravity = False
		
		if on_ground:
			me_vy = 0
			if space:
				me_vy -= me_jump
		
		if obj_type.can_vertical:
			if up or down:
				me.set_direction(to_forward)
				
				speed = me_yspeed * physics_step * cell_size
				me_vy += speed * (-1 if up else 1)
			
			if not on_ground:
				me_vx *= friction
				me_vy *= friction
			
			if me.get_direction() == to_forward:
				no_gravity = True
		
		if not no_gravity:
			me_vy += gravity
		
		
		global pltf_lost_dxy
		full_dx = me_vx * physics_step + pltf_lost_dxy[0]
		full_dy = me_vy * physics_step + pltf_lost_dxy[1]
		me_vx *= friction
		if abs(me_vx) < 0.15:
			me_vx = 0
		
		def sign(x):
			return -1 if x < 0 else 1 if x > 0 else 0
		def part(x):
			return 0 if abs(x) < 1 else sign(x)
		def to_zero(x):
			return x - part(x)
		
		i = 0
		while int(full_dx) or int(full_dy):
			dx = dy = 0
			
			i += 1
			if i % 2:
				dx = part(full_dx)
				full_dx = to_zero(full_dx)
				if not dx:
					continue
				
				if dx < 0:
					if x == 0:
						x += cell_size
						xcell -= 1
				else:
					if x == cell_size - 1:
						x -= cell_size
						xcell += 1
			else:
				dy = part(full_dy)
				full_dy = to_zero(full_dy)
				if not dy:
					continue
				
				if dy < 0:
					if y == 0:
						y += cell_size
						ycell -= 1
				else:
					if y == cell_size - 1:
						y -= cell_size
						ycell += 1
			
			
			symbol = get_symbol(level, xcell, ycell - 1)
			obj_type = level_classes[symbol]
			if hasattr(obj_type, 'physics'):
				_for_event_cheking = obj_type.physics((xcell, ycell - 1), (x, y), dx, dy) # coin, exit...
			
			
			symbol = get_symbol(level, xcell, ycell)
			obj_type = level_classes[symbol]
			
			if hasattr(obj_type, 'physics'):
				x, y = obj_type.physics((xcell, ycell), (x, y), dx, dy)
			else:
				x += dx
				y += dy
			
			xcell += x // cell_size
			x %= cell_size
			ycell += y // cell_size
			y %= cell_size
		
		pltf_lost_dxy = (full_dx, full_dy)
		
		me.x = xcell * cell_size + x
		me.y = ycell * cell_size + y
		
		moving_dtime = get_game_time() - last_stay_time
		fps = me.walk_fps if hotkeys.shift else me.run_fps
		me.set_frame(int(moving_dtime * fps))
		me.set_pose('walk' if moving_dtime else 'stay')
		
		me.update_crop()
		
		if next_level is not None:
			set_level(next_level)
	
	
	
	def get_symbol(level, xcell, ycell):
		if xcell < 0 or xcell >= level_w or ycell < 0 or ycell >= level_h:
			return ' '
		return level[ycell][xcell]
	
	def get_on_ground(xcell, ycell, x, y):
		if y == cell_size - 1:
			y = -1
			ycell += 1
		
		symbol = get_symbol(levels[cur_level], xcell, ycell)
		obj_type = level_classes[symbol]
		
		if not hasattr(obj_type, 'physics'):
			return False
		
		tx, ty = obj_type.physics((xcell, ycell), (x, y), 0, 1)
		free_to_down = tx == x and ty == y + 1
		return not free_to_down
