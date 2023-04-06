init python:
	location_ambience_path = None
	renpy.music.register_channel("location_ambience", "ambience", True)
	
	
	default_location_ambience_paths = { None: '' }
	default_location_ambience_volume = 1.0
	
	def set_default_location_ambience(paths, volume = 1.0):
		if type(paths) is str:
			paths = {
				None: paths
			}
		
		global default_location_ambience_paths, default_location_ambience_volume
		default_location_ambience_paths = paths
		default_location_ambience_volume = volume
	
	def set_location_ambience(location_name, paths, volume = 1.0):
		if location_name not in rpg_locations:
			out_msg('set_location_ambience', 'Location <' + location_name + '> not registered')
			return
		
		if type(paths) is str:
			paths = {
				None: paths
			}
		
		location = rpg_locations[location_name]
		location.ambience_paths = paths
		location.ambience_volume = volume
	
	
	def get_location_ambience(location):
		if not location:
			return None
		
		paths = location.ambience_paths
		if paths is None:
			paths = default_location_ambience_paths
		
		time = times['current_name']
		if time not in paths:
			time = None
		
		return paths.get(time, None)
	
	def start_location_ambience():
		path = get_location_ambience(cur_location)
		
		global location_ambience_path
		if path and path != location_ambience_path:
			renpy.play(path, "location_ambience", fadein = location_fade_time)
		
		location_ambience_path = path
		renpy.music.set_volume(cur_location.ambience_volume, "location_ambience")
	
	def end_location_ambience(next_location):
		path = get_location_ambience(next_location)
		if location_ambience_path and path != location_ambience_path:
			renpy.stop("location_ambience", fadeout = location_fade_time)
