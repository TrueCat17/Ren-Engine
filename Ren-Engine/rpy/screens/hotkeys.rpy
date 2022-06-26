init -901 python:
	def hotkeys__init(screen_name):
		hotkeys.prepared_keymap = {}
		hotkeys.keys_for_listen = set()
		for name, key_list in hotkeys.keymap.iteritems():
			for keys_str in key_list:
				keys_str = keys_str.replace('K_', '').upper()
				
				keys = keys_str.split('_')
				if len(keys) > 2 and hotkeys.only_one_mod_key:
					out_msg('hotkeys.init', 'Using more than 1 mod key (CTRL, SHIFT or ALT) in hotkey <%s>' % keys_str)
					continue
				
				ok = True
				for key in keys[:-1]:
					if key not in ('CTRL', 'SHIFT', 'ALT'):
						out_msg('hotkeys.init', 'First key <%s> in hotkey <%s> is not CTRL, SHIFT or ALT' % (keys[0], keys_str))
						ok = False
				if not ok:
					continue
				
				main_key = keys[-1]
				if main_key not in hotkeys.keys and main_key not in hotkeys.shift_to:
					out_msg('hotkeys.init', 'Unexpected key <%s> in hotkey <%s>' % (main_key, keys_str))
					continue
				
				if main_key in hotkeys.shift_to:
					index = hotkeys.shift_to.index(main_key)
					main_key = hotkeys.shift_from[index]
					if 'SHIFT' in keys:
						keys.remove('SHIFT')
				
				ctrl  = 'CTRL_'  if 'CTRL'  in keys else ''
				shift = 'SHIFT_' if 'SHIFT' in keys else ''
				alt   = 'ALT_'   if 'ALT'   in keys else ''
				keys_str = ctrl + shift + alt + keys[-1]
				
				hotkeys.prepared_keymap[keys_str] = name
				hotkeys.keys_for_listen.add(main_key)
	
	signals.add('show_screen', hotkeys__init, times=1)
	
	
	def hotkeys__pressed(key):
		add_shift = False
		if hotkeys.shift:
			if key in hotkeys.shift_from:
				index = hotkeys.shift_from.index(key)
				key = hotkeys.shift_to[index]
			else:
				add_shift = True
		elif key in hotkeys.keypad_synonyms:
			key = key[-1]
		
		ctrl  = 'CTRL_'  if hotkeys.ctrl else ''
		shift = 'SHIFT_' if add_shift    else ''
		alt   = 'ALT_'   if hotkeys.alt  else ''
		key = ctrl + shift + alt + key.upper()
		
		name = hotkeys.prepared_keymap.get(key, None)
		if name:
			for i in hotkeys.underlay:
				exec_funcs(i.get(name, None))
	
	
	build_object('hotkeys')
	# by default only 1 mod key (ctrl/shift/alt) available for no intersections with OS hotkeys
	hotkeys.only_one_mod_key = True
	hotkeys.ctrl = hotkeys.shift = hotkeys.alt = False
	hotkeys.underlay = []
	
	hotkeys.keys = alphabet
	for i in xrange(12):
		hotkeys.keys.append('F' + str(i + 1))
	hotkeys.keys.extend([
		'ESCAPE', 'TAB', 'RETURN', 'SPACE', 'MENU', 'BACKSPACE', 'DELETE', 'INSERT',
		'HOME', 'END', 'PAGEUP', 'PAGEDOWN',
		'LEFT', 'RIGHT', 'UP', 'DOWN',
	])
	
	hotkeys.keypad_synonyms = ['KEYPAD +', 'KEYPAD -', 'KEYPAD *', 'KEYPAD /', 'KEYPAD =']
	hotkeys.keys.extend(hotkeys.keypad_synonyms)
	
	hotkeys.shift_from = list("`1234567890-=,./;'[]\\")
	hotkeys.shift_to   = list('~!@#$%^&*()_+<>?:"{}|')
	hotkeys.keys.extend(hotkeys.shift_from)
	
	for i in xrange(len(hotkeys.keys)):
		hotkeys.keys[i] = hotkeys.keys[i].upper()


init -900 python:
	hotkeys.keymap = dict(
		screenshot = ['P'],
		console = ['shift_O'],
		
		help = ['F1'],
		toggle_fps_visible = ['F3'],
		toggle_fullscreen = ['F11'],
		
		quicksave = ['Q'],
		quickload = ['L'],
		
		pause = ['ESCAPE'],
	)
	
	hotkeys.underlay.append(renpy.Keymap(
		screenshot = make_screenshot,
		console = console.show,
		
		toggle_fps_visible = debug_screen.next_visible_mode,
		toggle_fullscreen = toggle_fullscreen,
		
		quicksave = QuickSave(),
		quickload = QuickLoad(),
		
		pause = pause_screen.show,
	))


screen hotkeys:
	zorder -1e9
	
	$ hotkeys.ctrl = hotkeys.shift = hotkeys.alt = False
	key 'LEFT CTRL'   action 'hotkeys.ctrl  = True' first_delay 0
	key 'RIGHT CTRL'  action 'hotkeys.ctrl  = True' first_delay 0
	key 'LEFT SHIFT'  action 'hotkeys.shift = True' first_delay 0
	key 'RIGHT SHIFT' action 'hotkeys.shift = True' first_delay 0
	key 'LEFT ALT'    action 'hotkeys.alt   = True' first_delay 0
	key 'RIGHT ALT'   action 'hotkeys.alt   = True' first_delay 0
	
	if hotkeys.ctrl + hotkeys.shift + hotkeys.alt < 2 or not hotkeys.only_one_mod_key:
		for key in hotkeys.keys_for_listen:
			key key action hotkeys.pressed(key)
