init -9990 python:
	
	pause_end = 0
	def pause(sec = None):
		global pause_end
		pause_end = get_game_time() + (1e9 if sec is None else sec)
	
	def pause_ended():
		return pause_end <= get_game_time()
	can_exec_next_check_funcs.append(pause_ended)
	
	def pause_to_end():
		pause(0)
	can_exec_next_skip_funcs.append(pause_to_end)
	
	
	def renpy__music__register_channel(name, mixer, loop):
		file_name, num_line = get_file_and_line(1)
		_register_channel(name, mixer, loop, file_name, num_line)
	
	def renpy__music__get_audio_len(path):
		return _get_audio_len(path)
	
	def renpy__music__get_mixer_volume(mixer):
		return config.get(mixer + '_volume', None)
	def renpy__music__set_mixer_volume(vol, mixer, depth = 0):
		vol = in_bounds(round(vol, 2), 0.0, 1.0)
		config[mixer + '_volume'] = vol
		file_name, num_line = get_file_and_line(depth + 1)
		_set_mixer_volume(vol, mixer, file_name, num_line)
	def renpy__music__add_mixer_volume(d, mixer):
		renpy.music.set_mixer_volume(config[mixer + '_volume'] + d, mixer, depth = 1)
	
	def renpy__music__get_volume(channel, depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		return _get_volume_on_channel(channel, file_name, num_line)
	def renpy__music__set_volume(vol, channel, depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		_set_volume_on_channel(in_bounds(vol, 0, 1), channel, file_name, num_line)
	
	def renpy__music__get_pos(channel = 'music', depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		return _get_pos_on_channel(channel, file_name, num_line)
	def renpy__music__set_pos(sec, channel = 'music', depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		return _set_pos_on_channel(sec, channel, file_name, num_line)
	
	def renpy__music__get_pause(channel = 'music', depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		return _get_pause_on_channel(channel, file_name, num_line)
	def renpy__music__set_pause(value, channel = 'music', depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		return _set_pause_on_channel(value, channel, file_name, num_line)
	
	def renpy__music__play(paths, channel = 'music', depth = 0, **kwargs):
		if type(paths) is str:
			paths = [paths]
		
		fadein  = kwargs.get('fadein', 0)
		fadeout = kwargs.get('fadeout', config.fadeout_audio)
		relative_volume = kwargs.get('relative_volume', 1.0)
		file_name, num_line = get_file_and_line(depth + 1)
		
		for i, path in enumerate(paths):
			if i == 0:
				_play(channel, path, fadeout, fadein, relative_volume, file_name, num_line)
			else:
				_queue(channel, path, 0, relative_volume, file_name, num_line)
	
	def renpy__music__queue(paths, channel = 'music', depth = 0, **kwargs):
		if type(paths) is str:
			paths = [paths]
		
		fadein  = kwargs.get('fadein', 0)
		relative_volume = kwargs.get('relative_volume', 1.0)
		file_name, num_line = get_file_and_line(depth + 1)
		
		for i, path in enumerate(paths):
			_queue(channel, path, fadein if i == 0 else 0, relative_volume, file_name, num_line)
	
	def renpy__music__stop(channel = 'music', depth = 0, **kwargs):
		fadeout = kwargs.get('fadeout', config.fadeout_audio)
		file_name, num_line = get_file_and_line(depth + 1)
		_stop(channel, fadeout, file_name, num_line)
	
	def renpy__music__get_playing(channel = 'music', depth = 0):
		file_name, num_line = get_file_and_line(depth + 1)
		return _get_playing(channel, file_name, num_line)
	def renpy__music__is_playing(channel = 'music', depth = 0):
		path = renpy.music.get_playing(channel, depth + 1)
		return bool(path)
	
	
	def renpy__easy__color(c):
		t = type(c)
		
		if t is int:
			if c >= 0x01000000:
				r =  c >> 24
				g = (c >> 16) & 0xFF
				b = (c >>  8) & 0xFF
				a = c & 0xFF
			elif c >= 0:
				r =  c >> 16
				g = (c >> 8) & 0xFF
				b = c & 0xFF
				a = 255
			else:
				r = g = b = 0
				a = 255
				out_msg('renpy.easy.color', 'Expected non-negative int, got: %s' % c)
		
		elif t in (tuple, list):
			if len(c) == 4:
				r, g, b, a = c
			else:
				a = 255
				if len(c) == 3:
					r, g, b = c
				else:
					r = g = b = 0
					out_msg('renpy.easy.color', 'Expected %s with size 3 or 4, got: %s' % (t, c))
		
		elif t is str:
			if c[0] == '#':
				c = c[1:]
			elif c[0:2] == '0x':
				c = c[2:]
			
			if len(c) == 6:
				r = int(c[0]+c[1], 16)
				g = int(c[2]+c[3], 16)
				b = int(c[4]+c[5], 16)
				a = 255
			elif len(c) == 8:
				r = int(c[0]+c[1], 16)
				g = int(c[2]+c[3], 16)
				b = int(c[4]+c[5], 16)
				a = int(c[6]+c[7], 16)
			elif len(c) == 3:
				r = int(c[0], 16) * 0x11
				g = int(c[1], 16) * 0x11
				b = int(c[2], 16) * 0x11
				a = 255
			elif len(c) == 4:
				r = int(c[0], 16) * 0x11
				g = int(c[1], 16) * 0x11
				b = int(c[2], 16) * 0x11
				a = int(c[3], 16) * 0x11
			else:
				r = g = b = 0
				a = 255
				out_msg('renpy.easy.color', 'Expected str with size 3, 4, 6 or 8, got: "%s"' % c)
		else:
			r = g = b = 0
			a = 255
			out_msg('renpy.easy.color', 'Expected int, list, tuple or str, got: %s' % t)
		
		return r, g, b, a
	
	
	
	def renpy__say(who, what):
		if who is None:
			who = narrator
		
		if type(who) is str:
			g = globals()
			if who in g:
				who = g[who]
			else:
				out_msg('renpy.say', 'Character <%s> not found' % who)
				tmp_character.name = who
				who = tmp_character
		
		if callable(who):
			who(what)
		else:
			out_msg('renpy.say', '%s is not callable' % (who, ))
			narrator(what)
	
	
	def renpy__has_label(label):
		return (type(label) is str) and _has_label(label)
	def renpy__jump(label):
		if renpy.has_label(label):
			file_name, num_line = get_file_and_line(1)
			_jump_next(label, False, file_name, num_line)
		else:
			out_msg('renpy.jump', 'Label <%s> not found' % (label, ))
	def renpy__call(label):
		if renpy.has_label(label):
			file_name, num_line = get_file_and_line(1)
			_jump_next(label, True, file_name, num_line)
		else:
			out_msg('renpy.call', 'Label <%s> not found' % (label, ))
	
	
	def renpy__call_screen(screen_name, ret_name, **kwargs):
		global call_screen_ready, call_screen_name, call_ret_name
		
		call_screen_ready = False
		call_screen_name, call_ret_name = screen_name, ret_name
		
		show_screen(screen_name)
	
	
	def renpy__seen_image(image):
		return image in persistent._seen_images
	
	def renpy__seen_audio(file_name):
		return file_name in persistent._seen_audio
	
	def renpy__seen_label(label):
		return label in persistent._seen_labels[get_current_mod()]
	
	def renpy__change_language(lang):
		if type(lang) is not str:
			out_msg('renpy.change_language', 'Type of <lang> is not str')
			return
		config.language = lang
		_set_lang(lang)
		signals.send('language')
	
	def renpy__input_save(res):
		renpy.input_res = res
	def renpy__input(var_name, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = None):
		if reset_btn is None:
			reset_btn = default != ''
		renpy.input_res = None
		renpy.input_var_name = var_name
		input.ask_str(renpy.input_save, prompt, default, allow, exclude, length, mask, reset_btn = reset_btn, cancel_btn = False)
		renpy.call('input_waiting')
	
	
	def renpy__has_image(image_name):
		return image_was_registered(image_name)
	
	def renpy__showing(image_name):
		tag = sprites.get_tag_of_image_name(image_name)
		for spr in sprites.list:
			if spr.tag == tag:
				return True
		return False
	
	def renpy__scene():
		sprites.set_scene((), ())
	
	def renpy__show(image_name, tag = None, behind = None, at = None, effect = None, is_scene = False):
		sprites.show_impl(image_name, tag, behind, at, effect, is_scene)
	
	def renpy__hide(image_name, effect = None):
		tag = sprites.get_tag_of_image_name(image_name)
		sprites.hide_impl(tag, effect)
	
	build_object('renpy')
	
	
	renpy.config   = persistent.config
	renpy.Keymap   = dict
	renpy.random   = random   # prop = module
	renpy.absolute = absolute # prop = type
	
	renpy.pause = pause
	
	renpy.music.has_channel = _has_channel
	renpy.play  = renpy.music.play
	renpy.queue = renpy.music.queue
	renpy.stop  = renpy.music.stop
	
	renpy.show_screen = show_screen
	renpy.hide_screen = hide_screen
	renpy.has_screen = has_screen
	
	renpy.get_all_labels = _get_all_labels
	renpy.known_languages = _known_languages
	
	renpy.can_load = slots.can_load
	renpy.load = slots.load
	renpy.save = slots.save
	renpy.unlink_save = slots.delete
	renpy.list_slots = slots.list_slots
	renpy.slot_mtime = slots.mtime
	renpy.slot_screenshot = slots.screenshot


label input_waiting:
	while renpy.input_res is None:
		pause 0.1
	$ getset_attr(renpy.input_var_name, renpy.input_res)
