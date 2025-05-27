init -100 python:
	def preferences__check_elem(elem):
		l = len(elem)
		if l < 2: return False
		
		obj = elem[0]
		if obj == 'str' and l > 6: return False
		if obj == 'bool' and l != 4: return False
		if obj == 'btn' and l not in (4, 5): return False
		if obj == 'bar' and l != 7: return False
		return obj in ('str', 'bool', 'btn', 'bar')
	
	def preferences__get_autosave_str():
		if config.autosave > 0:
			return '%s %s' % (config.autosave / 60, _('minutes'))
		return _('Disabled')
	def preferences__prev_autosave_time():
		autosave = config.autosave / 60
		if autosave <= 0:
			config.autosave = int(preferences.autosave_times[-2] * 60)
		else:
			i = 0
			while i < len(preferences.autosave_times) - 1 and autosave > preferences.autosave_times[i]:
				i += 1
			config.autosave = int(preferences.autosave_times[max(i - 1, 0)] * 60)
	def preferences__next_autosave_time():
		autosave = config.autosave / 60
		if autosave > 0:
			i = 0
			while i < len(preferences.autosave_times) - 1 and autosave > preferences.autosave_times[i]:
				i += 1
			config.autosave = int(preferences.autosave_times[i + 1] * 60)
	
	
	def preferences__add_text_cps(d):
		show_all_text = config.text_cps > 100000
		text_cps = in_bounds((config.text_cps % 100000) + d, 20, 220)
		config.text_cps = (100000 if show_all_text else 0) + text_cps
	def preferences__get_text_cps():
		return config.text_cps % 100000
	
	def preferences__get_text_cps_on():
		return config.text_cps > 100000
	def preferences__toggle_text_cps_on():
		v = not preferences.get_text_cps_on()
		config.text_cps = (100000 if v else 0) + (config.text_cps % 100000)
	
	
	def preferences__get_resolution_buttons():
		btns = []
		for resolution in preferences.resolutions:
			btns.append(['btn', '%sx%s' % resolution, (get_stage_size, resolution), Function(set_stage_size, *resolution), (4, 1)])
		
		res = []
		last_line = None
		for i, btn in enumerate(btns):
			if (i % 3) == 0:
				last_line = []
				res.append(last_line)
			last_line.append(btn)
		return res
	
	def preferences__get_mixer_bars():
		res = []
		for i in range(len(std_mixers)):
			mixer, mixer_name = std_mixers[i], std_mixers_names[i]
			
			mixer_text = ['str', '["%s"!t]:' % mixer_name, 1, 0.18]
			mixer_bar = ['bar', None, 'config.' + mixer + '_volume', 0, 1, Function(renpy.music.add_mixer_volume, -0.1, mixer), Function(renpy.music.add_mixer_volume, +0.1, mixer)]
			res.append([mixer_text, mixer_bar])
		return res
	
	
	build_object('preferences')
	
	
	k = get_from_hard_config('window_w_div_h', float)
	preferences.resolutions = tuple((i, int(i/k)) for i in (640, 960, 1200, 1366, 1920))
	del k
	
	preferences.langs = renpy.known_languages()
	
	preferences.show_mods = False
	
	preferences.autosave_times = (0.5, 1, 2, 3, 5, 7, 10, 15, 0)
	
	preferences.tabs = ['Screen', 'Sounds', 'Text', 'Other', 'Language']
	preferences.tab = preferences.tabs[0]
	
	
	preferences.content = {}
	# Elem of preferences.content['Your menu name'] -> hbox form list
	# Elems from the list:
	#  None
	#  ['str',  'Your text', text_size = 1, xsize = -1, text_align = 'left']   ## text_size - k for style.menu_text.text_size
	#  ['bool', 'Your text', function_for_get, function_for_set]
	#  ['btn',  'Your text', (function_for_get, result) - mb None, function_for_set, size = (5, 1)]   ## size - (w_div_h, k for style.[prefs_]menu_button.ysize)
	#  ['bar',  'Your text', obj (None for global), 'prop' (mb func), min_value, max_value, function_for_minus, function_for_plus]
	# You can use function (that returns str) instead of 'Your text'
	# You can use function (that returns items) instead of list of item params (see preferences__get_resolution_buttons)
	# Tags:
	#  "start [your code] end" -> "start " + str(eval(your code)) + " end"
	#  Simplest example:
	#   var_name = 123
	#    "[var_name]" -> "123"
	#  !t: "['text to translate'!t]:" -> _('text to translate') + ":"
	#  !i:
	#   who = "Name"
	#   welcome = "Hello, [who]!"
	#    "[welcome!i]" -> "Hello, Name!"
	#   translate russian strings:
	#    old "Hello, [who]!"
	#    new "Привет, [who!t]!"
	#    old "Name"
	#    new "Имя"
	#     "[welcome!ti]" -> "Привет, Имя!"
	#  !u for 'qWe' -> 'QWE' (str.upper)
	#  !l for 'qWe' -> 'qwe' (str.lower)
	#  !c for 'qWe' -> 'Qwe' (str.title, c - capitalize)
	#  !q for '{' -> '{{' (escaping text tags)
	#  [[ for escaping this tags ("[[var_name!t]" -> "[var_name!t]", "symbol [[" -> "symbol [")
	
	
	preferences.content['Screen'] = [
		[['str', '["Resolution"!t]:']],
		preferences.get_resolution_buttons,
		None,
		[['bool', '["Fullscreen"!t] [hotkeys.get_key_for(toggle_fullscreen)]', Function(get_from_hard_config, 'window_fullscreen', bool), toggle_fullscreen]],
	]
	
	preferences.content['Sounds'] = [
		[['str', '["Volume"!t]']],
		preferences.get_mixer_bars,
	]
	
	preferences.content['Text'] = [
		[['bool', '["Show all text at once"!t]', preferences.get_text_cps_on, preferences.toggle_text_cps_on]],
		None,
		[['str', '["Text display speed"!t]', 0.9]],
		[['bar', preferences, 'get_text_cps', 20, 220, Function(preferences.add_text_cps, -20), Function(preferences.add_text_cps, +20)]],
		None,
		[['bool', '["Skip after choices"!t]',            GetSetAttr('config.skip_after_choices'),        ToggleVariable('config.skip_after_choices')]],
		[['bool', '["Pause before skipping on Ctrl"!t]', GetSetAttr('config.pause_before_skip_on_ctrl'), ToggleVariable('config.pause_before_skip_on_ctrl')]],
	]
	
	preferences.content['Other'] = [
		[['str', '["Autosave"!t]', 0.9]],
		[
			['btn', gui.back_button_text, None, preferences.prev_autosave_time, (1, 1)],
			['str', preferences.get_autosave_str, 1, 0.15, 'center'],
			['btn', gui.next_button_text, None, preferences.next_autosave_time, (1, 1)],
		],
		None,
		[['bool', '["Show FPS"!t] [hotkeys.get_key_for(debug_screen.next_visible_mode)]', GetSetAttr('config.debug_screen_visible_mode'), debug_screen.toggle_fps]],
	]
	
	preferences.content['Language'] = []
	for lang in preferences.langs:
		preferences.content['Language'].append(
			[['btn', lang, None, Function(renpy.change_language, lang)]]
		)
	del lang

init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('preferences')


screen preferences:
	zorder 10001
	modal  True
	save   False
	
	image (gui.bg('prefs_bg') or gui.bg('main_bg')):
		size 1.0
	
	text _('Preferences'):
		style style.prefs_menu_title or style.menu_title
	
	
	if preferences.show_mods:
		use mods
	else:
		vbox:
			style style.prefs_pages_vbox or style.pages_vbox
			
			$ tmp_style = style.prefs_page_button or style.page_button
			for tab in preferences.tabs:
				if tab == 'Language' and len(preferences.langs) == 1:
					continue
				
				textbutton (tab if tab == 'Language' else _(tab)): # no translation for tab <Language>
					style tmp_style
					selected preferences.tab == tab
					action 'preferences.tab = tab'
		
		vbox:
			style 'prefs_content'
			
			python:
				orig_lines = preferences.content[preferences.tab]
				lines = []
				for elems in orig_lines:
					if callable(elems):
						lines.extend(elems())
					else:
						lines.append(elems)
			
			for elems in lines:
				if not elems:
					null ysize 1
				else:
					hbox:
						style 'prefs_line'
						
						for elem in elems:
							if callable(elem):
								$ elem = elem()
							if not preferences.check_elem(elem):
								$ out_msg('Screen <preferences>', 'Failed preferences.check_elem(elem)\nFor <%s>\nIn tab <%s>' % (elem, preferences.tab))
								continue
							
							python:
								obj = elem[0]
								if obj != 'bar':
									text = elem[1]
									if callable(text):
										text = text()
									text = interpolate_tags(str(text))
							
							if obj == 'str':
								python:
									tmp_style = style.menu_text
									
									text_size = tmp_style.get_current('text_size')
									if len(elem) > 2 and elem[2] > 0:
										text_size *= elem[2]
									text_xsize = -1 if len(elem) <= 3 or elem[3] <= 0 else get_absolute(elem[3], get_stage_width())
								
								text text:
									style tmp_style
									yalign 0.5
									text_size  text_size
									xsize      text_xsize
									text_align tmp_style.text_align if len(elem) <= 4 else elem[4]
							
							elif obj == 'bool':
								python:
									value = elem[2]()
									checkbox_image = gui['checkbox_yes' if value else 'checkbox_no']
								
								button:
									style 'bool_button'
									yalign 0.5
									action elem[3]
									
									hbox:
										style 'bool_hbox'
										
										image checkbox_image:
											style 'checkbox'
											yalign 0.5
										
										text text:
											style 'bool_text'
											yalign 0.5
							
							elif obj == 'btn':
								python:
									tmp_style = style.prefs_menu_button or style.menu_button
									
									if elem[2]:
										get, result_for_select = elem[2]
										selected_btn = get() == result_for_select
									else:
										selected_btn = False
									
									btn_height = tmp_style.get_current('ysize')
									if len(elem) > 4:
										w_div_h, kh = elem[4]
									else:
										w_div_h, kh = gui.prefs_std_btn_params
									btn_height = round(btn_height * kh)
									btn_width  = round(btn_height * w_div_h)
								
								textbutton text:
									style tmp_style
									yalign 0.5
									xsize btn_width
									ysize btn_height
									selected selected_btn
									action elem[3]
							
							elif obj == 'bar':
								python:
									obj, prop, min_value, max_value, function_for_minus, function_for_plus = elem[1:]
									value = getset_attr(prop, obj = obj)
									if callable(value):
										value = value()
									part = in_bounds((value - min_value) / (max_value - min_value), 0, 1)
								
								for i in (-1, 0, 1): # btn <minus>, bar, btn <plus>
									if i:
										textbutton gui['bar_minus_text' if i == -1 else 'bar_plus_text']:
											style 'bar_button'
											yalign 0.5
											action (function_for_minus if i == -1 else function_for_plus)
									else:
										image im.bar(part):
											style 'bar'
											yalign 0.5
	
	textbutton _('Preferences' if preferences.show_mods else 'Mods'):
		style 'mods_button'
		action ToggleVariable('preferences.show_mods')
	
	textbutton _('Return'):
		style 'return_button'
		action hide_screen('preferences')
	
	key 'ESCAPE' action hide_screen('preferences')
