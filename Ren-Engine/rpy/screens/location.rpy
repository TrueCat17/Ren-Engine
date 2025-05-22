init python:
	
	draw_location = None
	
	rpg_event = None
	rpg_event_object = None
	rpg_events = set()
	
	cur_exit = None
	
	
	rpg_control = False
	def get_rpg_control():
		return rpg_control
	
	def set_rpg_control(value):
		global rpg_control
		rpg_control = bool(value)
		if me.get_pose() in ('walk', 'run'):
			me.set_pose('stay')
	
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
	
	loc__max_time = 1e9
	loc__directions = [to_left, to_right, to_forward, to_back]
	loc__left_time = loc__right_time = loc__up_time = loc__down_time = loc__max_time
	
	
	def loc__move_character():
		dx = dy = 0
		if screen_tmp.left:
			dx -= 1
		if screen_tmp.right:
			dx += 1
		if screen_tmp.up:
			dy -= 1
		if screen_tmp.down:
			dy += 1
		
		if dx or dy:
			if get_run_allow():
				run = hotkeys.shift
				if config.default_moving_is_run:
					run = not run
			else:
				run = False
			
			pose = 'run' if run else 'walk'
			speed = me[pose + '_speed']
			
			to_x, to_y = physics.get_end_point(me.x, me.y, dx, dy, speed * get_last_tick())
			dx, dy = to_x - me.x, to_y - me.y
		
		if dx or dy:
			me.x = to_x
			me.y = to_y
			me.fps = me[pose + '_fps']
			me.set_pose(pose)
		else:
			me.set_pose('stay')
	
	
	def loc__rotate_character():
		global loc__left_time, loc__right_time, loc__up_time, loc__down_time
		
		if screen_tmp.left_pressed_now:
			loc__left_time = get_game_time()
		if screen_tmp.right_pressed_now:
			loc__right_time = get_game_time()
		if screen_tmp.up_pressed_now:
			loc__up_time = get_game_time()
		if screen_tmp.down_pressed_now:
			loc__down_time = get_game_time()
		
		side_times = (
			loc__left_time  if screen_tmp.left  else loc__max_time,
			loc__right_time if screen_tmp.right else loc__max_time,
			loc__up_time    if screen_tmp.up    else loc__max_time,
			loc__down_time  if screen_tmp.down  else loc__max_time,
		)
		min_value = min(side_times)
		
		if min_value != loc__max_time:
			min_index = side_times.index(min_value)
			direction = loc__directions[min_index]
			me.set_direction(direction)
	
	
	def loc__process_sit_action():
		if me.get_pose() == 'sit':
			me.stand_up()
			rpg_events.add('stand_up')
			signals.send('rpg-stand_up')
		else:
			objs = get_near_sit_objects()
			if objs:
				obj, point = objs[0]
				me.sit_down(obj)
				rpg_events.add('sit_down')
				signals.send('rpg-sit_down')
	
	def loc__process_action():
		obj = get_near_location_object_for_inventory()
		if obj is not None:
			left = inventory.add(obj.type, 1)
			if left == 0:
				remove_location_object(cur_location_name, me, obj.type, 1)
				inventory.add_event('take', obj.type)
		else:
			rpg_events.add('action')
			if cur_exit:
				loc__process_exit()
			else:
				signals.send('rpg-action')
	
	def loc__process_exit():
		if not cur_exit:
			return
		next_loc = rpg_locations.get(cur_exit.to_location_name)
		if not next_loc:
			out_msg('loc__process_exit', 'Location <%s> was not registered' % (cur_exit.to_location_name, ))
			return
		
		if cur_location.is_room or next_loc.is_room:
			if 'action' not in rpg_events:
				return
			rpg_events.remove('action')
		
		loc_place = (cur_location.name, cur_exit.name)
		if loc_place in location_banned_exits and loc_place not in me.allowed_exits:
			rpg_events.add('no_exit')
			signals.send('rpg-no_exit')
			return
		
		set_location(cur_exit.to_location_name, cur_exit.to_place_name)
		me.set_direction(cur_exit.to_side)
	
	
	
	location_cutscene_back = im.rect('#111')
	location_cutscene_size = 0.15
	
	location_cutscene_state = None # None | 'on' | 'off'
	location_cutscene_start = 0
	location_cutscene_end = 1.0
	
	def location_cutscene_on(t = 1.0, align = 'center', zoom = 1.2, obj = None):
		global location_cutscene_state, location_cutscene_start, location_cutscene_end
		location_cutscene_state = 'on'
		location_cutscene_start = get_game_time()
		location_cutscene_end = location_cutscene_start + max(t, 0)
		
		cam_to(obj or cur_location.cam_object, t, align, zoom)
	
	def location_cutscene_off(t = 1.0, align = 'center', zoom = 1.0, obj = None):
		global location_cutscene_state, location_cutscene_start, location_cutscene_end
		location_cutscene_state = 'off'
		location_cutscene_start = get_game_time()
		location_cutscene_end = location_cutscene_start + max(t, 0)
		
		cam_to(obj or cur_location.cam_object, t, align, zoom)
	
	def loc__calculate_cut_params():
		global location_cutscene_state, location_cutscene_up, location_cutscene_down
		
		cut_k = 0
		if get_game_time() < location_cutscene_end:
			cut_k = get_k_between(location_cutscene_start, location_cutscene_end, get_game_time(), location_cutscene_state == 'off')
		elif location_cutscene_state == 'off':
			location_cutscene_state = None
		else:
			cut_k = 1.0
		
		if location_cutscene_state:
			location_cutscene_up = location_cutscene_down = int(location_cutscene_size * cut_k * get_stage_height())
		else:
			location_cutscene_up = location_cutscene_down = 0
	
	def loc__get_list_to_draw():
		sequences = (tuple, list)
		floor = math.floor
		zoom = location_zoom
		
		res = []
		for obj in draw_location.objects:
			if obj.get('invisible'):
				continue
			
			datas = obj.get_draw_data()
			if type(datas) not in sequences:
				datas = (datas, )
			
			for data in datas:
				if not data.image:
					continue
				
				if 'size' in data:
					size = data.size
					if type(size) in sequences:
						w, h = size
					else:
						w = h = size
				else:
					w, h = get_image_size(data.image)
				
				pos = data.pos
				if type(pos) in sequences:
					x, y = pos
				else:
					x = y = pos
				
				if 'anchor' in data:
					anchor = data.anchor
					if type(anchor) in sequences:
						xa, ya = anchor
					else:
						xa = ya = anchor
					
					x -= get_absolute(xa, w)
					y -= get_absolute(ya, h)
				
				x *= zoom
				y *= zoom
				floor_x = floor(x)
				floor_y = floor(y)
				data.pos = floor_x, floor_y
				
				floor_w = floor(x + w * zoom) - floor_x
				floor_h = floor(y + h * zoom) - floor_y
				data.size = floor_w, floor_h
				
				res.append(data)
		
		def get_zorder(d):
			try:
				return d.zorder
			except:
				return d.pos[1]
		res.sort(key = get_zorder)
		
		return res
	
	def loc__update():
		for character in characters:
			character.update()
		
		if not has_screen('pause'):
			for obj in draw_location.objects.copy(): # copy for case <object removed itself>
				if isinstance(obj, Character):
					continue
				
				obj_update = getattr(obj, 'update', None)
				if obj_update:
					obj_update()
		
		draw_location.update_pos()


screen location:
	zorder -4
	
	python:
		screen_tmp = SimpleObject()
		screen_tmp.set_show_at_end = False
		
		dtime = get_game_time() - location_start_time
		
		# fade, back.alpha: 0 -> 1
		if dtime < location_fade_time:
			screen_tmp.alpha = dtime / location_fade_time
			
			if cur_location:
				cur_location.preload()
		
		# fade, back.alpha: 1 -> 0
		else:
			if dtime < location_fade_time * 2:
				screen_tmp.alpha = 1 - (dtime - location_fade_time) / location_fade_time
			else:
				screen_tmp.alpha = 0
			
			if times['next_name']:
				set_time_direct()
				screen_tmp.set_show_at_end = True
			
			if not location_was_show:
				draw_location = cur_location
				screen_tmp.set_show_at_end = True
				
				start_location_ambience()
		
		loc__calculate_cut_params()
	
	if draw_location:
		key 'I' action inventory.show
		
		python:
			for side in ('left', 'right', 'up', 'down'):
				screen_tmp[side] = False
				screen_tmp[side + '_pressed_now'] = False
			
			if rpg_event_processing:
				rpg_events.clear()
		
		if get_rpg_control() and location_showed() and (dtime - location_fade_time) > location_time_without_control:
			key 'z' delay 0.333 action loc__process_sit_action
			key 'e' delay 0.333 action loc__process_action
			
			for side, key in (('left', 'A'), ('right', 'D'), ('up', 'W'), ('down', 'S')):
				key key action 'screen_tmp[side]                  = True' first_delay 0
				key key action 'screen_tmp[side + "_pressed_now"] = True' first_delay 1e9
		
		python:
			if get_rpg_control() and me.get_pose() != 'sit':
				loc__move_character()
				loc__rotate_character()
			
			prev_exit = cur_exit
			cur_exit = get_location_exit()
			if cur_exit != prev_exit and get_rpg_control() and (me.pose in ('walk', 'run') or 'action' in rpg_events):
				loc__process_exit()
			
			cur_place = get_location_place()
			cur_place_name = cur_place.name if cur_place else ''
			if cur_place_name and cur_place_name != prev_place_name and get_rpg_control():
				rpg_events.add('enter')
				signals.send('rpg-place')
			prev_place_name = cur_place_name
			
			loc__update()
		
		null:
			clipping True
			xpos draw_location.x
			ypos draw_location.y
			xsize int(draw_location.xsize * location_zoom)
			ysize int(draw_location.ysize * location_zoom)
			
			for obj in loc__get_list_to_draw():
				image obj.image:
					pos    obj.pos
					size   obj.size
					crop   obj.get('crop', (0, 0, 1.0, 1.0))
					alpha  obj.get('alpha', 1)
					rotate obj.get('rotate', 0)
		
		image location_cutscene_back:
			xsize 1.0
			ysize location_cutscene_up
		
		image location_cutscene_back:
			ypos get_stage_height() - location_cutscene_down
			xsize 1.0
			ysize location_cutscene_down
		
		
		image 'images/bg/black.jpg':
			size 1.0
			alpha screen_tmp.alpha
		
		python:
			if screen_tmp.set_show_at_end:
				location_was_show = True
				return_prev_rpg_control()
