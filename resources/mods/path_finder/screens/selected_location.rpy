screen selected_location:
	key 'ESCAPE' action unselect_location
	
	if me.paths:
		key 'RETURN' action stop_moving
		key 'SPACE'  action stop_moving
	elif me is cam_object:
		$ stop_moving()
	
	if not me.paths:
		key 'w' action SetDict(cam_object, 'y', in_bounds(cam_object['y'] - common_speed, 0, draw_location.ysize))
		key 'a' action SetDict(cam_object, 'x', in_bounds(cam_object['x'] - common_speed, 0, draw_location.xsize))
		key 's' action SetDict(cam_object, 'y', in_bounds(cam_object['y'] + common_speed, 0, draw_location.ysize))
		key 'd' action SetDict(cam_object, 'x', in_bounds(cam_object['x'] + common_speed, 0, draw_location.xsize))
	
		if draw_location and draw_location.free():
			image draw_location.free():
				alpha 0.5
				
				xpos int(draw_location.x)
				ypos int(draw_location.y)
				xsize int(draw_location.xsize * location_zoom)
				ysize int(draw_location.ysize * location_zoom)
		
		button:
			size get_stage_size()
			alpha 0.01
			mouse False
			
			ground im.rect("#000")
			hover  im.rect("#000")
			
			action add_point
