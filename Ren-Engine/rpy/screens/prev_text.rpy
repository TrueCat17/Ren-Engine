init python:
	pt_background = im.rect('#181818BB')
	
	pt_spacing = 5
	pt_x_indent = 20
	pt_xsize = 0.75
	pt_ysize = 0.95
	
	slider_v_init('prev_text', 0, pt_ysize)
	
	
	pt_showed_time = 0
	pt_hided_time = 0
	pt_appearance_time = 0.4
	pt_disappearance_time = 0.4
	
	def prev_text_show():
		global pt_showed_time, pt_hided_time
		pt_showed_time = get_game_time()
		pt_hided_time = 0
		show_screen('prev_text')
	
	def prev_text_close():
		global pt_hided_time
		pt_hided_time = get_game_time()


screen prev_text:
	modal True
	zorder 10000
	
	key 'ESCAPE' action prev_text_close
	
	button:
		ground 'images/bg/black.jpg'
		size (1.0, 1.0)
		alpha 0.01
		mouse False
		action prev_text_close
	
	
	python:
		dtime = get_game_time() - pt_showed_time
		x = int(-pt_xsize * get_stage_width() * (pt_appearance_time - dtime) / pt_appearance_time)
		if x > 0:
			x = 0
		
		pt_viewport_content_height = len(db_prev_texts) * (db_text_size + pt_spacing) * 2 # 2 - extra space for wordwraps
		slider_v_change('prev_text', length = pt_viewport_content_height, button_size = db_text_size)
		y = int(-slider_v_get_value('prev_text') * (pt_viewport_content_height - pt_ysize * get_stage_height()))
		
		if pt_hided_time:
			dtime = get_game_time() - pt_hided_time
			alpha = (pt_disappearance_time - dtime) / pt_disappearance_time
			if alpha <= 0:
				renpy.hide_screen('prev_text')
		else:
			alpha = 1
	
	image pt_background:
		clipping True
		alpha alpha
		xpos x
		yalign 0.5
		size (pt_xsize, pt_ysize)
		
		vbox:
			ypos y
			spacing pt_spacing
			
			for name_text, name_color, text, text_color in db_prev_texts:
				python:
					if name_text:
						tmp_name = '{color=' + hex(name_color)[2:] + '}' + name_text + '{/color}: '
					else:
						tmp_name = ''
				
				text (tmp_name + text):
					text_size db_text_size
					color text_color
					xsize int(pt_xsize * get_stage_width()) - pt_x_indent * 2 - db_text_size
					xpos pt_x_indent
		
		null:
			align (0.99, 0.5)
			
			$ slider_v_set('prev_text')
			use slider_v

