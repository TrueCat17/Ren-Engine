init -1100 python:
	
	start_screens = ['hotkeys', 'debug_screen', 'dialogue_box']
	
	
	config = renpy.config
	
	config.enable_language_autodetect = True
	config.default_language = 'english'
	
	# height calculate: width / get_from_hard_config("window_w_div_h", float)
	config.thumbnail_width = 480
	
	
	config.long_next_is_skipping = True
	
	config.has_quicksave = True
	
	config.has_autosave = True
	if config.autosave is None:
		config.autosave = 60 # time, in sec.
	
	if config.default_moving_is_run is None:
		config.default_moving_is_run = True
	
	if config.max_location_zoom is None:
		config.max_location_zoom = 2.5
	
	if config.text_cps is None:
		config.text_cps = 60 # cps - chars per second, for dialogue_box
	
	for std_mixer in std_mixers:
		if config[std_mixer + '_volume'] is None:
			config[std_mixer + '_volume'] = 1
		renpy.music.set_mixer_volume(config[std_mixer + '_volume'], std_mixer)

