init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('selected_location')

screen selected_location:
	key 'ESCAPE' action unselect_location
	
	$ cam_object = draw_location.cam_object
	
	if me.paths:
		key 'RETURN' action stop_moving
		key 'SPACE'  action stop_moving
	elif me is cam_object:
		$ stop_moving()
	
	if not me.paths:
		key 'W' first_delay 0 action "cam_object['y'] = in_bounds(cam_object['y'] - common_speed, 0, draw_location.ysize)"
		key 'A' first_delay 0 action "cam_object['x'] = in_bounds(cam_object['x'] - common_speed, 0, draw_location.xsize)"
		key 'S' first_delay 0 action "cam_object['y'] = in_bounds(cam_object['y'] + common_speed, 0, draw_location.ysize)"
		key 'D' first_delay 0 action "cam_object['x'] = in_bounds(cam_object['x'] + common_speed, 0, draw_location.xsize)"
		
		if draw_location.free():
			image draw_location.free():
				alpha 0.5
				
				xpos int(draw_location.x)
				ypos int(draw_location.y)
				xsize int(draw_location.xsize * location_zoom)
				ysize int(draw_location.ysize * location_zoom)
		
		button:
			style 'path_finder_bg_btn'
			action add_point
