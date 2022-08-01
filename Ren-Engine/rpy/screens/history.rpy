init python:
	def history__on_show(screen_name):
		if screen_name == 'history':
			history.showed_time = get_game_time()
			history.hided_time = 0
	signals.add('show_screen', history__on_show)
	
	def history__close():
		history.hided_time = get_game_time()
	
	build_object('history')
	
	history.background = im.rect('#181818BB')
	
	history.ysize = 0.95
	
	slider_v_init('history', 0, history.ysize)
	
	
	history.showed_time = 0
	history.hided_time = 0
	history.appearance_time = 0.4
	history.disappearance_time = 0.4
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('history')


screen history:
	modal True
	zorder 10000
	
	key 'ESCAPE' action history.close
	
	button:
		ground 'images/bg/black.jpg'
		size  1.0
		alpha 0.01
		mouse False
		action history.close
	
	
	python:
		history.spacing = 0 if gui.history_height else gui.history_spacing
		history.text_size = max(gui.get_int('name_text_size'), gui.get_int('dialogue_text_size'))
		
		history.usual_xsize    = float(gui.get_int('history_text_xpos')    + gui.get_int('history_text_width')    + 70)
		history.narrator_xsize = float(gui.get_int('history_thought_xpos') + gui.get_int('history_thought_width') + 70)
		history.xsize = max(history.usual_xsize, history.narrator_xsize) / get_stage_width()
		
		dtime = get_game_time() - history.showed_time
		history.x = int(-history.xsize * get_stage_width() * (history.appearance_time - dtime) / history.appearance_time)
		if history.x > 0:
			history.x = 0
		
		if gui.history_height:
			history.viewport_content_height = len(db.prev_texts) * gui.get_int('history_height')
		else:
			history.viewport_content_height = len(db.prev_texts) * (history.text_size + history.spacing) * 2 # 2 - extra space for wordwraps
		slider_v_change('history', length = history.viewport_content_height, button_size = history.text_size)
		history.y = int(-slider_v_get_value('history') * (history.viewport_content_height - history.ysize * get_stage_height()))
		
		if history.hided_time:
			dtime = get_game_time() - history.hided_time
			history.alpha = (history.disappearance_time - dtime) / history.disappearance_time
			if history.alpha <= 0:
				hide_screen('history')
		else:
			history.alpha = 1
	
	image history.background:
		clipping True
		alpha history.alpha
		xpos history.x
		yalign 0.5
		xsize history.xsize
		ysize history.ysize
		
		vbox:
			ypos history.y
			spacing history.spacing
			
			null ysize 1 # for spacing before first text line
			
			$ _name_text_yoffset     = max(gui.get_int('dialogue_text_size') - gui.get_int('name_text_size'), 0)
			$ _dialogue_text_yoffset = max(gui.get_int('name_text_size') - gui.get_int('dialogue_text_size'), 0)
			for _name_text, _name_font, _name_color, _name_outlinecolor, _dialogue_text, _dialogue_font, _dialogue_color, _dialogue_outlinecolor in db.prev_texts:
				null:
					ysize gui.get_int('history_height') if gui.history_height else -1
					
					if _name_text:
						text _name_text:
							xpos gui.get_int('history_name_xpos')
							ypos gui.get_int('history_name_ypos') + _name_text_yoffset
							xsize gui.get_int('history_name_width')
							xanchor    gui.history_name_xalign
							text_align gui.history_name_xalign
							
							font         _name_font
							text_size    gui.get_int('name_text_size')
							color        _name_color
							outlinecolor _name_outlinecolor
					
					$ history_text_prefix = 'history_' + ('text' if _name_text else 'thought') + '_'
					text _dialogue_text:
						xpos gui.get_int(history_text_prefix + 'xpos')
						ypos gui.get_int(history_text_prefix + 'ypos') + (_dialogue_text_yoffset if _name_text else 0)
						xsize gui.get_int(history_text_prefix + 'width')
						xanchor    gui[history_text_prefix + 'xalign']
						text_align gui[history_text_prefix + 'xalign']
						
						font         _dialogue_font
						text_size    gui.get_int('dialogue_text_size')
						color        _dialogue_color
						outlinecolor _dialogue_outlinecolor
		
		null:
			align (0.99, 0.5)
			
			$ slider_v_set('history')
			use slider_v

