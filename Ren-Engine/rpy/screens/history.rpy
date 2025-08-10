init python:
	def history__on_show(screen_name):
		if screen_name != 'history':
			return
		
		history.closing = False
		
		smooth_changes.start(
			'history',
			history.pre_start_props, history.start_props, history.cur_props,
			history.appearance_time, None,
		)
	
	signals.add('show_screen', history__on_show)
	
	
	def history__close():
		if history.closing:
			return
		history.closing = True
		
		smooth_changes.start(
			'history',
			history.cur_props.copy(), history.end_props, history.cur_props,
			history.disappearance_time, HideScreen('history'),
		)
	
	
	build_object('history')
	history.cur_props = {}
	
	history.pre_start_props = {
		'xpos': 0,
		'ypos': 0.5,
		'xanchor': 1.0,
		'yanchor': 0.5,
		'alpha': 1.0,
	}
	history.start_props = {
		'xpos': 0,
		'ypos': 0.5,
		'xanchor': 0.0,
		'yanchor': 0.5,
		'alpha': 1.0,
	}
	history.end_props = {
		'xpos': 0,
		'ypos': 0.5,
		'xanchor': 0.0,
		'yanchor': 0.5,
		'alpha': 0.0,
	}
	
	history.ysize = 0.95
	
	history.appearance_time = 0.4
	history.disappearance_time = 0.4
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('history')
	
	
	def history__init_slider():
		slider_v.init('history', history.ysize, value = 1.0)
	signals.add('inited', history__init_slider, times = 1)


screen history:
	modal True
	zorder 10000
	
	key 'ESCAPE' action history.close
	button:
		ground 'images/bg/black.jpg'
		size  1.0
		alpha 0.01
		mouse False
		action    history.close
		alternate history.close
	
	
	python:
		screen_tmp = SimpleObject()
		
		screen_tmp.spacing = 0 if gui.history_height else gui.get_int('history_spacing')
		
		screen_tmp.name_text_size = gui.get_int('name_text_size')
		screen_tmp.dialogue_text_size = gui.get_int('dialogue_text_size')
		screen_tmp.text_size = max(screen_tmp.name_text_size, screen_tmp.dialogue_text_size)
		
		screen_tmp.usual_xsize    = gui.get_int('history_text_xpos')    + gui.get_int('history_text_width')    + 70
		screen_tmp.narrator_xsize = gui.get_int('history_thought_xpos') + gui.get_int('history_thought_width') + 70
		screen_tmp.xsize = max(screen_tmp.usual_xsize, screen_tmp.narrator_xsize)
		
		smooth_changes.update('history')
		screen_tmp.__dict__.update(history.cur_props)
	
	image gui.bg('history_bg'):
		clipping True
		alpha screen_tmp.alpha
		xpos screen_tmp.xpos
		ypos screen_tmp.ypos
		xanchor screen_tmp.xanchor
		yanchor screen_tmp.yanchor
		xsize screen_tmp.xsize
		ysize history.ysize
		
		vbox:
			ysize_min history.ysize
			yalign slider_v.get_value('history')
			spacing screen_tmp.spacing
			
			null ysize 1 # for spacing before first text line
			
			python:
				screen_tmp.name_text_yoffset     = max(screen_tmp.dialogue_text_size - screen_tmp.name_text_size, 0)
				screen_tmp.dialogue_text_yoffset = max(screen_tmp.name_text_size - screen_tmp.dialogue_text_size, 0)
				
				screen_tmp.history_name_xpos   = gui.get_int('history_name_xpos')
				screen_tmp.history_name_ypos   = gui.get_int('history_name_ypos') + screen_tmp.name_text_yoffset
				screen_tmp.history_name_width  = gui.get_int('history_name_width')
				screen_tmp.history_name_xalign = gui.history_name_xalign
				
				for name in ('text', 'thought'):
					history_text_prefix = 'history_%s_' % name
					
					for prop in ('xpos', 'ypos', 'width'):
						screen_tmp[history_text_prefix + prop] = gui.get_int(history_text_prefix + prop)
					screen_tmp[history_text_prefix + 'xalign'] = gui[history_text_prefix + 'xalign']
				
				screen_tmp.history_height = gui.get_int('history_height') if gui.history_height else -1
			
			for text_object in db.prev_texts:
				null:
					ysize screen_tmp.history_height
					
					if text_object.name_text:
						text text_object.name_text:
							xpos  screen_tmp.history_name_xpos
							ypos  screen_tmp.history_name_ypos
							xsize screen_tmp.history_name_width
							xanchor    screen_tmp.history_name_xalign
							text_align screen_tmp.history_name_xalign
							text_size  screen_tmp.name_text_size
							
							font         text_object.name_font
							color        text_object.name_color
							outlinecolor text_object.name_outlinecolor
					
					$ history_text_prefix = 'history_%s_' % ('text' if text_object.name_text else 'thought')
					text text_object.dialogue_text:
						xpos       screen_tmp[history_text_prefix + 'xpos']
						ypos       screen_tmp[history_text_prefix + 'ypos'] + (screen_tmp.dialogue_text_yoffset if text_object.name_text else 0)
						xsize      screen_tmp[history_text_prefix + 'width']
						xanchor    screen_tmp[history_text_prefix + 'xalign']
						text_align screen_tmp[history_text_prefix + 'xalign']
						text_size  screen_tmp.dialogue_text_size
						
						font         text_object.dialogue_font
						color        text_object.dialogue_color
						outlinecolor text_object.dialogue_outlinecolor
		
		null:
			align (0.99, 0.5)
			use slider_v('history')
