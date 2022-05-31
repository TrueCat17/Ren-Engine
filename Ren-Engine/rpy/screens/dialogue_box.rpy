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
			
			# maybe start/end style, for example: {size=25} or {/b}
			if index < l and text[index] == '{':
				
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
			
			# going to the next symbol
			while index < l and not is_first_byte(text[index]):
				index += 1
		
		return index, symbols, tag, value
	
	
	def db__show_text(name, name_prefix, name_postfix, name_color, text, text_prefix, text_postfix, text_color):
		db.pause_after_text = 0
		db.pause_end = 0
		db.voice_text_after_pause = ''
		
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
				
				db.voice_text_after_pause = text[index:]
				text = text[0:prev_index]
				break
		
		db.read = False
		
		# new text
		if name is not None:
			db.start_time = get_game_time()
			
			db.name_text = name_prefix + name + name_postfix
			db.name_color = name_color
			
			db.voice_text = ''
			db.voice_full_text = text_prefix + text
			if not db.voice_text_after_pause:
				db.voice_full_text += text_postfix
			db.last_text_postfix = text_postfix
		
		# continuation of prev text
		else:
			db.start_time = get_game_time() - len_unicode(db.voice_text) / float(renpy.config.text_cps)
			
			db.voice_full_text += text
			if not db.voice_text_after_pause:
				db.voice_full_text += db.last_text_postfix
		
		text_object = (db.name_text, db.name_color, text, text_color)
		db.prev_texts.append(text_object)
		db.prev_texts = db.prev_texts[-config.count_prev_texts:]
		
		db.voice_color = text_color
		
		window_show()
	
	
	def db__update():
		db.text_size = max(14, get_stage_height() / 30)
		db.voice_size = get_stage_width() - (db.prev_btn_size + db.next_btn_size + 20), int(max(80, 0.2 * get_stage_height()))
		
		if db.voice_text != db.voice_full_text:
			symbols_to_render = int((get_game_time() - db.start_time) * renpy.config.text_cps)
			
			prev_index = None
			index = 0
			while symbols_to_render and prev_index != index:
				prev_index = index
				index, symbols, tag, value = db.make_step(db.voice_full_text, index, symbols_to_render)
				symbols_to_render -= symbols
			
			if index < len(db.voice_full_text):
				db.voice_text = db.voice_full_text[0:index] + '{invisible}' + db.voice_full_text[index:]
			else:
				db.voice_text = db.voice_full_text
		else:
			if db.pause_after_text != 0:
				if db.pause_end == 0:
					db.pause_end = get_game_time() + db.pause_after_text
				elif db.pause_end < get_game_time():
					db.pause_after_text = 0
					db.pause_end = 0
					db.show_text(None, '', '', 0, db.voice_text_after_pause, '', db.last_text_postfix, db.voice_color)
	
	
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
		
		if db.voice_text == db.voice_full_text:
			if db.read:
				return
			db.read = True
			
			if db.mode == 'nvl':
				db.dialogue += [(db.name_text, db.name_color, db.voice_text, db.voice_color)]
				db.name_text = db.voice_text = db.voice_full_text = ''
			else:
				db.dialogue = []
		else:
			db.voice_text = db.voice_full_text
	
	
	
	build_object('db')
	
	
	db.pause_after_text = 0
	db.pause_end = 0
	
	db.dialogue = []
	
	db.visible = False
	db.hide_interface = False
	db.skip_tab = False
	db.mode = 'adv'
	
	db.font = style.text.font
	
	db.name = gui + 'dialogue/name.png'
	db.name_color = 0xFF0000
	db.name_text = ''
	
	db.voice = gui + 'dialogue/voice.png'
	db.voice_color = 0xFFFF00
	db.voice_text = ''
	db.voice_full_text = ''
	db.voice_text_after_pause = ''
	
	db.menu_btn = gui + 'dialogue/to_menu.png'
	db.menu_btn_size = 50
	db.menu_btn_indent = 20
	
	db.next_btn = gui + 'dialogue/to_next.png'
	db.next_btn_size = 50
	
	db.prev_btn = gui + 'dialogue/to_prev.png'
	db.prev_btn_size = 50
	
	db.prev_texts = []
	
	
	db.read = True
	def db_read_func():
		return db.read
	can_exec_next_check_funcs.append(db_read_func)
	
	
	
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


screen dialogue_box:
	zorder -2
	
	key 'h' action SetDict(db, 'hide_interface', not db.hide_interface)
	
	$ db.to_next = False
	key 'RETURN' action db.enter_action
	key 'SPACE'  action db.enter_action
	if db.to_next:
		$ db.skip_tab = False
	
	$ db.skip_ctrl = False
	key 'LEFT CTRL'  action SetDict(db, 'skip_ctrl', True) first_delay 0
	key 'RIGHT CTRL' action SetDict(db, 'skip_ctrl', True) first_delay 0
	if db.visible:
		key 'TAB'    action  SetDict(db, 'skip_tab', not db.skip_tab)
	key 'ESCAPE' action [SetDict(db, 'skip_tab', False), SetDict(db, 'hide_interface', False)]
	
	python:
		if db.skip_ctrl or db.skip_tab:
			db.hide_interface = False
			db.to_next = True
		
		if db.to_next:
			db.on_enter()
	
	if not db.hide_interface:
		$ db.update()
		
		if db.visible:
			
			button:
				ground 'images/bg/black.jpg'
				hover  'images/bg/black.jpg'
				
				size  (1.0, 1.0)
				alpha (0.01 if db.mode == 'adv' else 0.30)
				mouse False
				
				action db.on_enter
			
			
			if db.mode == 'adv':
				vbox:
					align (0.5, 0.99)
					
					image db.name:
						xpos max(get_stage_width() / 10, db.prev_btn_size * 2)
						size (max(250, get_stage_width() / 5), int(db.text_size * 1.5))
						
						text db.name_text:
							font       db.font
							text_align 'center'
							text_size  db.text_size
							color      db.name_color
							align      (0.5, 0.8)
					
					hbox:
						spacing 5
						xalign 0.5
						
						button:
							yalign 0.5
							ground db.prev_btn
							size   (db.prev_btn_size, db.prev_btn_size)
							action prev_text_show
						
						image db.voice:
							size db.voice_size
							
							text db.voice_text:
								font      db.font
								text_size db.text_size
								color     db.voice_color
								align     (0.5, 0.5)
								size      (db.voice_size[0] - 30, db.voice_size[1] - 15)
						
						button:
							yalign 0.5
							ground db.next_btn
							size   (db.next_btn_size, db.next_btn_size)
							action db.on_enter
			
			
			elif db.mode == 'nvl':
				vbox:
					anchor 	(0.5, 0.0)
					pos		(0.5, 0.05)
					
					
					$ db.last_dialogue = db.dialogue + [(db.name_text, db.name_color, db.voice_text, db.voice_color)]
					
					for _name_text_i, _name_color_i, _voice_text_i, _voice_color_i in db.last_dialogue:
						$ _tmp_name = ('{color=' + hex(_name_color_i) + '}' + _name_text_i + '{/color}: ') if _name_text_i else ''
						$ _tmp_voice = _voice_text_i or ''
						
						text (_tmp_name + _tmp_voice):
							font      db.font
							text_size db.text_size
							color     _voice_color_i
							xsize     0.75
				
				vbox:
					align (0.5, 0.99)
					
					null ysize int(db.text_size * 1.5)
					
					hbox:
						spacing 5
						xalign 0.5
						
						button:
							yalign 0.5
							ground db.prev_btn
							size   (db.prev_btn_size, db.prev_btn_size)
							action prev_text_show
						
						null size db.voice_size
						
						button:
							yalign 0.5
							ground db.next_btn
							size   (db.next_btn_size, db.next_btn_size)
							action db.on_enter
			
			if db.skip_ctrl or db.skip_tab:
				text 'Skip Mode':
					color 0xFFFFFF
					text_size 30
					pos (20, 20)
		
		
		button:
			ground 	db.menu_btn
			
			anchor (0.5, 0.5)
			pos    (get_stage_width() - db.menu_btn_indent - db.menu_btn_size / 2, db.menu_btn_indent + db.menu_btn_size / 2)
			size   (db.menu_btn_size, db.menu_btn_size)
			action pause_screen.show
	else:
		button:
			ground 'images/bg/black.jpg'
			hover  'images/bg/black.jpg'
			
			size   (1.0, 1.0)
			alpha  0.01
			mouse  False
			
			action SetDict(db, 'hide_interface', False)

