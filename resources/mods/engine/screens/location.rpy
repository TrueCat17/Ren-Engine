init python:
	
	loc__background_alpha = 0.0
	
	draw_location = None
	
	loc__max_time = time.time() * 2
	
	
	rpg_control = False
	def get_rpg_control():
		return rpg_control
	
	def set_rpg_control(value):
		global rpg_control
		rpg_control = bool(value)
		me.move_kind = 'stay'
	
	prev_rpg_control = None
	def save_rpg_control():
		global prev_rpg_control
		prev_rpg_control = get_rpg_control()
	
	def return_prev_rpg_control():
		if prev_rpg_control is not None:
			set_rpg_control(prev_rpg_control)
			ignore_prev_rpg_control()
	
	def ignore_prev_rpg_control():
		global prev_rpg_control
		prev_rpg_control = None
	
	loc__prev_left = loc__prev_right = loc__prev_up = loc__prev_down = False
	loc__left = loc__right = loc__up = loc__down = False
	
	loc__directions = [to_left, to_right, to_forward, to_back]
	loc__direction = to_back
	loc__left_time = loc__right_time = loc__up_time = loc__down_time = loc__max_time
	
	loc__set_show_at_end = False
	
	def loc__get_min(a, b, c, d):
		return [a, b, c, d].index(min(a, b, c, d))
	
	
	loc__prev_time = 0
	def loc__move_character(dx, dy):
		global loc__prev_time
		
		if me.pose != 'stance' or not get_rpg_control():
			loc__prev_time = time.time()
			return
		
		if dx == 0 and dy == 0:
			me.move_kind = 'stay'
			loc__prev_time = time.time()
			return
		
		if dx and dy:
			dx /= 2 ** 0.5
			dy /= 2 ** 0.5
		
		me.fps =            character_run_fps if loc__shift_is_down else character_walk_fps
		me.move_kind =                  'run' if loc__shift_is_down else 'walk'
		character_speed = character_run_speed if loc__shift_is_down else character_walk_speed
		
		dtime = time.time() - loc__prev_time
		loc__prev_time = time.time()
		
		dx *= character_speed * dtime
		dy *= character_speed * dtime
		
		to_x, to_y = get_end_point(me.x, me.y, dx, dy)
		dx, dy = to_x - me.x, to_y - me.y
		if dx or dy:
			me.x = me.to_x = to_x
			me.y = me.to_y = to_y
		else:
			me.move_kind = 'stay'
	
	
	location_cutscene_back = im.Rect('#111')
	location_cutscene_size = 0.15
	
	location_cutscene_state = None # None | 'on' | 'off'
	location_cutscene_start = 0
	location_cutscene_end = 1.0
	
	def location_cutscene_on(t = 1.0, align = 'center', zoom = 1.2, obj = None):
		global location_cutscene_state, location_cutscene_start, location_cutscene_end
		location_cutscene_state = 'on'
		location_cutscene_start = time.time()
		location_cutscene_end = time.time() + max(t, 0.001)
		
		cam_to(obj or cam_object, t, align, zoom)
	
	def location_cutscene_off(t = 1.0, align = 'center', zoom = 1.0, obj = None):
		global location_cutscene_state, location_cutscene_start, location_cutscene_end
		location_cutscene_state = 'off'
		location_cutscene_start = time.time()
		location_cutscene_end = time.time() + max(t, 0.001)
		
		cam_to(obj or cam_object, t, align, zoom)


screen location:
	zorder -4
	
	python:
		dtime = time.time() - location_start_time
		
		# fade, back.alpha: 0 -> 1
		if dtime < location_fade_time:
			loc__background_alpha = dtime / location_fade_time
			
			if cur_location:
				cur_location.preload()
		
		# fade, back.alpha: 1 -> 0
		else:
			if dtime < location_fade_time * 2:
				loc__background_alpha = 1.0 - (dtime - location_fade_time) / location_fade_time
			else:
				if loc__background_alpha:
					loc__background_alpha = 0.0
			
			if draw_location is not cur_location:
				draw_location = cur_location
				cam_object = me
				loc__set_show_at_end = True
				
				if times['next_name']:
					set_time_direct()
				
				start_location_ambience()
		
		cut_k = 0
		if time.time() < location_cutscene_end:
			cut_k = get_k_between(location_cutscene_start, location_cutscene_end, time.time(), location_cutscene_state == 'off')
		elif location_cutscene_state == 'off':
			location_cutscene_state = None
		else:
			cut_k = 1.0
		
		if location_cutscene_state:
			location_cutscene_up = location_cutscene_down = int(location_cutscene_size * cut_k * get_stage_height())
		else:
			location_cutscene_up = location_cutscene_down = 0
	
	if draw_location:
		python:
			loc__shift_is_down = False
			
			loc__prev_left, loc__prev_right, loc__prev_up, loc__prev_down = loc__left, loc__right, loc__up, loc__down
			loc__left = loc__right = loc__up = loc__down = False
		
		if get_rpg_control() and location_showed():
			key 'z' action SetVariable('sit_action', True)
			key 'e' action SetVariable('exec_action', True)
			
			key 'LEFT SHIFT'  action SetVariable('loc__shift_is_down', True) first_delay 0
			key 'RIGHT SHIFT' action SetVariable('loc__shift_is_down', True) first_delay 0
			
			key 'LEFT'  action SetVariable('loc__left',  True) first_delay 0
			key 'RIGHT' action SetVariable('loc__right', True) first_delay 0
			key 'UP'    action SetVariable('loc__up',    True) first_delay 0
			key 'DOWN'  action SetVariable('loc__down',  True) first_delay 0
			key 'a'     action SetVariable('loc__left',  True) first_delay 0
			key 'd'     action SetVariable('loc__right', True) first_delay 0
			key 'w'     action SetVariable('loc__up',    True) first_delay 0
			key 's'     action SetVariable('loc__down',  True) first_delay 0
		
		python:
			if not config.shift_is_run:
				loc__shift_is_down = not loc__shift_is_down
			if not get_run_allow():
				loc__shift_is_down = False
			
			loc__character_dx = loc__character_dy = 0
			if loc__left:
				loc__character_dx -= 1
			if loc__right:
				loc__character_dx += 1
			if loc__up:
				loc__character_dy -= 1
			if loc__down:
				loc__character_dy += 1
			loc__move_character(loc__character_dx, loc__character_dy)
			
			if get_rpg_control() and me.get_pose() == 'stance':
				if loc__left and not loc__prev_left:
					loc__left_time = time.time()
				if loc__right and not loc__prev_right:
					loc__right_time = time.time()
				if loc__up and not loc__prev_up:
					loc__up_time = time.time()
				if loc__down and not loc__prev_down:
					loc__down_time = time.time()
				
				min_index = loc__get_min(loc__left_time  if loc__left  else loc__max_time,
				                         loc__right_time if loc__right else loc__max_time,
				                         loc__up_time    if loc__up    else loc__max_time,
				                         loc__down_time  if loc__down  else loc__max_time)
				if (loc__left, loc__right, loc__up, loc__down)[min_index]:
					loc__direction = loc__directions[min_index]
					me.set_direction(loc__direction)
			
			for obj in draw_location.objects:
				if obj.update:
					obj.update()
			
			draw_location.objects.sort(key = lambda obj: obj.get_zorder())
			
			draw_location.update_pos()
		
		null:
			clipping True
			zoom location_zoom
			pos  (draw_location.x, draw_location.y)
			size (draw_location.xsize, draw_location.ysize)
			
			python:
				list_to_draw = []
				
				for obj in draw_location.objects:
					if obj.invisible:
						continue
					
					datas = obj.get_draw_data()
					if type(datas) in (tuple, list):
						for data in datas:
							list_to_draw.append(data)
					else:
						list_to_draw.append(datas)
			
			for obj in list_to_draw:
				image obj['image']:
					pos    obj['pos']
					anchor obj['anchor']
					size   obj['size']
					crop   obj['crop']
					alpha  obj['alpha']
		
		image location_cutscene_back:
			xsize 1.0
			ysize location_cutscene_up
		
		image location_cutscene_back:
			ypos get_stage_height() - location_cutscene_down
			xsize 1.0
			ysize location_cutscene_down
		
		
		image 'images/bg/black.jpg':
			size (1.0, 1.0)
			alpha loc__background_alpha
		
		if times['next_name'] is not None:
			text times[times['next_name']]['text']:
				text_size 50
				color 0xFFFFFF
				align (0.5, 0.5)
		
		python:
			if loc__set_show_at_end:
				loc__set_show_at_end = False
				location_was_show = True
				return_prev_rpg_control()

