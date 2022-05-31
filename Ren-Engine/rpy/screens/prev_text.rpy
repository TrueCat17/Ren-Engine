init python:
	def prev_text__show():
		prev_text.showed_time = get_game_time()
		prev_text.hided_time = 0
		show_screen('prev_text')
	
	def prev_text__close():
		prev_text.hided_time = get_game_time()
	
	build_object('prev_text')
	
	prev_text.background = im.rect('#181818BB')
	
	prev_text.spacing = 5
	prev_text.x_indent = 20
	prev_text.xsize = 0.75
	prev_text.ysize = 0.95
	
	slider_v_init('prev_text', 0, prev_text.ysize)
	
	
	prev_text.showed_time = 0
	prev_text.hided_time = 0
	prev_text.appearance_time = 0.4
	prev_text.disappearance_time = 0.4


screen prev_text:
	modal True
	zorder 10000
	
	key 'ESCAPE' action prev_text.close
	
	button:
		ground 'images/bg/black.jpg'
		size  1.0
		alpha 0.01
		mouse False
		action prev_text.close
	
	
	python:
		dtime = get_game_time() - prev_text.showed_time
		x = int(-prev_text.xsize * get_stage_width() * (prev_text.appearance_time - dtime) / prev_text.appearance_time)
		if x > 0:
			x = 0
		
		prev_text.viewport_content_height = len(db.prev_texts) * (db.text_size + prev_text.spacing) * 2 # 2 - extra space for wordwraps
		slider_v_change('prev_text', length = prev_text.viewport_content_height, button_size = db.text_size)
		y = int(-slider_v_get_value('prev_text') * (prev_text.viewport_content_height - prev_text.ysize * get_stage_height()))
		
		if prev_text.hided_time:
			dtime = get_game_time() - prev_text.hided_time
			alpha = (prev_text.disappearance_time - dtime) / prev_text.disappearance_time
			if alpha <= 0:
				hide_screen('prev_text')
		else:
			alpha = 1
	
	image prev_text.background:
		clipping True
		alpha alpha
		xpos x
		yalign 0.5
		size (prev_text.xsize, prev_text.ysize)
		
		vbox:
			ypos y
			spacing prev_text.spacing
			
			for name_text, name_color, text, text_color in db.prev_texts:
				if name_text:
					$ tmp_name = '{color=' + hex(name_color) + '}' + name_text + '{/color}: '
				else:
					$ tmp_name = ''
				
				text (tmp_name + text):
					text_size db.text_size
					color text_color
					xsize int(prev_text.xsize * get_stage_width()) - prev_text.x_indent * 2 - db.text_size
					xpos prev_text.x_indent
		
		null:
			align (0.99, 0.5)
			
			$ slider_v_set('prev_text')
			use slider_v

