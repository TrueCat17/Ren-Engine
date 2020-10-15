init -999 python:
	
	pause_end = 0
	def pause(sec = None):
		global pause_end
		pause_end = time.time() + (1e9 if sec is None else sec)
	
	def pause_ended():
		return pause_end < time.time()
	can_exec_next_check_funcs.append(pause_ended)
	
	def pause_to_end():
		pause(0)
	can_exec_next_skip_funcs.append(pause_to_end)
	
	class Music:
		@staticmethod
		def register_channel(name, mixer, loop):
			file_name, num_line = get_file_and_line(1)
			_register_channel(name, mixer, loop, file_name, num_line)
		@staticmethod
		def has_channel(name):
			return _has_channel(name)
		
		@staticmethod
		def play(music_urls, channel, depth = 0, **kwargs):
			fadein = kwargs.get('fadein', 0)
			music_url = music_urls if isinstance(music_urls, str) else music_urls[0]
			file_name, num_line = get_file_and_line(depth + 1)
			_play(channel + ' "' + music_url + '" fadein ' + str(float(fadein)), file_name, num_line)
		@staticmethod
		def stop(channel, depth = 0, **kwargs):
			fadeout = kwargs.get('fadeout', 0)
			file_name, num_line = get_file_and_line(depth + 1)
			_stop(channel + ' fadeout ' + str(float(fadeout)), file_name, num_line)
		
		@staticmethod
		def set_volume(vol, channel, depth = 0):
			file_name, num_line = get_file_and_line(depth + 1)
			_set_volume(in_bounds(vol, 0, 1), channel, file_name, num_line)
		@staticmethod
		def set_mixer_volume(vol, mixer, depth = 0):
			vol = in_bounds(round(vol, 2), 0.0, 1.0)
			config[mixer + '_volume'] = vol
			file_name, num_line = get_file_and_line(depth + 1)
			_set_mixer_volume(vol, mixer, file_name, num_line)
		@staticmethod
		def add_mixer_volume(d, mixer):
			Music.set_mixer_volume(config[mixer + '_volume'] + d, mixer)
	
	
	class Easy:
		@staticmethod
		def color(c):
			if isinstance(c, tuple) or isinstance(c, list):
				if len(c) == 4:
					r, g, b, a = c
				else:
					a = 255
					if len(c) == 3:
						r, g, b = c
					else:
						r = g = b = 0
						out_msg('Unexpected size of list (expected: 3 or 4, got: ' + str(len(c)) + ', c = ' + str(c) + ')')
			
			elif isinstance(c, basestring):
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
					out_msg('renpy.ease.color', 'Unexpected size of str (expected: 3, 4, 6 or 8, got: ' + str(len(c)) + ', c = "' + c + '")')
			else:
				r = g = b = 0
				a = 255
				out_msg('renpy.ease.color', 'Unexpected argument type (expected: list, tuple or basestring, got: ' + str(type(c)) + ')')
			
			return r, g, b, a
	
	
	class Renpy:
		config = persistent.config
		
		music = Music
		easy = Easy
		
		# prop = module
		random = random
		
		# prop = type
		absolute = absolute
		
		
		@staticmethod
		def pause(sec = None):
			pause(sec)
		
		@staticmethod
		def say(who, what):
			if who is None:
				who = narrator
			
			if isinstance(who, str):
				g = globals()
				if g.has_key(who):
					who = g[who]
				else:
					out_msg('renpy.say', 'Character <' + who + '> not found')
					tmp_character.name = who
					who = tmp_character
			
			if callable(who):
				who(what)
			else:
				out_msg('renpy.say', str(who) + ' is not callable')
				narrator(what)
		
		@staticmethod
		def play(file_name, channel, **kwargs):
			Renpy.music.play(file_name, channel, 1, **kwargs)
		@staticmethod
		def stop(channel, **kwargs):
			Renpy.music.stop(channel, 1, **kwargs)
		
		@staticmethod
		def has_label(label):
			return _has_label(label)
		@staticmethod
		def jump(label):
			_jump_next(label, False)
		@staticmethod
		def call(label):
			_jump_next(label, True)
		
		@staticmethod
		def show_screen(name):
			show_screen(name)
		@staticmethod
		def hide_screen(name):
			hide_screen(name)
		@staticmethod
		def has_screen(name):
			return has_screen(name)
		
		@staticmethod
		def call_screen(screen_name, ret_name, **kwargs):
			global call_screen_choosed, call_screen_name, call_ret_name
			
			call_screen_choosed = False
			call_screen_name, call_ret_name = screen_name, ret_name
			
			show_screen(screen_name)
		
		@staticmethod
		def seen_image(image):
			return persistent._seen_images.has_key(image)
		
		@staticmethod
		def seen_audio(file_name):
			return persistent._seen_audio.has_key(file_name)
		
		@staticmethod
		def seen_label(label):
			return persistent._seen_labels[get_current_mod()].has_key(label)
		@staticmethod
		def get_all_labels():
			return _get_all_labels()
		
		@staticmethod
		def change_language(lang):
			config.language = lang
			_set_lang(str(lang))
		@staticmethod
		def known_languages():
			return _known_languages()
	
	
	
	renpy = Renpy

