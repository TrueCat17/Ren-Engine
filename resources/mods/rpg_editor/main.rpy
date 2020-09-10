init 1 python:
	set_can_mouse_hide(False)
	set_can_autosave(False)
	
	db_hide_interface = True # for disable pause-menu in screen <hotkeys>
	start_screens = ['hotkeys', 'all_locations', 'menu']
	
	need_save_locations = False
	save_counter = 0
	set_save_locations = SetVariable('need_save_locations', True)
	
	
	def get_image_or_similar(path):
		if os.path.exists(path):
			return path
		
		directory = os.path.dirname(path)
		path_start = os.path.splitext(path[len(directory)+1:])[0]
		for name in os.listdir(directory):
			if name.startswith(path_start):
				return os.path.join(directory, name)
		
		return im.rect('#888')
	
	
	register_new_locations()
	
	for location_name in locations:
		location = locations[location_name]
		location.using = location.x is not None and location.y is not None
		
		for place_name in location.places:
			place = location.places[place_name]
			px, py, pw, ph = place.x, place.y, place.xsize, place.ysize
			
			for exit in location.exits:
				if exit.to_place_name != location_name:
					continue
				
				ex, ey, ew, eh = exit.x, exit.y, exit.xsize, exit.ysize
				
				if px == ex and pw == ew:
					if py + ph == ey:
						place.side_exit = 'down'
					elif ey + eh == py:
						place.side_exit = 'up'
						place.y -= eh
					if place.side_exit:
						place.ysize += eh
						location.exits.remove(exit)
						break
				if py == ey and ph == eh:
					if px + pw == ex:
						place.side_exit = 'right'
					elif ex + ew == px:
						place.side_exit = 'left'
						place.x -= ew
					if place.side_exit:
						place.xsize += ew
						location.exits.remove(exit)
						break


label start:
	while True:
		python:
			save_counter += 1
			if save_counter % 10 == 0 and need_save_locations:
				need_save_locations = False
				save()
		
		pause 0.1

