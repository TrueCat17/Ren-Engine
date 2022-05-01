init python:
	def debug_screen__get_last_fps():
		cur_time = get_game_time()
		
		array = debug_screen.fps_time_array
		i = 0
		for i in xrange(len(array)):
			if cur_time - array[i] < 1:
				i -= 1
				break
		
		debug_screen.fps_time_array = array = array[i+1:]
		array.append(cur_time)
		
		dtime = array[-1] - array[0]
		fps = len(array) / (dtime or 1)
		return str(min(int(round(fps)), get_fps()))
	
	def debug_screen__clear_fps_time_array(screen):
		if screen == 'debug_screen':
			debug_screen.fps_time_array = []
	
	
	def debug_screen__toggle_fps():
		if config.debug_screen_visible_mode:
			config.debug_screen_visible_mode = 0
		else:
			config.debug_screen_visible_mode = 3
	
	def debug_screen__next_visible_mode():
		config.debug_screen_visible_mode = (config.debug_screen_visible_mode + 1) % 4
	
	def debug_screen__get_show_mode():
		return config.debug_screen_show_mode
	def debug_screen__set_show_mode(mode):
		config.debug_screen_show_mode = mode
	
	def debug_screen__update():
		cur_time = get_game_time()
		
		screen_times = get_screen_times()
		for name, time in screen_times.iteritems():
			if name not in debug_screen.screen_time_arrays:
				debug_screen.screen_time_arrays[name] = []
			debug_screen.screen_time_arrays[name].append((cur_time, time * 1000))
		
		for name, array in debug_screen.screen_time_arrays.iteritems():
			i = 0
			for i in xrange(len(array)):
				if cur_time - array[i][0] < 1:
					i -= 1
					break
			debug_screen.screen_time_arrays[name] = array[i+1:]
	
	def debug_screen__get_screen_times():
		res = []
		max_len = 0
		for name in debug_screen.screen_time_arrays:
			max_len = max(max_len, len_unicode(name))
		
		def with_spaces(name):
			return name + ' ' * (max_len - len_unicode(name))
		
		names = sorted(debug_screen.screen_time_arrays.keys())
		mode = debug_screen.get_show_mode()
		if mode == 'Last Frame':
			for name in names:
				array = debug_screen.screen_time_arrays[name]
				if not array: continue
				res.append([with_spaces(name), ': ', '%.2f' % array[-1][1], ' ms'])
		elif mode == 'Mean (second)':
			for name in names:
				array = debug_screen.screen_time_arrays[name]
				if not array: continue
				sum_value = 0.0
				for prev_time, time in array:
					sum_value += time
				res.append([with_spaces(name), ': ', '%.2f' % (sum_value / len(array)), ' ms'])
		elif mode == 'Max (second)':
			for name in names:
				array = debug_screen.screen_time_arrays[name]
				if not array: continue
				max_value = 0.0
				for prev_time, time in array:
					max_value = max(max_value, time)
				res.append([with_spaces(name), ': ', '%.2f' % max_value, ' ms'])
		else:
			out_msg('Unexpected debug_screen show_mode: ' + str(mode))
		
		return res
	
	build_object('debug_screen')
	signals.add('show_screen', debug_screen.clear_fps_time_array)
	
	
	if config.debug_screen_visible_mode is None:
		config.debug_screen_visible_mode = 0
	if config.debug_screen_show_mode is None:
		config.debug_screen_show_mode = 'Mean (second)'
	if config.debug_screen_align is None:
		config.debug_screen_align = (0, 0)
	
	debug_screen.size = (400, 250)
	debug_screen.indent = 10
	debug_screen.background_alpha = 0.3
	debug_screen.font = 'Consola'
	debug_screen.text_size = 18
	debug_screen.color = 0xFFFFFF
	debug_screen.outlinecolor = 0x000000
	
	debug_screen.fps_font = 'Alcdnova'
	debug_screen.fps_text_size = 24
	debug_screen.fps_color = 0xFFFF00
	debug_screen.fps_outlinecolor = 0x800000
	
	debug_screen.btn_ground = im.rect('#CCC')
	debug_screen.btn_ground_selected = im.rect('#0AF')
	debug_screen.btn_hover = im.rect('#F80')
	debug_screen.btn_xsize = 130
	
	debug_screen.fps_time_array = []
	debug_screen.screen_time_arrays = {}


screen debug_screen:
	zorder 1000000
	
	# 0, 1, 2, 3 -> off, back+fps+text, fps+text, fps
	if config.debug_screen_visible_mode:
		$ debug_screen.update()
		
		image im.rect('#000'):
			xsize debug_screen.size[0] + 2 * debug_screen.indent
			ysize debug_screen.size[1] + 2 * debug_screen.indent
			alpha debug_screen.background_alpha if config.debug_screen_visible_mode == 1 else 0
		
		null:
			align config.debug_screen_align
			pos   debug_screen.indent
			size  debug_screen.size
			
			vbox:
				spacing debug_screen.indent
				size    debug_screen.size
				
				null:
					xsize debug_screen.size[0] - 2 * debug_screen.indent
					
					text debug_screen.get_last_fps():
						font         debug_screen.fps_font
						text_size    debug_screen.fps_text_size
						color        debug_screen.fps_color
						outlinecolor debug_screen.fps_outlinecolor
				
				if config.debug_screen_visible_mode != 3:
					hbox:
						xalign  0.5
						spacing debug_screen.indent
						
						for mode in ('Mean (second)', 'Max (second)', 'Last Frame'):
							textbutton _(mode):
								ground       debug_screen['btn_ground_selected' if debug_screen.get_show_mode() == mode else 'btn_ground']
								hover        debug_screen.btn_hover
								color        debug_screen.color
								outlinecolor debug_screen.outlinecolor
								xsize        debug_screen.btn_xsize
								action       debug_screen.set_show_mode(mode)
					
					vbox:
						spacing debug_screen.indent / 2
						
						for text_parts in debug_screen.get_screen_times():
							hbox:
								for text in text_parts:
									text text:
										font         debug_screen.font
										text_size    debug_screen.text_size
										color        debug_screen.color
										outlinecolor debug_screen.outlinecolor

