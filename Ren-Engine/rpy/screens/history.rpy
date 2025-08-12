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
	
	def history__update():
		screen_tmp.spacing = 0 if gui.history_height else gui.get_int('history_spacing')
		
		screen_tmp.name_text_size = gui.get_int('name_text_size')
		screen_tmp.dialogue_text_size = gui.get_int('dialogue_text_size')
		screen_tmp.text_size = max(screen_tmp.name_text_size, screen_tmp.dialogue_text_size)
		
		sw, sh = get_stage_size()
		screen_tmp.padding_left   = get_absolute(history.padding[0], sw)
		screen_tmp.padding_top    = get_absolute(history.padding[1], sh)
		screen_tmp.padding_right  = get_absolute(history.padding[2], sw)
		screen_tmp.padding_bottom = get_absolute(history.padding[3], sh)
		
		if history.xsize is None:
			usual_xsize    = gui.get_int('history_text_xpos')    + gui.get_int('history_text_width')
			narrator_xsize = gui.get_int('history_thought_xpos') + gui.get_int('history_thought_width')
			screen_tmp.xsize = max(usual_xsize, narrator_xsize)
			screen_tmp.xsize += screen_tmp.padding_left + screen_tmp.padding_right
			screen_tmp.xsize += screen_tmp.padding_right + history.slider_width
		else:
			screen_tmp.xsize = get_absolute(history.xsize, sw)
		
		screen_tmp.container_xsize = screen_tmp.xsize - (screen_tmp.padding_left + screen_tmp.padding_right)
		screen_tmp.container_ysize = get_absolute(history.ysize, sh) - (screen_tmp.padding_top + screen_tmp.padding_bottom)
		
		smooth_changes.update('history')
		screen_tmp.__dict__.update(history.cur_props)
	
	
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
	
	#history.title_text = 'History'
	history.title_text = ''
	
	history.xsize = None # None - auto
	history.ysize = 0.95
	history.bg_corner_sizes = 0
	history.padding = [15, 10, 15, 10] # left, top, right, bottom
	
	# None - auto
	history.slider_ground = None
	history.slider_hover  = None
	history.slider_button_style = None
	history.slider_width = 25 # only in pixels
	
	history.appearance_time = 0.4
	history.disappearance_time = 0.4
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('history')
	
	
	def history__init_slider():
		slider_v.init(
			'history', 0.5, value = 1.0,
			ground = history.slider_ground,
			hover  = history.slider_hover,
			button_size  = history.slider_width,
			button_style = history.slider_button_style,
		)
		history__on_resized_stage()
	signals.add('inited', history__init_slider, times = 1)
	
	def history__on_resized_stage():
		sh = get_stage_height()
		padding_top    = get_absolute(history.padding[1], sh)
		padding_bottom = get_absolute(history.padding[3], sh)
		size = get_absolute(history.ysize, sh) - (padding_top + padding_bottom)
		slider_v.change('history', size = size)
	signals.add('resized_stage', history__on_resized_stage)


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
	
	
	$ screen_tmp = SimpleObject()
	$ history.update()
	
	image gui.bg('history_bg'):
		clipping True
		alpha screen_tmp.alpha
		xpos screen_tmp.xpos
		ypos screen_tmp.ypos
		xanchor screen_tmp.xanchor
		yanchor screen_tmp.yanchor
		xsize screen_tmp.xsize
		ysize history.ysize
		corner_sizes history.bg_corner_sizes
		
		if history.title_text:
			text _(history.title_text):
				style 'history_title' if style.history_title else 'text'
		
		null:
			xpos screen_tmp.padding_left
			ypos screen_tmp.padding_top
			xsize screen_tmp.container_xsize
			ysize screen_tmp.container_ysize
			
			vbox:
				ysize_min screen_tmp.container_ysize
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
							if text_object.history_name_bg:
								image text_object.history_name_bg:
									style text_object.history_name_bg_style
							
							text (text_object.history_name_prefix + text_object.name_text + text_object.history_name_suffix):
								xpos  screen_tmp.history_name_xpos
								ypos  screen_tmp.history_name_ypos
								xsize screen_tmp.history_name_width
								xanchor    screen_tmp.history_name_xalign
								text_align screen_tmp.history_name_xalign
								text_size  screen_tmp.name_text_size
								
								font         text_object.name_font
								color        text_object.name_color
								outlinecolor text_object.name_outlinecolor
						
						text (text_object.history_text_prefix + text_object.dialogue_text + text_object.history_text_suffix):
							
							$ history_text_prefix = 'history_%s_' % ('text' if text_object.name_text else 'thought')
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
				align (1.0, 0.5)
				use slider_v('history')
