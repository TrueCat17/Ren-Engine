init -1100 python:
	
	start_screens = ['hotkeys', 'debug_screen', 'dialogue_box']
	
	
	
	config = renpy.config
	
	
	config.set_prop_is_not_persistent('enable_language_autodetect')
	config.set_prop_is_not_persistent('default_language')
	config.set_prop_is_not_persistent('thumbnail_width')
	
	config.set_prop_is_not_persistent('long_next_is_skipping')
	config.set_prop_is_not_persistent('has_quicksave')
	config.set_prop_is_not_persistent('has_autosave')
	
	config.set_prop_is_not_persistent('width')
	config.set_prop_is_not_persistent('height')
	
	config.set_prop_is_not_persistent('fadeout_audio')
	
	
	config.enable_language_autodetect = True
	config.default_language = 'english'
	
	
	# height calculate: width / get_from_hard_config('window_w_div_h', float)
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
	
	if config.skip_after_choices is None:
		config.skip_after_choices = False
	
	if config.pause_before_skip_on_ctrl is None:
		config.pause_before_skip_on_ctrl = False
	
	if config.fadeout_audio is None:
		config.fadeout_audio = 0
	
	for std_mixer in std_mixers:
		if config[std_mixer + '_volume'] is None:
			config[std_mixer + '_volume'] = 1
		renpy.music.set_mixer_volume(config[std_mixer + '_volume'], std_mixer)
