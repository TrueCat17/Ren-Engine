init -1003 python:
	
	cam_object_start_moving = 0.0
	cam_object_end_moving = 0.0
	cam_object_align = (0.5, 0.5)
	cam_object_align_old = (0.5, 0.5)
	cam_object_zoom = 1.0
	cam_object_zoom_old = 1.0
	
	
	def cam_to(obj, moving_time = 1.0, align = None, zoom = None):
		if cur_location is None:
			out_msg('cam_to', 'Current location is not defined, need to call set_location')
			return
		
		old = cur_location.cam_object
		if isinstance(obj, str):
			place = cur_location.get_place(obj)
			if not place:
				out_msg('cam_to', 'Place <' + obj + '> not found in location <' + cur_location.name + '>')
				return
			cur_location.cam_object = place
		else:
			cur_location.cam_object = obj
		
		global cam_object_start_moving, cam_object_end_moving
		if cur_location.cam_object is not None:
			cur_location.cam_object_old = old
			cam_object_start_moving = get_game_time()
			cam_object_end_moving = cam_object_start_moving + max(moving_time, 0)
		
		if align is not None:
			global cam_object_align, cam_object_align_old
			if type(align) not in (list, tuple):
				align = {
					'left':   (0.0, 0.5),
					'right':  (1.0, 0.5),
					'up':     (0.5, 0.0),
					'down':   (0.5, 1.0),
					'center': (0.5, 0.5)
				}[align]
			cam_object_align_old = cam_object_align
			cam_object_align = align
		
		if zoom is not None:
			global cam_object_zoom, cam_object_zoom_old
			cam_object_zoom_old = cam_object_zoom
			cam_object_zoom = zoom
	
	
	loc_zoom_prev_stage_size = None
	loc_zoom_prev_result = None
	def get_location_zoom():
		global loc_zoom_prev_stage_size, loc_zoom_prev_result
		if loc_zoom_prev_stage_size == get_stage_size():
			return loc_zoom_prev_result
		stage_width, stage_height = loc_zoom_prev_stage_size = get_stage_size()
		
		res = 1.0
		for location_name in rpg_locations:
			location = rpg_locations[location_name]
			if location.is_room:
				zoom = min(stage_width / location.xsize, stage_height / location.ysize) # increase to width OR height
			else:
				zoom = max(stage_width / location.xsize, stage_height / location.ysize) # increase to width AND height
			
			res = max(res, zoom)
		
		loc_zoom_prev_result = min(res, config.max_location_zoom)
		return loc_zoom_prev_result
	
	
	def get_camera_params(location):
		k = get_k_between(cam_object_start_moving, cam_object_end_moving, get_game_time())
		if cam_object_end_moving <= get_game_time():
			global cam_object_align_old, cam_object_zoom_old
			cam_object_align_old = cam_object_align
			cam_object_zoom_old = cam_object_zoom
		
		if location.cam_object_old is None or location.cam_object is None or cam_object_end_moving < get_game_time():
			if location.cam_object is None:
				cam_object_x, cam_object_y = 0, 0
			else:
				cam_object_x, cam_object_y = get_place_center(location.cam_object, cam_object_align)
		else:
			ax = interpolate(cam_object_align_old[0], cam_object_align[0], k)
			ay = interpolate(cam_object_align_old[1], cam_object_align[1], k)
			
			from_x, from_y = get_place_center(location.cam_object_old, (ax, ay))
			to_x, to_y = get_place_center(location.cam_object, (ax, ay))
			
			cam_object_x = interpolate(from_x, to_x, k)
			cam_object_y = interpolate(from_y, to_y, k)
		
		global location_zoom
		zoom_main = get_location_zoom()
		zoom_extra = interpolate(cam_object_zoom_old, cam_object_zoom, k)
		location_zoom = zoom_main * zoom_extra
		
		cam_object_x *= location_zoom
		cam_object_y *= location_zoom
		
		
		stage_width = get_stage_width()
		main_width = location.xsize * location_zoom
		if main_width < stage_width or location.cam_object is None:
			x = (stage_width - main_width) / 2
		else:
			xalign = interpolate(cam_object_align_old[0], cam_object_align[0], k)
			indent = stage_width * xalign
			indent_right = stage_width - indent
			
			if cam_object_x <= indent:
				x = 0
			elif cam_object_x >= main_width - indent_right:
				x = stage_width - main_width
			else:
				x = indent - cam_object_x
		
		stage_height = get_stage_height() - location_cutscene_up - location_cutscene_down
		main_height = location.ysize * location_zoom
		if main_height < stage_height or location.cam_object is None:
			y = (stage_height - main_height) / 2
		else:
			yalign = interpolate(cam_object_align_old[1], cam_object_align[1], k)
			indent = stage_height * yalign
			indent_down = stage_height - indent
			
			if cam_object_y <= indent:
				y = 0
			elif cam_object_y >= main_height - indent_down :
				y = stage_height - main_height
			else:
				y = indent - cam_object_y
		y += location_cutscene_up
		
		return absolute(x), absolute(y)
	
	
	def cam_object_moved():
		return cam_object_end_moving <= get_game_time()
	can_exec_next_check_funcs.append(cam_object_moved)
	
	def cam_object_move():
		global cam_object_start_moving, location_cutscene_start, cam_object_end_moving, location_cutscene_end
		cam_object_start_moving = location_cutscene_start = 0.0
		cam_object_end_moving = location_cutscene_end = 0.0
	can_exec_next_skip_funcs.append(cam_object_move)

