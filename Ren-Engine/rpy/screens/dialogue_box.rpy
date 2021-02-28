init -1000 python:
	# db = dialogue box
	
	db_read = True
	def db_read_func():
		return db_read
	can_exec_next_check_funcs.append(db_read_func)
	
	
	db_pause_after_text = 0
	db_pause_end = 0
	
	db_dialogue = []
	
	db_visible = False
	db_hide_interface = False
	db_skip_tab = False
	db_mode = 'adv'
	
	db_font = style.text.font
	
	db_name = gui + 'dialogue/name.png'
	db_name_color = 0xFF0000
	db_name_text = ''
	
	db_voice = gui + 'dialogue/voice.png'
	db_voice_color = 0xFFFF00
	db_voice_text = ''
	db_voice_full_text = ''
	db_voice_text_after_pause = ''
	
	db_menu_btn = gui + 'dialogue/to_menu.png'
	db_menu_btn_size = 50
	db_menu_btn_indent = 20
	
	db_next_btn = gui + 'dialogue/to_next.png'
	db_next_btn_size = 50
	
	db_prev_btn = gui + 'dialogue/to_prev.png'
	db_prev_btn_size = 50
	
	db_prev_texts = []
	
	
	def db_make_step(text, start_index, max_count_symbols = -1):
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
	
	
	def show_text(name, name_prefix, name_postfix, name_color, text, text_prefix, text_postfix, text_color):
		global db_name_text, db_name_color
		global db_voice_text, db_voice_full_text, db_last_text_postfix, db_voice_color
		global db_voice_text_after_pause, db_pause_after_text, db_pause_end
		global db_read, db_start_time, db_prev_texts
		
		db_pause_after_text = 0
		db_pause_end = 0
		db_voice_text_after_pause = ''
		
		prev_index = None
		index = 0
		while prev_index != index:
			prev_index = index
			index, symbols, tag, value = db_make_step(text, index)
			
			if tag == 'w':
				db_pause_after_text = 1e9
				if value:
					try:
						db_pause_after_text = float(value)
					except:
						out_msg('show_text', 'Pause time <' + value + '> is not float')
				
				db_voice_text_after_pause = text[index:]
				text = text[0:prev_index]
				break
		
		db_read = False
		
		# new text
		if name is not None:
			db_start_time = get_game_time()
			
			db_name_text = name_prefix + name + name_postfix
			db_name_color = name_color
			
			db_voice_text = ''
			db_voice_full_text = text_prefix + text
			if not db_voice_text_after_pause:
				db_voice_full_text += text_postfix
			db_last_text_postfix = text_postfix
		
		# continuation of prev text
		else:
			db_start_time = get_game_time() - len_unicode(db_voice_text) / float(renpy.config.text_cps)
			
			db_voice_full_text += text
			if not db_voice_text_after_pause:
				db_voice_full_text += db_last_text_postfix
		
		text_object = (db_name_text, db_name_color, text, text_color)
		db_prev_texts.append(text_object)
		db_prev_texts = db_prev_texts[-config.count_prev_texts:]
		
		db_voice_color = text_color
		
		window_show()
	
	
	def db_update():
		global db_text_size, db_voice_size, db_voice_text
		db_text_size = max(14, get_stage_height() / 30)
		db_voice_size = get_stage_width() - (db_prev_btn_size + db_next_btn_size + 20), int(max(80, 0.2 * get_stage_height()))
		
		if db_voice_text != db_voice_full_text:
			symbols_to_render = int((get_game_time() - db_start_time) * renpy.config.text_cps)
			
			prev_index = None
			index = 0
			while symbols_to_render and prev_index != index:
				prev_index = index
				index, symbols, tag, value = db_make_step(db_voice_full_text, index, symbols_to_render)
				symbols_to_render -= symbols
			
			if index < len(db_voice_full_text):
				db_voice_text = db_voice_full_text[0:index] + '{invisible}' + db_voice_full_text[index:]
			else:
				db_voice_text = db_voice_full_text
		else:
			global db_pause_after_text, db_pause_end
			if db_pause_after_text != 0:
				if db_pause_end == 0:
					db_pause_end = get_game_time() + db_pause_after_text
				elif db_pause_end < get_game_time():
					db_pause_after_text = 0
					db_pause_end = 0
					show_text(None, '', '', 0, db_voice_text_after_pause, '', db_last_text_postfix, db_voice_color)
	
	
	def enter_action():
		global db_hide_interface, db_to_next
		if db_hide_interface:
			db_hide_interface = False
		else:
			db_to_next = True
	
	def db_on_enter():
		skip_exec_current_command()
		
		global db_pause_end, db_dialogue, db_name_text, db_voice_text, db_voice_full_text, db_read
		
		if db_pause_end > get_game_time():
			db_pause_end = get_game_time()
			return
		
		if db_voice_text == db_voice_full_text:
			if db_read:
				return
			db_read = True
			
			if db_mode == 'nvl':
				db_dialogue += [(db_name_text, db_name_color, db_voice_text, db_voice_color)]
				db_name_text = db_voice_text = db_voice_full_text = ''
			else:
				db_dialogue = []
		else:
			db_voice_text = db_voice_full_text
	
	
	window_show = SetVariable('db_visible', True)
	window_hide = SetVariable('db_visible', False)
	
	def nvl_clear():
		global db_dialogue
		db_dialogue = []
	
	def set_mode_adv():
		global db_mode
		db_mode = 'adv'
		nvl_clear()
	def set_mode_nvl():
		global db_mode
		db_mode = 'nvl'
		nvl_clear()


screen dialogue_box:
	zorder -2
	
	key 'h' action SetVariable('db_hide_interface', not db_hide_interface)
	
	$ db_to_next = False
	key 'RETURN' action enter_action
	key 'SPACE'  action enter_action
	if db_to_next:
		$ db_skip_tab = False
	
	$ db_skip_ctrl = False
	key 'LEFT CTRL'  action SetVariable('db_skip_ctrl', True) first_delay 0
	key 'RIGHT CTRL' action SetVariable('db_skip_ctrl', True) first_delay 0
	key 'TAB'    action  SetVariable('db_skip_tab', not db_skip_tab)
	key 'ESCAPE' action [SetVariable('db_skip_tab', False), SetVariable('db_hide_interface', False)]
	
	python:
		if db_skip_ctrl or db_skip_tab:
			db_hide_interface = False
			db_to_next = True
		
		if db_to_next:
			db_on_enter()
	
	if not db_hide_interface:
		$ db_update()
		
		if db_visible:
			
			button:
				ground 'images/bg/black.jpg'
				hover  'images/bg/black.jpg'
				
				size  (1.0, 1.0)
				alpha (0.01 if db_mode == 'adv' else 0.30)
				mouse False
				
				action db_on_enter
			
			
			if db_mode == 'adv':
				vbox:
					align (0.5, 0.99)
					
					image db_name:
						xpos max(get_stage_width() / 10, db_prev_btn_size * 2)
						size (max(250, get_stage_width() / 5), int(db_text_size * 1.5))
						
						text db_name_text:
							font       db_font
							text_align 'center'
							text_size  db_text_size
							color      db_name_color
							align      (0.5, 0.8)
					
					hbox:
						spacing 5
						xalign 0.5
						
						button:
							yalign 0.5
							ground db_prev_btn
							size   (db_prev_btn_size, db_prev_btn_size)
							action prev_text_show
						
						image db_voice:
							size db_voice_size
							
							text db_voice_text:
								font      db_font
								text_size db_text_size
								color     db_voice_color
								align     (0.5, 0.5)
								size      (db_voice_size[0] - 30, db_voice_size[1] - 15)
						
						button:
							yalign 0.5
							ground db_next_btn
							size   (db_next_btn_size, db_next_btn_size)
							action db_on_enter
			
			
			elif db_mode == 'nvl':
				vbox:
					anchor 	(0.5, 0.0)
					pos		(0.5, 0.05)
					
					
					$ db_last_dialogue = db_dialogue + [(db_name_text, db_name_color, db_voice_text, db_voice_color)]
					
					for db_name_text_i, db_name_color_i, db_voice_text_i, db_voice_color_i in db_last_dialogue:
						python:
							db_tmp_name = ('{color=' + hex(db_name_color_i)[2:] + '}' + db_name_text_i + '{/color}: ') if db_name_text_i else ''
							db_tmp_voice = db_voice_text_i if db_voice_text_i else ' '
						
						text (db_tmp_name + db_tmp_voice):
							font      db_font
							text_size db_text_size
							color     db_voice_color_i
							xsize     0.75
				
				vbox:
					align (0.5, 0.99)
					
					null ysize int(db_text_size * 1.5)
					
					hbox:
						spacing 5
						xalign 0.5
						
						button:
							yalign 0.5
							ground db_prev_btn
							size   (db_prev_btn_size, db_prev_btn_size)
							action prev_text_show
						
						null size db_voice_size
						
						button:
							yalign 0.5
							ground db_next_btn
							size   (db_next_btn_size, db_next_btn_size)
							action db_on_enter
			
			if db_skip_ctrl or db_skip_tab:
				text 'Skip Mode':
					color 0xFFFFFF
					text_size 30
					pos (20, 20)
		
		
		button:
			ground 	db_menu_btn
			
			anchor (0.5, 0.5)
			pos    (get_stage_width() - db_menu_btn_indent - db_menu_btn_size / 2, db_menu_btn_indent + db_menu_btn_size / 2)
			size   (db_menu_btn_size, db_menu_btn_size)
			action show_pause
	else:
		button:
			ground 'images/bg/black.jpg'
			hover  'images/bg/black.jpg'
			
			size   (1.0, 1.0)
			alpha  0.01
			mouse  False
			
			action SetVariable('db_hide_interface', False)

