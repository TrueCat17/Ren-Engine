init -995 python:
	def debug_screen__get_last_fps():
		cur_time = get_game_time()
		
		array = debug_screen.fps_time_array
		array.append(cur_time)
		for i in range(len(array)):
			if cur_time - array[i] < 1:
				break
		if i:
			array[:i - 1] = []
		
		dtime = array[-1] - array[0]
		fps = len(array) / (dtime or 1)
		return str(min(round(fps), get_fps()))
	
	def debug_screen__clear_fps_time_array(screen):
		if screen == 'debug_screen':
			debug_screen.fps_time_array = []
	
	
	def debug_screen__toggle_fps(v = None):
		if v is None:
			if config.debug_screen_visible_mode:
				config.debug_screen_visible_mode = 0
			else:
				config.debug_screen_visible_mode = 3
		else:
			if type(v) is int:
				config.debug_screen_visible_mode = v % 4
			else:
				config.debug_screen_visible_mode = 3 if v else 0
	
	def debug_screen__next_visible_mode():
		config.debug_screen_visible_mode = (config.debug_screen_visible_mode + 1) % 4
	
	def debug_screen__get_show_mode():
		return config.debug_screen_show_mode
	def debug_screen__set_show_mode(mode):
		config.debug_screen_show_mode = mode
	
	def debug_screen__update():
		cur_time = get_game_time()
		arrays = debug_screen.screen_time_arrays
		
		screen_times = get_screen_times()
		
		for name in list(arrays.keys()):
			if name not in screen_times:
				del arrays[name]
		
		for name, time in screen_times.items():
			array = arrays.setdefault(name, [])
			array.append((cur_time, time * 1000))
			
			for i in range(len(array)):
				if cur_time - array[i][0] < 1:
					break
			array[:i] = []
		
		screen_tmp.text_align = 0.0 if config.debug_screen_align[0] < 0.5 else 1.0
		screen_tmp.mode_btn_xsize = (debug_screen.size[0] - debug_screen.indent * 2) // 3
	
	def debug_screen__get_screen_times():
		mode = debug_screen.get_show_mode()
		
		if mode == 'Last Frame':
			def get_time(array):
				return array[-1][1]
		
		elif mode == 'Mean (second)':
			def get_time(array):
				sum_value = 0.0
				for prev_time, time in array:
					sum_value += time
				return sum_value / len(array)
		
		elif mode == 'Max (second)':
			def get_time(array):
				max_value = 0.0
				for prev_time, time in array:
					max_value = max(max_value, time)
				return max_value
		
		else:
			out_msg('debug_screen.get_screen_times', 'Unexpected debug_screen show_mode: %s', mode)
			return []
		
		arrays = debug_screen.screen_time_arrays
		params = []
		max_name_len = 0
		for name in sorted(arrays.keys()):
			array = arrays[name]
			if array:
				params.append((name, get_time(array)))
				max_name_len = max(max_name_len, len(name))
		
		res = []
		for name, time in params:
			res.append('%s: %.1f ms' % (name.ljust(max_name_len), time))
		return res
	
	build_object('debug_screen')
	signals.add('show_screen', debug_screen.clear_fps_time_array)
	
	
	config.setdefault('debug_screen_visible_mode', 0)
	config.setdefault('debug_screen_show_mode', 'Mean (second)')
	config.setdefault('debug_screen_align', (0, 0))
	
	debug_screen.size = (400, 250)
	debug_screen.indent = 10
	debug_screen.ext_size = (debug_screen.size[0] + debug_screen.indent * 2, debug_screen.size[1] + debug_screen.indent * 2)
	debug_screen.bg = im.rect('#0005')
	
	debug_screen.align_btn_corner = im.rect('#F00')
	
	debug_screen.btn_ground = im.rect('#CCC')
	debug_screen.btn_ground_selected = im.rect('#0AF')
	debug_screen.btn_hover = im.rect('#F80')
	
	debug_screen.fps_time_array = []
	debug_screen.screen_time_arrays = {}


init -1000:
	style debug_screen_fps_text is text:
		font     'Alcdnova'
		text_size 24
		color        '#FF0'
		outlinecolor '#800'
	
	
	style debug_screen_mode_btn is textbutton:
		color '#FFF'
		outlinecolor 0
	
	style debug_screen_text is text:
		color '#FFF'
		outlinecolor 0
		font     'Consola'
		text_size 18
	
	
	style debug_screen_align_btn is button:
		size (40, 30)
		ground im.rect('#FFF')
		hover  im.rect('#FFF')
		alpha 0.5
	
	style debug_screen_align_img is debug_screen_align_btn:
		zoom 0.4
		alpha 1


screen debug_screen_fps_and_alignment:
	xsize debug_screen.size[0]
	ysize max(style.debug_screen_fps_text.get_current('text_size'), style.debug_screen_align_btn.get_current('ysize'))
	
	text debug_screen.get_last_fps():
		style 'debug_screen_fps_text'
		xalign screen_tmp.text_align
		yalign 0.5
	
	if config.debug_screen_visible_mode == 1:
		hbox:
			spacing debug_screen.indent
			xalign 1.0 - screen_tmp.text_align
			yalign 0.5
			
			for align in ((0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)):
				button:
					style 'debug_screen_align_btn'
					action 'config.debug_screen_align = align'
					
					image debug_screen.align_btn_corner:
						align align
						style 'debug_screen_align_img'

screen debug_screen_buttons:
	has hbox
	spacing debug_screen.indent
	xalign 0.5
	
	for mode in ('Mean (second)', 'Max (second)', 'Last Frame'):
		textbutton _(mode):
			style 'debug_screen_mode_btn'
			xsize screen_tmp.mode_btn_xsize
			
			ground debug_screen['btn_ground_selected' if debug_screen.get_show_mode() == mode else 'btn_ground']
			hover  debug_screen.btn_hover
			action debug_screen.set_show_mode(mode)

screen debug_screen_text:
	has vbox
	spacing debug_screen.indent // 2
	
	xsize debug_screen.size[0]
	
	for text in debug_screen.get_screen_times():
		text text:
			style 'debug_screen_text'
			align screen_tmp.text_align

screen debug_screen:
	zorder 1000000
	ignore_modal True
	
	# 0, 1, 2, 3 -> off, back+fps+text, fps+text, fps
	if config.debug_screen_visible_mode:
		$ screen_tmp = SimpleObject()
		$ debug_screen.update()
		
		null:
			align config.debug_screen_align
			size  debug_screen.ext_size
			
			image debug_screen.bg:
				size  debug_screen.ext_size
				alpha 1 if config.debug_screen_visible_mode == 1 else 0
			
			null:
				align 0.5
				size debug_screen.size
				
				vbox:
					spacing debug_screen.indent
					size    debug_screen.size
					align   config.debug_screen_align
					
					if config.debug_screen_align[1] < 0.5:
						use debug_screen_fps_and_alignment
						if config.debug_screen_visible_mode != 3:
							use debug_screen_buttons
							use debug_screen_text
					else:
						if config.debug_screen_visible_mode != 3:
							use debug_screen_text
							use debug_screen_buttons
						use debug_screen_fps_and_alignment
