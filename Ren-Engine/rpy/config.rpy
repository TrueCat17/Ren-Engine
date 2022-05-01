init -997 python:
	
	start_screens = ['hotkeys', 'dialogue_box', 'debug_screen']
	
	
	config = renpy.config
	
	config.enable_language_autodetect = True
	config.default_language = 'english'
	
	config.quick_save_table = 'quick'
	config.quick_save_name  = '0'
	config.quick_load_key = 'L'
	config.quick_save_key = 'Q'
	
	config.count_prev_texts = 50
	
	# height calculate: width / get_from_hard_config("window_w_div_h", float)
	config.save_screenshot_width = 480
	
	
	if config.autosave is None:
		config.autosave = 60 # time, in sec.
	
	if config.shift_is_run is None:
		config.shift_is_run = False # If True, usual moving is walking, with shift - running
	
	if config.max_location_zoom is None:
		config.max_location_zoom = 2.5
	
	if config.text_cps is None:
		config.text_cps = 60 # cps - chars per second, for dialogue_box
	
	for std_mixer in std_mixers:
		if config[std_mixer + '_volume'] is None:
			config[std_mixer + '_volume'] = 1
		renpy.music.set_mixer_volume(config[std_mixer + '_volume'], std_mixer)

