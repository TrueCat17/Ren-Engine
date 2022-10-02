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
			return str(config.autosave / 60.0) + ' ' + _('minutes')
		return _('Disabled')
	def preferences__prev_autosave_time():
		autosave = config.autosave / 60.0
		if autosave <= 0:
			config.autosave = int(preferences.autosave_times[-2] * 60)
		else:
			i = 0
			while i < len(preferences.autosave_times) - 1 and autosave > preferences.autosave_times[i]:
				i += 1
			config.autosave = int(preferences.autosave_times[max(i - 1, 0)] * 60)
	def preferences__next_autosave_time():
		autosave = config.autosave / 60.0
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
			btns.append(['btn', '%sx%s' % resolution, (get_stage_size, resolution), Function(set_stage_size, *resolution), (100, 25)])
		
		res = []
		last_line = None
		for i in xrange(len(btns)):
			if (i % 3) == 0:
				last_line = []
				res.append(last_line)
			last_line.append(btns[i])
		return res
	
	def preferences__get_mixer_bars():
		res = []
		for i in xrange(len(std_mixers)):
			mixer, mixer_name = std_mixers[i], std_mixers_names[i]
			
			mixer_text = ['str', '["%s"!t]:' % mixer_name, 25, 150]
			mixer_bar = ['bar', config, mixer + '_volume', 0, 1, Function(renpy.music.add_mixer_volume, -0.1, mixer), Function(renpy.music.add_mixer_volume, +0.1, mixer)]
			res.append([mixer_text, mixer_bar])
		return res
	
	
	build_object('preferences')
	
	
	k = get_from_hard_config("window_w_div_h", float)
	preferences.resolutions = tuple((i, int(i/k)) for i in (640, 960, 1200, 1366, 1920))
	
	preferences.langs = renpy.known_languages()
	
	preferences.show_mods = False
	
	preferences.autosave_times = (0.5, 1, 2, 3, 5, 7, 10, 15, 0)
	
	preferences.tabs = ['Screen', 'Sounds', 'Other', 'Language']
	preferences.tab = preferences.tabs[0]
	
	
	preferences.content = {}
	# Elem of preferences.content['Your menu name'] -> hbox form list
	# Elems from the list:
	#  None
	#  ['str', 'Your text', text_size = -1, xsize = -1, text_align = 'left']
	#  ['bool', 'Your text', function_for_get, function_for_set]
	#  ['btn', 'Your text', (function_for_get, result) - mb None, function_for_set, size = (100, 25)]
	#  ['bar', 'Your text', obj (None for global), 'prop' (mb func), min_value, max_value, function_for_minus, function_for_plus]
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
		[['bool', '["Fullscreen"!t] [hotkeys.get_key_for(toggle_fullscreen)]', Function(get_from_hard_config, 'window_fullscreen', bool), toggle_fullscreen]],
		None,
		[['str', '["Resolution"!t]:', 20]],
		preferences.get_resolution_buttons,
	]
	
	preferences.content['Sounds'] = [
		[['str', '["Volume"!t]']],
		preferences.get_mixer_bars,
	]
	
	preferences.content['Other'] = [
		[['bool', '["Show all text at once"!t]', preferences.get_text_cps_on, preferences.toggle_text_cps_on]],
		[['str', '["Text display speed"!t]', 20]],
		[['bar', preferences, 'get_text_cps', 20, 220, Function(preferences.add_text_cps, -20), Function(preferences.add_text_cps, +20)]],
		None,
		[['str', '["Autosave"!t]', 20]],
		[
			['btn', '<-', None, preferences.prev_autosave_time, 25],
			['str', preferences.get_autosave_str, 20, 130, 'center'],
			['btn', '->', None, preferences.next_autosave_time, 25],
		],
		None,
		[['bool', '["Show FPS"!t] [hotkeys.get_key_for(debug_screen.next_visible_mode)]', GetSetAttr('config.debug_screen_visible_mode'), debug_screen.toggle_fps]],
	]
	
	preferences.content['Language'] = []
	for lang in preferences.langs:
		preferences.content['Language'].append(
			[['btn', lang, None, Function(renpy.change_language, lang), (150, 25)]]
		)

init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('preferences')


screen preferences:
	zorder 10001
	modal  True
	
	image gui.bg('prefs_bg'):
		size 1.0
	
	text _('Preferences'):
		align (0.5, 0.02)
		
		font         gui.title_text_font
		color        gui.get_int('title_text_color')
		outlinecolor gui.get_int('title_text_outlinecolor')
		text_size gui.get_int('title_text_size')
	
	
	if preferences.show_mods:
		vbox:
			align 0.5
			spacing gui.get_int('page_spacing')
			
			for name, dir_name in get_mods():
				textbutton name:
					ground gui.bg('button_ground')
					hover  gui.bg('button_hover')
					xsize gui.get_int('button_width')
					ysize gui.get_int('button_height')
					font         gui.button_text_font
					color        gui.get_int('button_text_color')
					outlinecolor gui.get_int('button_text_outlinecolor')
					text_size   gui.get_int('button_text_size')
					text_align  gui.button_text_xalign
					
					action start_mod(dir_name)
	else:
		$ content_xindent = gui.get_int('prefs_xindent') * 2 + gui.get_int('page_button_width')
		vbox:
			xsize content_xindent
			yalign 0.5
			spacing gui.get_int('page_spacing')
			
			for tab in preferences.tabs:
				if tab == 'Language' and len(preferences.langs) == 1:
					continue
				
				textbutton (tab if tab == 'Language' else _(tab)): # no translation for tab <Language>
					font      gui.page_button_text_font
					text_size gui.get_int('page_button_text_size')
					color        gui.get_int('page_button_text_color')
					outlinecolor gui.get_int('page_button_text_outlinecolor')
					text_align gui.page_button_text_xalign
					
					xsize  gui.get_int('page_button_width')
					ysize  gui.get_int('page_button_height')
					xalign 0.5
					
					ground gui.bg('page_button_ground' if tab != preferences.tab else 'page_button_hover')
					hover  gui.bg('page_button_hover')
					
					action SetDict(preferences, 'tab', tab)
		
		vbox:
			xpos content_xindent
			xsize get_stage_width() - content_xindent
			yalign 0.5
			spacing 10
			
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
					null size 10
				else:
					hbox:
						xalign 0.5
						spacing 10
						
						for elem in elems:
							if callable(elem):
								$ elem = elem()
							if not preferences.check_elem(elem):
								$ out_msg('Screen <preferences>', 'Failed preferences.check_elem(elem)\nFor <' + str(elem) + '>\nIn tab <' + preferences.tab + '>')
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
									if len(elem) <= 2 or elem[2] <= 0:
										text_size = gui.get_int('interface_text_size')
									else:
										text_size = get_absolute(elem[2], get_stage_height())
								
								text text:
									yalign 0.5
									font         gui.interface_text_font
									color        gui.get_int('interface_text_color')
									outlinecolor gui.get_int('interface_text_outlinecolor')
									text_size  text_size
									xsize      -1 if len(elem) <= 3 or elem[3] <= 0 else elem[3]
									text_align gui.interface_text_xalign if len(elem) <= 4 else elem[4]
							
							elif obj == 'bool':
								python:
									value = elem[2]()
									image = gui.checkbox_yes if value else gui.checkbox_no
									text = '{image=' + image + '} ' + text
								textbutton text:
									font         gui.prefs_bool_text_font
									color        gui.get_int('prefs_bool_text_color')
									outlinecolor gui.get_int('prefs_bool_text_outlinecolor')
									text_size   gui.get_int('prefs_bool_text_size')
									text_align  gui.prefs_bool_text_xalign
									
									ground gui.bg('prefs_bool_ground')
									hover  gui.bg('prefs_bool_hover')
									
									yalign 0.5
									xsize  gui.get_int('prefs_bool_width')
									ysize  gui.get_int('prefs_bool_height')
									action elem[3]
							
							elif obj == 'btn':
								python:
									if elem[2]:
										get, result_for_select = elem[2]
										selected_btn = get() == result_for_select
									else:
										selected_btn = False
								textbutton text:
									ground gui.bg('button_hover' if selected_btn else 'button_ground')
									hover  gui.bg('button_hover')
									
									yalign 0.5
									size   (gui.get_int('button_width'), gui.get_int('button_height')) if len(elem) <= 4 else elem[4]
									action elem[3]
							
							elif obj == 'bar':
								python:
									obj, prop, min_value, max_value, function_for_minus, function_for_plus = elem[1:]
									value = getset_attr(prop, obj = obj)
									if callable(value):
										value = value()
									part = in_bounds((value - min_value) / float(max_value - min_value), 0, 1)
								
								textbutton '-':
									ground gui.bg('button_ground')
									hover  gui.bg('button_hover')
									size   gui.get_int('button_height')
									font         gui.button_text_font
									color        gui.get_int('button_text_color')
									outlinecolor gui.get_int('button_text_outlinecolor')
									text_size   gui.get_int('button_text_size')
									text_align  gui.button_text_xalign
									action function_for_minus
								image im.bar(part):
									xsize gui.get_int('prefs_bar_width')
									ysize gui.get_int('prefs_bar_height')
								textbutton '+':
									ground gui.bg('button_ground')
									hover  gui.bg('button_hover')
									size   gui.get_int('button_height')
									font         gui.button_text_font
									color        gui.get_int('button_text_color')
									outlinecolor gui.get_int('button_text_outlinecolor')
									text_size   gui.get_int('button_text_size')
									text_align  gui.button_text_xalign
									action function_for_plus
	
	
	textbutton _('Preferences' if preferences.show_mods else 'Mods'):
		ground gui.bg('button_ground')
		hover  gui.bg('button_hover')
		xsize gui.get_int('button_width')
		ysize gui.get_int('button_height')
		font         gui.button_text_font
		color        gui.get_int('button_text_color')
		outlinecolor gui.get_int('button_text_outlinecolor')
		text_size   gui.get_int('button_text_size')
		text_align  gui.button_text_xalign
		
		align (0.05, 0.95)
		action ToggleDict(preferences, 'show_mods')
	
	textbutton _('Return'):
		ground gui.bg('button_ground')
		hover  gui.bg('button_hover')
		xsize gui.get_int('button_width')
		ysize gui.get_int('button_height')
		font         gui.button_text_font
		color        gui.get_int('button_text_color')
		outlinecolor gui.get_int('button_text_outlinecolor')
		text_size   gui.get_int('button_text_size')
		text_align  gui.button_text_xalign
		
		align (0.95, 0.95)
		action HideMenu('preferences')
	
	key 'ESCAPE' action HideMenu('preferences')
