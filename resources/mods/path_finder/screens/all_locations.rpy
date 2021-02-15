init python:
	common_cam_x, common_cam_y = 0, 0
	common_k = 1.0
	common_speed = 25
	
	locations_coords = {}
	def save_locations_coords():
		for name in rpg_locations:
			locations_coords[name] = rpg_locations[name].x, rpg_locations[name].y
	def restore_locations_coords():
		for name in rpg_locations:
			rpg_locations[name].x, rpg_locations[name].y = locations_coords[name]
	
	def select_location(location):
		global selected_location_coords
		selected_location_coords = location.x, location.y
		
		hide_screen('all_locations')
		show_screen('selected_location')
		
		set_location(location.name, {'x': 0, 'y': 0})
		me.paths = None
		hide_character(me)
		cam_to({'x': location.xsize / 2, 'y': location.ysize / 2}, 0)
		
		global location_start_time
		location_start_time = 0
		
		save_locations_coords()
	
	def unselect_location():
		restore_locations_coords()
		
		hide_screen('location')
		hide_screen('selected_location')
		show_screen('all_locations')
		
		stop_moving()
		global draw_location
		draw_location = None

screen all_locations:
	zorder -100
	
	image im.rect('#444'):
		size (1.0, 1.0)
	
	key 'w' action AddVariable('common_cam_y', -common_speed)
	key 'a' action AddVariable('common_cam_x', -common_speed)
	key 's' action AddVariable('common_cam_y', +common_speed)
	key 'd' action AddVariable('common_cam_x', +common_speed)
	
	key '9' action SetVariable('common_k', max(common_k - 0.25, 0.25))
	key '0' action SetVariable('common_k', min(common_k + 0.25, 2.00))
	
	null:
		xpos int(-common_cam_x * common_k)
		ypos int(-common_cam_y * common_k)
		
		for name, location in rpg_locations.iteritems():
			$ k = 128.0 / max(location.xsize, location.ysize)
			
			button:
				pos (int(location.x * common_k), int(location.y * common_k))
				size (int(k * location.xsize * common_k), int(k * location.ysize * common_k))
				
				ground location.main()
				hover  location.main()
				
				action select_location(location)
