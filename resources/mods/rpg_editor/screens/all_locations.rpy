init python:
	selected_location = None
	
	drag_location_name = None
	
	local_mouse_pos = None
	start_mouse_pos = None
	mouse_moved = True
	
	common_cam_x, common_cam_y = 0, 0
	
	common_k = absolute(1.0)
	common_k_min = absolute(0.25)
	common_k_max = absolute(2.0)
	
	common_speed = 25
	
	start_x, start_y = common_cam_x, common_cam_y
	
	def start_drag_location(name):
        global drag_location_name
        drag_location_name = name
        
		global local_mouse_pos, start_mouse_pos, mouse_moved
		local_mouse_pos = get_local_mouse()
        start_mouse_pos = get_mouse()
        mouse_moved = False
        
        global start_x, start_y
        start_x, start_y = x, y
	
	def select_location(name):
		global selected_location_name
		selected_location_name = name
		
		hide_screen('all_locations')
		show_screen('selected_location')


screen all_locations:
	image im.rect('#444'):
		size 1.0
	
	key 'w' action [AddVariable('common_cam_y', -common_speed), AddVariable('start_y', -common_speed)]
	key 'a' action [AddVariable('common_cam_x', -common_speed), AddVariable('start_x', -common_speed)]
	key 's' action [AddVariable('common_cam_y', +common_speed), AddVariable('start_y', +common_speed)]
	key 'd' action [AddVariable('common_cam_x', +common_speed), AddVariable('start_x', +common_speed)]
	
	for key in '-_':
		key key action SetVariable('common_k', max(common_k - common_k_min, common_k_min))
	for key in '+=':
		key key action SetVariable('common_k', min(common_k + common_k_min, common_k_max))
	
	python:
		x, y = int(-common_cam_x * common_k), int(-common_cam_y * common_k)
		
		mouse_down = get_mouse_down()
		if not mouse_down:
			if not mouse_moved:
				select_location(drag_location_name)
			drag_location_name = None
			local_mouse_pos = None
			mouse_moved = True
		
		if drag_location_name is not None:
			mouse_pos = get_mouse()
			if start_mouse_pos != mouse_pos:
				mouse_moved = True
			
			if mouse_pos[0] < get_stage_width() - locations_width:
				location = rpg_locations[drag_location_name]
				old_x, old_y = location.x, location.y
				
				new_x = (mouse_pos[0] - local_mouse_pos[0] - x) / common_k
				new_y = (mouse_pos[1] - local_mouse_pos[1] - y) / common_k
				
				if old_x != new_x or old_y != new_y:
					location.x, location.y = new_x, new_y
					set_save_locations()
	
	null:
		pos (x, y)
		zoom common_k
		
		for name, location in rpg_locations.items():
			if location.x is None:
				continue
			
			$ preview = get_preview(name)
			button:
				pos (location.x, location.y)
				size get_image_size(preview)
				
				ground preview
				hover preview
				
				action start_drag_location(name)
	
	use locations_list

