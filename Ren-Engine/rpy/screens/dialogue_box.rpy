init -1000 python:
	# db = dialogue box
	
	
	def db__make_step(text, start_index, max_count_symbols = -1):
		l = len(text)
		index = start_index
		symbols = 0
		tag = value = None
		
		while index < l and (max_count_symbols == -1 or symbols < max_count_symbols):
			while index < l and text[index].isspace():
				index += 1
			if index == l:
				break
			
			# maybe style start/end, for example: {size=25} or {/b}
			if text[index] == '{':
				
				# not style, '{{' render to '{'
				if index + 1 < l and text[index + 1] == '{':
					index += 2
					symbols += 1
				
				# style
				else:
					if symbols != 0:
						break # stop if was text on current step
					
					start_index = index
					while index < l and text[index] != '}':
						index += 1
					tag = text[start_index+1:index]
					assign = tag.find('=')
					if assign != -1:
						value = tag[assign+1:].strip()
						tag = tag[:assign]
					tag = tag.strip()
					
					index += 1
					break
			else:
				index += 1
				symbols += 1
		
		return index, symbols, tag, value
	
	
	def db__update_styles(local_styles = None):
		if local_styles is None:
			local_styles = db.local_styles or {}
		db.local_styles = local_styles
		
		for style in ('dialogue_box', 'dialogue_text', 'name_box', 'name_text'):
			props = []
			for prop in ('xpos', 'ypos', 'width', 'height'):
				props += [prop, prop + '_min', prop + '_max']
			
			if style.endswith('text'):
				props += ['font', 'size', 'size_min', 'size_max', 'color', 'outlinecolor', 'prefix', 'suffix']
			else:
				props += ['bg']
			
			for prop in props:
				prop = style + '_' + prop
				db[prop] = (local_styles if prop in local_styles else gui)[prop]
				if prop.endswith('color'):
					db[prop] = color_to_int(db[prop])
				elif prop.endswith('bg') and callable(db[prop]):
					db[prop] = db[prop]()
		
		for style in ('dialogue_button', 'dialogue_menu_button'):
			props = []
			for prop in ('width', 'height') + (('spacing', 'yalign') if style == 'dialogue_button' else ('xpos', 'ypos', 'xanchor', 'yanchor')):
				props += [prop, prop + '_min', prop + '_max']
			
			for prop in props:
				prop = style + '_' + prop
				db[prop] = (local_styles if prop in local_styles else gui)[prop]
		
		for style in ('prev', 'next', 'menu_button'):
			for prop in ('ground', 'hover'):
				prop = 'dialogue_' + style + '_' + prop
				value = (local_styles if prop in local_styles else gui)[prop]
				if callable(value):
					value = value()
				db[prop] = value
	
	def db__show_text(name, text, local_styles):
		if name is None:
			if db.local_styles is None:
				name = ''
				local_styles = narrator
				out_msg('db.show_text', "Don't use <extend> character before usual character to say text")
			else:
				local_styles = db.local_styles
		
		db.update_styles(local_styles)
		
		db.pause_after_text = 0
		db.pause_end = 0
		db.dialogue_text_after_pause = ''
		
		prev_index = None
		index = 0
		while prev_index != index:
			prev_index = index
			index, symbols, tag, value = db.make_step(text, index)
			
			if tag == 'w':
				db.pause_after_text = 1e9
				if value:
					try:
						db.pause_after_text = float(value)
					except:
						out_msg('db.show_text', 'Pause time <' + value + '> is not float')
				
				db.dialogue_text_after_pause = text[index:]
				text = text[:prev_index]
				break
		
		db.read = False
		
		# new text
		if name is not None:
			db.start_time = get_game_time()
			
			db.name_text = db.name_text_prefix + name + db.name_text_suffix
			
			db.dialogue_text = ''
			db.dialogue_full_text = db.dialogue_text_prefix + text
			if not db.dialogue_text_after_pause:
				db.dialogue_full_text += db.dialogue_text_suffix
			db.last_dialogue_text_suffix = db.dialogue_text_suffix
		
		# continuation of prev text (for <extend> character)
		else:
			db.start_time = get_game_time() - len(db.dialogue_text) / config.text_cps
			
			if db.last_dialogue_text_suffix and db.dialogue_full_text.endswith(db.last_dialogue_text_suffix):
				db.dialogue_full_text = db.dialogue_full_text[:-len(db.last_dialogue_text_suffix)]
			db.dialogue_full_text += text
			if not db.dialogue_text_after_pause:
				db.dialogue_full_text += db.last_dialogue_text_suffix
		
		if name is not None or db.dialogue_text_after_pause or not db.prev_texts:
			text_object = [
				db.name_text, db.name_text_font,     db.name_text_color,     db.name_text_outlinecolor,
				text,         db.dialogue_text_font, db.dialogue_text_color, db.dialogue_text_outlinecolor
			]
			db.prev_texts.append(text_object)
			db.prev_texts = db.prev_texts[-config.history_length:]
		else:
			db.prev_texts[-1][4] += text
		
		window_show()
	
	
	def db__recalc_props():
		if db.local_styles:
			db._dialogue_button_spacing = gui.get_int('dialogue_button_spacing', obj = db)
			db._dialogue_button_width = gui.get_int('dialogue_button_width', obj = db)
			db._dialogue_button_height = gui.get_int('dialogue_button_height', obj = db)
			if db._dialogue_button_width <= 0 or db._dialogue_button_height <= 0:
				db._dialogue_button_width = db._dialogue_button_height = db._dialogue_button_spacing = 0
			
			db._dialogue_menu_button_width  = gui.get_int('dialogue_menu_button_width', obj = db)
			db._dialogue_menu_button_height = gui.get_int('dialogue_menu_button_height', obj = db)
			if db._dialogue_menu_button_width <= 0 or db._dialogue_menu_button_height <= 0:
				db._dialogue_menu_button_width = db._dialogue_menu_button_height = 0
			
			if db.dialogue_box_width:
				db._dialogue_box_width = gui.get_int('dialogue_box_width', obj = db)
			else:
				db._dialogue_box_width = get_stage_width() - 4 * db._dialogue_button_spacing - 2 * db._dialogue_button_width # spc btn spc dialogue spc btn spc
			db._dialogue_box_height = gui.get_int('dialogue_box_height', obj = db)
			
			db._dialogue_text_xpos = gui.get_int('dialogue_text_xpos', obj = db)
			db._dialogue_text_ypos = gui.get_int('dialogue_text_ypos', obj = db)
			
			db._dialogue_text_width = gui.dialogue_text_width
			if not db._dialogue_text_width:
				db._dialogue_text_width = db._dialogue_box_width - 2 * db._dialogue_text_xpos
			db._dialogue_text_height = gui.dialogue_text_height
			if not db._dialogue_text_height:
				db._dialogue_text_height = db._dialogue_box_height - 2 * db._dialogue_text_ypos
			
			name_box_width_is_auto = db.name_box_width is None
			if name_box_width_is_auto:
				db.name_box_width = get_text_width(db.name_text_prefix + db.name_text + db.name_text_suffix, gui.get_int('name_text_size', obj = db)) + 20
			db._name_box_width = gui.get_int('name_box_width', obj = db)
			if name_box_width_is_auto:
				db.name_box_width = None
		
		
		if db.dialogue_text != db.dialogue_full_text:
			symbols_to_render = int((get_game_time() - db.start_time) * config.text_cps)
			
			prev_index = None
			index = 0
			while symbols_to_render and prev_index != index:
				prev_index = index
				index, symbols, tag, value = db.make_step(db.dialogue_full_text, index, symbols_to_render)
				symbols_to_render -= symbols
			
			if index < len(db.dialogue_full_text):
				db.dialogue_text = db.dialogue_full_text[0:index] + '{invisible}' + db.dialogue_full_text[index:]
			else:
				db.dialogue_text = db.dialogue_full_text
		else:
			if db.pause_after_text != 0:
				if db.pause_end == 0:
					db.pause_end = get_game_time() + db.pause_after_text
				elif db.pause_end < get_game_time():
					db.pause_after_text = 0
					db.pause_end = 0
					db.show_text(None, db.dialogue_text_after_pause, db.local_styles)
	
	
	def db__enter_action():
		if db.hide_interface:
			db.hide_interface = False
		else:
			db.to_next = True
	
	def db__on_enter():
		skip_exec_current_command()
		
		if db.pause_end > get_game_time():
			db.pause_end = get_game_time()
			return
		
		if db.dialogue_text == db.dialogue_full_text:
			if db.read:
				return
			db.read = True
			
			if db.mode == 'nvl':
				text_object = (
					db.name_text,     db.name_text_font,     db.name_text_color,     db.name_text_outlinecolor,
					db.dialogue_text, db.dialogue_text_font, db.dialogue_text_color, db.dialogue_text_outlinecolor
				)
				db.dialogue.append(text_object)
				db.name_text = db.dialogue_text = db.dialogue_full_text = ''
			else:
				db.dialogue = []
		else:
			db.dialogue_text = db.dialogue_full_text
	
	
	
	build_object('db')
	
	
	db.pause_after_text = 0
	db.pause_end = 0
	
	db.dialogue = []
	
	db.visible = False
	db.hide_interface = False
	db.skip_tab = False
	db.mode = 'adv'
	
	db.last_alt_time = -1
	db.last_shift_time = -1
	db.press_ctrl_time = -1
	db.ctrl = False
	db.no_skip_time_alt_shift = 0.5
	db.no_skip_time_ctrl = 0.33
	
	db.prev_texts = []
	
	
	db.read = True
	def db_read_func():
		return db.read
	can_exec_next_check_funcs.append(db_read_func)
	
	
	def db__disable_skipping_on_menu(screen_name):
		if screen_name == 'choice_menu':
			db.skip_tab = False
	signals.add('show_screen', db__disable_skipping_on_menu)
	
	
	
	window_show = SetDict(db, 'visible', True)
	window_hide = SetDict(db, 'visible', False)
	
	def nvl_clear():
		db.dialogue = []
	
	def set_mode_adv():
		db.mode = 'adv'
		nvl_clear()
	def set_mode_nvl():
		db.mode = 'nvl'
		nvl_clear()



screen dialogue_box_buttons(disable_next_btn = False):
	vbox:
		xalign gui.dialogue_box_xalign
		yalign gui.dialogue_box_yalign
		spacing gui.get_int('quick_buttons_top_indent')
		
		if db._dialogue_button_width and db.visible:
			hbox:
				spacing db._dialogue_button_spacing
				
				button:
					ground db.dialogue_prev_ground
					hover  style.get_default_hover(db.dialogue_prev_hover, db.dialogue_prev_ground)
					action    ShowScreen('history')
					alternate pause_screen.show
					
					yalign db.dialogue_button_yalign
					xsize db._dialogue_button_width
					ysize db._dialogue_button_height
				
				null size (db._dialogue_box_width, db._dialogue_box_height)
				
				if not screen.disable_next_btn:
					button:
						ground db.dialogue_next_ground
						hover  style.get_default_hover(db.dialogue_next_hover, db.dialogue_next_ground)
						action    db.on_enter
						alternate pause_screen.show
						
						yalign db.dialogue_button_yalign
						xsize db._dialogue_button_width
						ysize db._dialogue_button_height
				else:
					null:
						xsize db._dialogue_button_width
						ysize db._dialogue_button_height
		else:
			null:
				ysize db._dialogue_button_height or 0
		
		if quick_menu:
			use quick_menu
	
	if db._dialogue_menu_button_width:
		button:
			ground db.dialogue_menu_button_ground
			hover  style.get_default_hover(db.dialogue_menu_button_hover, db.dialogue_menu_button_ground)
			action    pause_screen.show
			alternate pause_screen.show
			
			xpos gui.dialogue_menu_button_xpos
			ypos gui.dialogue_menu_button_ypos
			xanchor gui.dialogue_menu_button_xanchor
			yanchor gui.dialogue_menu_button_yanchor
			xsize db._dialogue_menu_button_width
			ysize db._dialogue_menu_button_height


screen dialogue_box_adv:
	vbox:
		xalign gui.dialogue_box_xalign
		yalign gui.dialogue_box_yalign
		spacing gui.get_int('quick_buttons_top_indent')
		
		image db.dialogue_box_bg:
			xsize db._dialogue_box_width
			ysize db._dialogue_box_height
			
			image db.name_box_bg:
				xanchor gui.name_box_xanchor
				yanchor gui.name_box_yanchor
				xpos gui.get_int('name_box_xpos', obj = db)
				ypos gui.get_int('name_box_ypos', obj = db)
				xsize db._name_box_width
				ysize gui.get_int('name_box_height', obj = db)
				
				alpha 1 if db.name_text else 0
				
				text db.name_text:
					font         gui.name_text_font
					text_size    gui.get_int('name_text_size', obj = db)
					color        db.name_text_color
					outlinecolor db.name_text_outlinecolor
					xalign       gui.name_text_xalign
					yalign       gui.name_text_yalign
			
			text db.dialogue_text:
				xpos gui.get_int('dialogue_text_xpos', obj = db)
				ypos gui.get_int('dialogue_text_ypos', obj = db)
				xsize db._dialogue_text_width
				ysize db._dialogue_text_height
				
				font         db.dialogue_text_font
				text_size    gui.get_int('dialogue_text_size', obj = db)
				color        db.dialogue_text_color
				outlinecolor db.dialogue_text_outlinecolor
				text_align   gui.dialogue_text_align
		
		if quick_menu:
			null ysize style.quick_buttons_bg.get_current('ysize')


screen dialogue_box_nvl:
	image 'images/bg/black.jpg':
		size 1.0
		alpha 0.30
	
	vbox:
		ypos 0.05
		
		spacing 0 if gui.nvl_height else gui.get_int('nvl_spacing')
		
		python:
			db.last_dialogue = db.dialogue + [(
				db.name_text,     db.name_text_font,     db.name_text_color,     db.name_text_outlinecolor,
				db.dialogue_text, db.dialogue_text_font, db.dialogue_text_color, db.dialogue_text_outlinecolor
			)]
			_name_text_yoffset     = max(gui.get_int('dialogue_text_size') - gui.get_int('name_text_size'), 0)
			_dialogue_text_yoffset = max(gui.get_int('name_text_size') - gui.get_int('dialogue_text_size'), 0)
		
		for _name_text, _name_font, _name_color, _name_outlinecolor, _dialogue_text, _dialogue_font, _dialogue_color, _dialogue_outlinecolor in db.last_dialogue:
			null:
				ysize gui.get_int('nvl_height') if gui.nvl_height else -1
				
				if _name_text:
					text _name_text:
						xpos gui.get_int('nvl_name_xpos')
						ypos gui.get_int('nvl_name_ypos') + _name_text_yoffset
						xsize gui.get_int('nvl_name_width')
						xanchor    gui.nvl_name_xalign
						text_align gui.nvl_name_xalign
						
						font         _name_font
						text_size    gui.get_int('name_text_size')
						color        _name_color
						outlinecolor _name_outlinecolor
				
				$ nvl_text_prefix = 'nvl_' + ('text' if _name_text else 'thought') + '_'
				text _dialogue_text:
					xpos gui.get_int(nvl_text_prefix + 'xpos')
					ypos gui.get_int(nvl_text_prefix + 'ypos') + (_dialogue_text_yoffset if _name_text else 0)
					xsize gui.get_int(nvl_text_prefix + 'width')
					xanchor    gui[nvl_text_prefix + 'xalign']
					text_align gui[nvl_text_prefix + 'xalign']
					
					font         _dialogue_font
					text_size    gui.get_int('dialogue_text_size')
					color        _dialogue_color
					outlinecolor _dialogue_outlinecolor



screen dialogue_box:
	zorder -2
	
	key 'h' action SetDict(db, 'hide_interface', not db.hide_interface)
	
	$ db.to_next = False
	for key in ('RETURN', 'SPACE'):
		key key:
			action db.enter_action
			first_delay style.key.first_delay if config.long_next_is_skipping else 1e9
			delay       style.key.delay       if config.long_next_is_skipping else 1e9
	if db.to_next:
		$ db.skip_tab = False
	
	key 'LEFT SHIFT'  action SetDict(db, 'last_shift_time', get_game_time()) first_delay 0
	key 'RIGHT SHIFT' action SetDict(db, 'last_shift_time', get_game_time()) first_delay 0
	key 'LEFT ALT'    action SetDict(db, 'last_alt_time', get_game_time()) first_delay 0
	key 'RIGHT ALT'   action SetDict(db, 'last_alt_time', get_game_time()) first_delay 0
	
	$ db.prev_ctrl = db.ctrl
	$ db.ctrl = False
	key 'LEFT CTRL'  action SetDict(db, 'ctrl', True) first_delay 0
	key 'RIGHT CTRL' action SetDict(db, 'ctrl', True) first_delay 0
	if db.ctrl and not db.prev_ctrl:
		$ db.press_ctrl_time = get_game_time()
	
	if db.visible:
		key 'TAB' action ToggleDict(db, 'skip_tab')
	
	python:
		db.skip = False
		if get_game_time() - max(db.last_shift_time, db.last_alt_time) > db.no_skip_time_alt_shift:
			db.skip_ctrl = db.ctrl and get_game_time() - db.press_ctrl_time > db.no_skip_time_ctrl
			if db.skip_ctrl or db.skip_tab:
				db.hide_interface = False
				db.skip = True
				db.to_next = True
		else:
			db.skip_ctrl = db.skip_tab = False
		
		if db.to_next:
			db.on_enter()
	
	if not db.hide_interface:
		$ db.recalc_props()
		
		if db.visible:
			if db.mode == 'adv':
				use dialogue_box_adv
			elif db.mode == 'nvl':
				use dialogue_box_nvl
			else:
				$ out_msg('dialogue_box', 'Expected db.mode will be "adv" or "nvl", got "%s"' % (str(db.mode), ))
			
			if db.skip:
				text 'Skip Mode':
					style 'skip_text'
		
		button:
			ground 'images/bg/black.jpg'
			hover  'images/bg/black.jpg'
			
			size  1.0
			alpha 0.01
			mouse False
			
			action    db.on_enter
			alternate pause_screen.show
		
		use dialogue_box_buttons
	
	else:
		button:
			ground 'images/bg/black.jpg'
			hover  'images/bg/black.jpg'
			
			size   1.0
			alpha  0.01
			mouse  False
			
			action SetDict(db, 'hide_interface', False)

