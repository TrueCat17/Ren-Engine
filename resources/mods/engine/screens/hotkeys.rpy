screen hotkeys:
	if not has_screen('console'):
		key 'p' action make_screenshot
		key 'F11' action set_fullscreen(not get_from_hard_config('window_fullscreen', bool))
		
		$ hotkeys_shift = False
		key 'LEFT SHIFT'  action SetVariable('hotkeys_shift', True) first_delay 0
		key 'RIGHT SHIFT' action SetVariable('hotkeys_shift', True) first_delay 0
		if hotkeys_shift:
			key 'o' action show_console
		
		if not has_screen('prev_text') and not db_hide_interface:
			key 'ESCAPE' action show_pause
		
		key config.quick_load_key action quick_load
		key config.quick_save_key action quick_save

