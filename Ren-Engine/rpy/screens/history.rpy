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
	
	history.spacing = 5
	history.x_indent = 20
	history.xsize = 0.75
	history.ysize = 0.95
	
	slider_v_init('history', 0, history.ysize)
	
	
	history.showed_time = 0
	history.hided_time = 0
	history.appearance_time = 0.4
	history.disappearance_time = 0.4


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
		dtime = get_game_time() - history.showed_time
		x = int(-history.xsize * get_stage_width() * (history.appearance_time - dtime) / history.appearance_time)
		if x > 0:
			x = 0
		
		history.viewport_content_height = len(db.prev_texts) * (db.text_size + history.spacing) * 2 # 2 - extra space for wordwraps
		slider_v_change('history', length = history.viewport_content_height, button_size = db.text_size)
		y = int(-slider_v_get_value('history') * (history.viewport_content_height - history.ysize * get_stage_height()))
		
		if history.hided_time:
			dtime = get_game_time() - history.hided_time
			alpha = (history.disappearance_time - dtime) / history.disappearance_time
			if alpha <= 0:
				hide_screen('history')
		else:
			alpha = 1
	
	image history.background:
		clipping True
		alpha alpha
		xpos x
		yalign 0.5
		size (history.xsize, history.ysize)
		
		vbox:
			ypos y
			spacing history.spacing
			
			for name_text, name_color, text, text_color in db.prev_texts:
				if name_text:
					$ tmp_name = '{color=' + hex(name_color) + '}' + name_text + '{/color}: '
				else:
					$ tmp_name = ''
				
				text (tmp_name + text):
					text_size db.text_size
					color text_color
					xsize int(history.xsize * get_stage_width()) - history.x_indent * 2 - db.text_size
					xpos history.x_indent
		
		null:
			align (0.99, 0.5)
			
			$ slider_v_set('history')
			use slider_v

