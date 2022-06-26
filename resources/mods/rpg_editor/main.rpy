init 1 python:
	set_fps(30)
	set_can_mouse_hide(False)
	set_can_autosave(False)
	
	pause_screen.disable = True
	start_screens = ['hotkeys', 'all_locations', 'menu']
	
	need_save_locations = False
	set_save_locations = SetVariable('need_save_locations', True)
	
	def check_need_save_location():
		global need_save_locations
		if need_save_locations:
			need_save_locations = False
			save()
	set_interval(check_need_save_location, 1)
	
	
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
