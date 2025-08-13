init 1 python:
	set_fps(30)
	set_can_mouse_hide(False)
	config.has_autosave = False
	
	start_screens = ['hotkeys', 'debug_screen', 'all_locations', 'menu']
	
	need_save_locations = False
	def set_save_locations():
		global need_save_locations
		need_save_locations = True
	
	def check_need_save_location():
		global need_save_locations
		if need_save_locations:
			need_save_locations = False
			save()
	set_interval(check_need_save_location, 1.0)
	
	
	def get_image_or_similar(path):
		cache = get_image_or_similar.__dict__
		res = cache.get(path)
		if res:
			return res
		
		directory = os.path.dirname(path) + '/'
		filename = os.path.basename(path) + '.'
		
		for name in os.listdir(directory):
			if name.startswith(filename):
				res = directory + name
				break
		else:
			res = im.rect('#888')
		
		cache[path] = res
		return res
	
	register_new_locations()
