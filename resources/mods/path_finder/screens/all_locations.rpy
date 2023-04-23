init python:
	common_cam_x, common_cam_y = 0, 0
	
	common_k = absolute(1.0)
	common_k_min = absolute(0.25)
	common_k_max = absolute(2.0)
	
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
		cam_to({'x': location.xsize // 2, 'y': location.ysize // 2}, 0)
		
		global location_start_time
		location_start_time = 0
		
		save_locations_coords()
	
	def unselect_location():
		restore_locations_coords()
		
		hide_screen('location')
		hide_screen('selected_location')
		show_screen('all_locations')
		
		stop_moving()

screen all_locations:
	zorder -100
	
	image im.rect('#444'):
		size 1.0
	
	key 'w' action AddVariable('common_cam_y', -common_speed)
	key 'a' action AddVariable('common_cam_x', -common_speed)
	key 's' action AddVariable('common_cam_y', +common_speed)
	key 'd' action AddVariable('common_cam_x', +common_speed)
	
	for key in '-_':
		key key action SetVariable('common_k', max(common_k - common_k_min, common_k_min))
	for key in '+=':
		key key action SetVariable('common_k', min(common_k + common_k_min, common_k_max))
	
	null:
		zoom common_k
		xpos -common_cam_x
		ypos -common_cam_y
		
		for name, location in rpg_locations.items():
			$ k = absolute(128.0) / max(location.xsize, location.ysize)
			
			button:
				pos (location.x, location.y)
				size (location.xsize * k, location.ysize * k)
				
				ground location.main()
				hover  location.main()
				
				action select_location(location)
