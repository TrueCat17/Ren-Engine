init -1000 python:
	# db = dialogue box
	
	
	def db__make_step(text, start_index, start_symbols):
		l = len(text)
		index = start_index
		symbols = start_symbols
		tag = value = None
		
		while index < l:
			while index < l and text[index] in ' \n':
				index += 1
			if index == l:
				break
			
			# maybe style start/end, for example: {size=25} or {/b}
			if text[index] == '{':
				
				# not style, '{{' renders to '{'
				if index + 1 < l and text[index + 1] == '{':
					index += 2
					symbols += 1
				
				# style
				else:
					if symbols != start_symbols:
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
	
	# without tags, space symbols and escaped {
	def db__count_symbols(text):
		prev_index = None
		index = 0
		symbols = 0
		while prev_index != index:
			prev_index = index
			index, symbols, tag, value = db.make_step(text, index, symbols)
		return symbols
	
	
	def db__update_styles(local_styles = None):
		if local_styles is None:
			local_styles = db.local_styles or {}
		db.local_styles = local_styles
		
		for style in ('dialogue_box', 'dialogue_text', 'name_box', 'name_text'):
			props = []
			for prop in ('xpos', 'ypos', 'width', 'height'):
				props += [prop, prop + '_min', prop + '_max']
			
			if style.endswith('text'):
				props += ['font', 'size', 'size_min', 'size_max', 'color', 'outlinecolor']
			else:
				props += ['bg']
			
			for prop in props:
				prop = style + '_' + prop
				value = (local_styles if prop in local_styles else gui)[prop]
				if prop.endswith('color'):
					value = color_to_int(value)
				elif prop.endswith('bg') and callable(value):
					value = value()
				db[prop] = value
		
		for prop in ('history_name_bg', 'history_name_bg_style'):
			db[prop] = (local_styles if prop in local_styles else gui)[prop]
		
		# prefixes and suffixes
		for prop in character_text_edges:
			if prop in local_styles:
				value = local_styles[prop]
			else:
				prop_without_mode = prop.replace('nvl_', '').replace('history_', '')
				if prop_without_mode in local_styles:
					value = local_styles[prop_without_mode]
				else:
					value = gui[prop]
			db[prop] = value
		
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
	
	def db__show_text(name, text, local_styles, need_interpolate = True):
		if need_interpolate:
			text = text.replace('%%', '%') # percent escaping from old renpy versions (see config.old_substitutions for renpy)
			text = interpolate_tags(text)
		
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
		symbols = 0
		while prev_index != index:
			prev_index = index
			index, symbols, tag, value = db.make_step(text, index, symbols)
			
			if tag == 'w':
				db.pause_after_text = 1e9
				if value:
					try:
						db.pause_after_text = float(value)
					except:
						out_msg('db.show_text', 'Pause time <%s> is not float', value)
				
				db.dialogue_text_after_pause = text[index:]
				text = text[:prev_index]
				break
		
		db.read = False
		
		# new text
		if name is not None:
			db.start_time = get_game_time()
			
			db.name_text = name
			db.dialogue_text = text
			db.visible_symbols = 0
		
		# continuation of prev text (for <extend> character)
		else:
			db.start_time = get_game_time() - db.all_visible_symbols / config.text_cps
			
			db.dialogue_text += text
			db.visible_symbols = db.all_visible_symbols
		
		mode_prefix = 'nvl_' if db.mode == 'nvl' else ''
		prefix_symbols = db.count_symbols(db[mode_prefix + 'text_prefix'])
		suffix_symbols = db.count_symbols(db[mode_prefix + 'text_suffix'])
		
		db.all_visible_symbols = prefix_symbols + db.count_symbols(db.dialogue_text) + suffix_symbols
		
		if name is not None or db.dialogue_text_after_pause or not db.prev_texts:
			if text:
				text_object = db.get_cur_text_object(text)
				db.dialogue.append(text_object)
				db.prev_texts.append(text_object)
				db.prev_texts[:-config.history_length] = []
		else:
			text_object = db.prev_texts[-1]
			text_object.dialogue_text += text
		
		window_show()
	
	
	def db__recalc_props():
		if db.local_styles:
			screen_tmp.dialogue_button_spacing = gui.get_int('dialogue_button_spacing', obj = db)
			screen_tmp.dialogue_button_width   = gui.get_int('dialogue_button_width', obj = db)
			screen_tmp.dialogue_button_height  = gui.get_int('dialogue_button_height', obj = db)
			if screen_tmp.dialogue_button_width <= 0 or screen_tmp.dialogue_button_height <= 0:
				screen_tmp.dialogue_button_width = screen_tmp.dialogue_button_height = screen_tmp.dialogue_button_spacing = 0
			
			screen_tmp.dialogue_menu_button_width  = gui.get_int('dialogue_menu_button_width', obj = db)
			screen_tmp.dialogue_menu_button_height = gui.get_int('dialogue_menu_button_height', obj = db)
			if screen_tmp.dialogue_menu_button_width <= 0 or screen_tmp.dialogue_menu_button_height <= 0:
				screen_tmp.dialogue_menu_button_width = screen_tmp.dialogue_menu_button_height = 0
			
			if db.dialogue_box_width:
				screen_tmp.dialogue_box_width = gui.get_int('dialogue_box_width', obj = db)
			else:
				screen_tmp.dialogue_box_width = ( # spc btn spc dialogue spc btn spc
					get_stage_width() - 4 * screen_tmp.dialogue_button_spacing - 2 * screen_tmp.dialogue_button_width
				)
			screen_tmp.dialogue_box_height = gui.get_int('dialogue_box_height', obj = db)
			
			screen_tmp.dialogue_text_xpos = gui.get_int('dialogue_text_xpos', obj = db)
			screen_tmp.dialogue_text_ypos = gui.get_int('dialogue_text_ypos', obj = db)
			
			screen_tmp.dialogue_text_width = gui.dialogue_text_width
			if not screen_tmp.dialogue_text_width:
				screen_tmp.dialogue_text_width = screen_tmp.dialogue_box_width - 2 * screen_tmp.dialogue_text_xpos
			screen_tmp.dialogue_text_height = gui.dialogue_text_height
			if not screen_tmp.dialogue_text_height:
				screen_tmp.dialogue_text_height = screen_tmp.dialogue_box_height - 2 * screen_tmp.dialogue_text_ypos
			
			screen_tmp.is_auto = db.name_box_width is None
			if screen_tmp.is_auto:
				db.name_box_width = get_text_width(db.name_text_prefix + db.name_text + db.name_text_suffix, gui.get_int('name_text_size', obj = db)) + 20
			screen_tmp.name_box_width = gui.get_int('name_box_width', obj = db)
			if screen_tmp.is_auto:
				db.name_box_width = None
		
		
		if db.visible_symbols != db.all_visible_symbols:
			db.visible_symbols = int((get_game_time() - db.start_time) * config.text_cps)
			db.visible_symbols = min(db.visible_symbols, db.all_visible_symbols)
		else:
			if db.pause_after_text != 0:
				if db.pause_end == 0:
					db.pause_end = get_game_time() + db.pause_after_text
				elif db.pause_end <= get_game_time():
					db.pause_after_text = 0
					db.pause_end = 0
					db.show_text(None, db.dialogue_text_after_pause, db.local_styles, need_interpolate = False)
	
	
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
		
		if db.visible_symbols == db.all_visible_symbols:
			if db.read:
				return
			db.read = True
			
			if db.mode != 'nvl':
				db.dialogue = []
		else:
			db.visible_symbols = db.all_visible_symbols
	
	def db__get_cur_text_object(text = None):
		if text is None:
			text = db.dialogue_text
		
		res = SimpleObject()
		res.dialogue_text = text
		
		for prop in character_text_edges + ('history_name_bg', 'history_name_bg_style'):
			res[prop] = db[prop]
		
		for prop in ('name_text', 'name_text_font', 'name_text_color', 'name_text_outlinecolor', 'dialogue_text_font', 'dialogue_text_color', 'dialogue_text_outlinecolor'):
			simple_prop = prop.replace('text_', '')
			res[simple_prop] = db[prop]
		
		return res
	
	
	def db__get_no_skip_time(keys):
		if config.pause_before_skip_on_ctrl:
			return db['no_skip_time_' + keys]
		return 0
	
	
	
	build_object('db')
	
	
	db.pause_after_text = 0
	db.pause_end = 0
	
	db.dialogue = []
	db.prev_texts = []
	
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
	
	
	db.read = True
	def db__showed_completely():
		return db.read
	can_exec_next_check_funcs.append(db__showed_completely)
	
	
	def db__disable_skipping_on_menu(screen_name):
		if screen_name == 'pause':
			db.skip_tab = False
		elif screen_name == 'choice_menu' and not config.skip_after_choices:
			db.skip_tab = False
	signals.add('show_screen', db__disable_skipping_on_menu)
	
	
	
	def window_show():
		db.visible = True
	def window_hide():
		db.visible = False
	
	def nvl_clear():
		db.dialogue = []
	
	def set_mode_adv():
		db.mode = 'adv'
		nvl_clear()
	def set_mode_nvl():
		db.mode = 'nvl'
		nvl_clear()
	
	
	def db__on_init_end():
		narrator('')
		db.read = True
		window_hide()
	signals.add('inited', db__on_init_end, times = 1)



screen dialogue_box_buttons(disable_next_btn = False):
	$ screen_tmp = db.screen_tmp # if included from screen <choice_menu>
	
	vbox:
		xalign gui.dialogue_box_xalign
		yalign gui.dialogue_box_yalign
		spacing gui.get_int('quick_buttons_top_indent')
		
		if screen_tmp.get('dialogue_button_width') and db.visible:
			hbox:
				spacing screen_tmp.dialogue_button_spacing
				xalign 0.5
				
				button:
					corner_sizes 0
					ground db.dialogue_prev_ground
					hover  style.get_default_hover(db.dialogue_prev_hover, db.dialogue_prev_ground)
					action    show_screen('history')
					alternate pause_screen.show
					
					yalign db.dialogue_button_yalign
					xsize screen_tmp.dialogue_button_width
					ysize screen_tmp.dialogue_button_height
				
				null size (screen_tmp.dialogue_box_width, screen_tmp.dialogue_box_height)
				
				if not screen.disable_next_btn:
					button:
						corner_sizes 0
						ground db.dialogue_next_ground
						hover  style.get_default_hover(db.dialogue_next_hover, db.dialogue_next_ground)
						action    db.on_enter
						alternate pause_screen.show
						
						yalign db.dialogue_button_yalign
						xsize screen_tmp.dialogue_button_width
						ysize screen_tmp.dialogue_button_height
				else:
					null:
						xsize screen_tmp.dialogue_button_width
						ysize screen_tmp.dialogue_button_height
		else:
			null:
				ysize screen_tmp.get('dialogue_button_height', 0)
		
		if quick_menu:
			$ screen.tmp = screen_tmp
			use quick_menu
			$ screen_tmp = screen.tmp
	
	if screen_tmp.get('dialogue_menu_button_width'):
		button:
			corner_sizes 0
			ground db.dialogue_menu_button_ground
			hover  style.get_default_hover(db.dialogue_menu_button_hover, db.dialogue_menu_button_ground)
			action    pause_screen.show
			alternate pause_screen.show
			
			xpos gui.dialogue_menu_button_xpos
			ypos gui.dialogue_menu_button_ypos
			xanchor gui.dialogue_menu_button_xanchor
			yanchor gui.dialogue_menu_button_yanchor
			xsize screen_tmp.dialogue_menu_button_width
			ysize screen_tmp.dialogue_menu_button_height
	
	use dialogue_box_skip_keys


screen dialogue_box_adv:
	vbox:
		xalign gui.dialogue_box_xalign
		yalign gui.dialogue_box_yalign
		spacing gui.get_int('quick_buttons_top_indent')
		
		image db.dialogue_box_bg:
			xsize screen_tmp.dialogue_box_width
			ysize screen_tmp.dialogue_box_height
			
			image db.name_box_bg:
				xanchor gui.name_box_xanchor
				yanchor gui.name_box_yanchor
				xpos gui.get_int('name_box_xpos', obj = db)
				ypos gui.get_int('name_box_ypos', obj = db)
				xsize screen_tmp.name_box_width
				ysize gui.get_int('name_box_height', obj = db)
				
				alpha 1 if db.name_text else 0
				
				text (db.name_prefix + db.name_text + db.name_suffix):
					font         db.name_text_font
					text_size    gui.get_int('name_text_size', obj = db)
					color        db.name_text_color
					outlinecolor db.name_text_outlinecolor
					xalign       gui.name_text_xalign
					yalign       gui.name_text_yalign
			
			text (db.text_prefix + db.dialogue_text + db.text_suffix):
				visible_symbols db.visible_symbols
				
				xpos gui.get_int('dialogue_text_xpos', obj = db)
				ypos gui.get_int('dialogue_text_ypos', obj = db)
				xsize screen_tmp.dialogue_text_width
				ysize screen_tmp.dialogue_text_height
				
				font         db.dialogue_text_font
				text_size    gui.get_int('dialogue_text_size', obj = db)
				color        db.dialogue_text_color
				outlinecolor db.dialogue_text_outlinecolor
				text_align   gui.dialogue_text_align
		
		if quick_menu:
			null ysize style.quick_buttons_bg.get_current('ysize')


screen dialogue_box_nvl:
	image gui.nvl_bg:
		size 1.0
	
	vbox:
		ypos gui.get_int('nvl_top_indent')
		
		spacing 0 if gui.nvl_height else gui.get_int('nvl_spacing')
		
		python:
			screen_tmp.name_text_size     = gui.get_int('name_text_size')
			screen_tmp.dialogue_text_size = gui.get_int('dialogue_text_size')
			
			screen_tmp.name_text_yoffset     = max(screen_tmp.dialogue_text_size - screen_tmp.name_text_size, 0)
			screen_tmp.dialogue_text_yoffset = max(screen_tmp.name_text_size - screen_tmp.dialogue_text_size, 0)
			
			screen_tmp.nvl_name_xpos   = gui.get_int('nvl_name_xpos')
			screen_tmp.nvl_name_ypos   = gui.get_int('nvl_name_ypos')
			screen_tmp.nvl_name_width  = gui.get_int('nvl_name_width')
			screen_tmp.nvl_name_xalign = gui.nvl_name_xalign
			
			for name in ('text', 'thought'):
				nvl_text_prefix = 'nvl_%s_' % name
				
				for prop in ('xpos', 'ypos', 'width'):
					screen_tmp[nvl_text_prefix + prop] = gui.get_int(nvl_text_prefix + prop)
				screen_tmp[nvl_text_prefix + 'xalign'] = gui[nvl_text_prefix + 'xalign']
			
			screen_tmp.nvl_height = gui.get_int('nvl_height') if gui.nvl_height else -1
		
		for text_object in db.dialogue:
			null:
				ysize screen_tmp.nvl_height
				
				if text_object.name_text:
					text (text_object.nvl_name_prefix + text_object.name_text + text_object.nvl_name_suffix):
						xpos  screen_tmp.nvl_name_xpos
						ypos  screen_tmp.nvl_name_ypos + screen_tmp.name_text_yoffset
						xsize screen_tmp.nvl_name_width
						xanchor    screen_tmp.nvl_name_xalign
						text_align screen_tmp.nvl_name_xalign
						text_size  screen_tmp.name_text_size
						
						font         text_object.name_font
						color        text_object.name_color
						outlinecolor text_object.name_outlinecolor
				
				text (text_object.nvl_text_prefix + text_object.dialogue_text + text_object.nvl_text_suffix):
					visible_symbols db.visible_symbols if text_object is db.dialogue[-1] else 1e9
					
					$ nvl_text_prefix = 'nvl_%s_' % ('text' if text_object.name_text else 'thought')
					xpos  screen_tmp[nvl_text_prefix + 'xpos']
					ypos  screen_tmp[nvl_text_prefix + 'ypos'] + (screen_tmp.dialogue_text_yoffset if text_object.name_text else 0)
					xsize screen_tmp[nvl_text_prefix + 'width']
					xanchor    screen_tmp[nvl_text_prefix + 'xalign']
					text_align screen_tmp[nvl_text_prefix + 'xalign']
					text_size  screen_tmp.dialogue_text_size
					
					font         text_object.dialogue_font
					color        text_object.dialogue_color
					outlinecolor text_object.dialogue_outlinecolor



screen dialogue_box_skip_keys:
	# not skip instantly (option for users with shortcuts on Ctrl+...)
	if config.pause_before_skip_on_ctrl:
		key 'LEFT SHIFT'  action 'db.last_shift_time = get_game_time()' first_delay 0
		key 'RIGHT SHIFT' action 'db.last_shift_time = get_game_time()' first_delay 0
		key 'LEFT ALT'    action 'db.last_alt_time = get_game_time()' first_delay 0
		key 'RIGHT ALT'   action 'db.last_alt_time = get_game_time()' first_delay 0
	
	$ db.prev_ctrl = db.ctrl
	$ db.ctrl = False
	key 'LEFT CTRL'  action 'db.ctrl = True' first_delay 0
	key 'RIGHT CTRL' action 'db.ctrl = True' first_delay 0
	if db.ctrl and not db.prev_ctrl:
		$ db.press_ctrl_time = get_game_time()
	
	key 'TAB' action 'db.skip_tab = not db.skip_tab'


screen dialogue_box_skip_text:
	zorder 100
	
	if db.skip:
		text 'Skip Mode':
			style 'skip_text'
	
	python:
		if not has_screen('dialogue_box'):
			hide_screen('dialogue_box_skip_text')


screen dialogue_box:
	zorder -2
	
	python:
		# save tmp in db, because it used in screen <dialogue_box_buttons>, that can be included from <choice_menu>
		screen_tmp = db.screen_tmp = SimpleObject()
		
		if not has_screen('dialogue_box_skip_text'):
			show_screen('dialogue_box_skip_text')
	
	key 'H' action 'db.hide_interface = not db.hide_interface; db.skip_tab = False'
	
	$ db.to_next = False
	for key in ('RETURN', 'SPACE'):
		key key:
			action db.enter_action
			first_delay style.key.first_delay if config.long_next_is_skipping else 1e9
	if db.to_next:
		$ db.skip_tab = False
	
	python:
		db.skip = False
		if get_game_time() - max(db.last_shift_time, db.last_alt_time) > db.get_no_skip_time('alt_shift'):
			db.skip_ctrl = db.ctrl and get_game_time() - db.press_ctrl_time >= db.get_no_skip_time('ctrl')
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
		
		if db.visible and db.local_styles:
			if db.mode == 'adv':
				use dialogue_box_adv
			elif db.mode == 'nvl':
				use dialogue_box_nvl
			else:
				$ out_msg('dialogue_box', "Expected db.mode will be 'adv' or 'nvl', got %r", db.mode)
		
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
			
			action    'db.hide_interface = False'
			alternate 'db.hide_interface = False'
