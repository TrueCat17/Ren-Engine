init -1000 python:
	def start_mod(dir_name):
		if dir_name == 'tutorial':
			persistent.common_screen_tab = None
		elif common_screen.name:
			persistent.common_screen_tab = common_screen.name
		
		renpy.music.stop(channel = 'ambience', fadeout = black_cover.appearance_time)
		
		black_cover.start()
		set_timeout(Function(__builtins__.start_mod, dir_name), black_cover.appearance_time)
	
	def show_bg_entry():
		renpy.show('bg entry', is_scene = True)
		
		for name in ('planet', 'planet_light'):
			renpy.show('anim ' + name, tag = name, at = empty_transform)
		
		for i in '123456':
			renpy.show('anim monitor' + i, tag = 'monitor' + i, at = empty_transform)
