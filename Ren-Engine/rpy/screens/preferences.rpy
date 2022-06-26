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
	#  ['str', 'Your text', text_size = 25, xsize = -1, text_align = 'left']
	#  ['bool', 'Your text', function_for_get, function_for_set]
	#  ['btn', 'Your text', (function_for_get, result) - mb None, function_for_set, size = (100, 25)]
	#  ['bar', 'Your text', obj (None for global), 'prop' (mb func), min_value, max_value, function_for_minus, function_for_plus]
	# You can use function instead 'Your text'
	# Tags:
	#  "start [your code] end" -> "start " + str(eval(your code)) + " end"
	#  Simplest example:
	#   var_name = 123
	#    "[var_name]" -> "123"
	#  !t: "['text to translate'!t]:" -> _('text to translate')
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
		[['bool', '["Fullscreen"!t] (F11)', Function(get_from_hard_config, 'window_fullscreen', bool), toggle_fullscreen]],
		None,
		[['str', '["Resolution"!t]:', 20]],
		[
			['btn', '%sx%s' % preferences.resolutions[0], (get_stage_size, preferences.resolutions[0]), Function(set_stage_size, *preferences.resolutions[0]), (100, 25)],
			['btn', '%sx%s' % preferences.resolutions[1], (get_stage_size, preferences.resolutions[1]), Function(set_stage_size, *preferences.resolutions[1]), (100, 25)],
			['btn', '%sx%s' % preferences.resolutions[2], (get_stage_size, preferences.resolutions[2]), Function(set_stage_size, *preferences.resolutions[2]), (100, 25)],
		],
		[
			['btn', '%sx%s' % preferences.resolutions[3], (get_stage_size, preferences.resolutions[3]), Function(set_stage_size, *preferences.resolutions[3]), (100, 25)],
			['btn', '%sx%s' % preferences.resolutions[4], (get_stage_size, preferences.resolutions[4]), Function(set_stage_size, *preferences.resolutions[4]), (100, 25)],
		],
	]
	
	preferences.content['Sounds'] = [
		[['str', '["Volume"!t]']],
	]
	for i in xrange(len(std_mixers)):
		mixer, mixer_name = std_mixers[i], std_mixers_names[i]
		
		mixer_text = ['str', '["%s"!t]:' % mixer_name, 25, 150]
		mixer_bar = ['bar', config, mixer + '_volume', 0, 1, Function(renpy.music.add_mixer_volume, -0.1, mixer), Function(renpy.music.add_mixer_volume, +0.1, mixer)]
		preferences.content['Sounds'].append([mixer_text, mixer_bar])
	
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
		[['bool', '["Show FPS"!t] (F3)', Function(getattr, config, 'debug_screen_visible_mode'), debug_screen.toggle_fps]],
	]
	
	preferences.content['Language'] = []
	for lang in preferences.langs:
		preferences.content['Language'].append(
			[['btn', lang, None, Function(renpy.change_language, lang), (150, 25)]]
		)

init 1 python:
	preferences.background = gui + 'menu/main/back.png'
	preferences.text_size = 25
	preferences.color = 0x000000
	preferences.outlinecolor = None
	preferences.bool_ground = im.rect('#00000002')
	preferences.bool_hover  = im.rect('#00000010')
	preferences.btn_ground = style.textbutton.ground
	preferences.btn_hover  = style.textbutton.hover
	preferences.btn_size = style.textbutton.size


screen preferences:
	zorder 10001
	modal  True
	
	if not has_screen('pause') and not has_screen('choose_menu'):
		use hotkeys
	
	image preferences.background:
		size 1.0
	
	text _('Preferences'):
		align (0.5, 0.02)
		
		color 0xFFFFFF
		outlinecolor 0
		text_size get_stage_height() / 10
	
	
	if preferences.show_mods:
		vbox:
			align 0.5
			spacing 10
			
			for name, dir_name in get_mods():
				textbutton name:
					xalign 0.5
					action start_mod(dir_name)
	else:
		$ preferences.menu_size = 150
		$ preferences.menu_xpos = int(get_stage_width() * 0.05)
		vbox:
			anchor (0, 0.5)
			pos (preferences.menu_xpos, 0.5)
			spacing 10
			
			for tab in preferences.tabs:
				if tab != 'Language' or len(preferences.langs) > 1:
					textbutton (tab if tab == 'Language' else _(tab)): # no translation for tab <Language>
						xsize preferences.menu_size
						action SetDict(preferences, 'tab', tab)
		
		vbox:
			xpos preferences.menu_xpos + preferences.menu_size
			xsize get_stage_width() - (preferences.menu_xpos + preferences.menu_size)
			yalign 0.5
			spacing 10
			
			for elems in preferences.content[preferences.tab]:
				if not elems:
					null size 10
				else:
					hbox:
						xalign 0.5
						spacing 10
						
						for elem in elems:
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
								text text:
									yalign 0.5
									color        preferences.color
									outlinecolor preferences.outlinecolor
									text_size preferences.text_size if len(elem) <= 2 else elem[2]
									xsize          -1 if len(elem) <= 3 else elem[3]
									text_align 'left' if len(elem) <= 4 else elem[4]
							
							elif obj == 'bool':
								python:
									value = elem[2]()
									image = checkbox_yes_orig if value else checkbox_no_orig
									text = '{image=' + image + '} ' + text
								textbutton text:
									color        preferences.color
									outlinecolor preferences.outlinecolor
									
									ground preferences.bool_ground
									hover  preferences.bool_hover
									
									yalign 0.5
									xsize  min(350, int(get_stage_width() * 0.6))
									ysize  25
									text_size 20
									action elem[3]
							
							elif obj == 'btn':
								python:
									if elem[2]:
										get, result_for_select = elem[2]
										selected_btn = get() == result_for_select
									else:
										selected_btn = False
								textbutton text:
									ground preferences.btn_hover if selected_btn else preferences.btn_ground
									hover  preferences.btn_hover
									
									yalign 0.5
									size   preferences.btn_size if len(elem) <= 4 else elem[4]
									action elem[3]
							
							elif obj == 'bar':
								python:
									obj, prop, min_value, max_value, function_for_minus, function_for_plus = elem[1:]
									value = getset_attr(prop, obj = obj)
									if callable(value):
										value = value()
									part = in_bounds((value - min_value) / float(max_value - min_value), 0, 1)
								
								textbutton '-':
									ground preferences.btn_ground
									hover  preferences.btn_hover
									size   25
									action function_for_minus
								image im.bar(part):
									xsize min(300, int(get_stage_width() * 0.3))
									ysize 25
								textbutton '+':
									ground preferences.btn_ground
									hover  preferences.btn_hover
									size   25
									action function_for_plus
	
	
	textbutton _('Preferences' if preferences.show_mods else 'Mods'):
		align (0.05, 0.95)
		action ToggleDict(preferences, 'show_mods')
	
	textbutton _('Return'):
		align (0.95, 0.95)
		action HideMenu('preferences')
	key 'ESCAPE' action HideMenu('preferences')
