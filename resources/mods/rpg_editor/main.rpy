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


label start:
	while True:
		python:
			save_counter += 1
			if save_counter % 10 == 0 and need_save_locations:
				need_save_locations = False
				save()
		
		pause 0.1

