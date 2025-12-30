init:
	style code_text is text:
		xpos 0.04
		yalign 0.5
		font 'Monospace'
		text_size 0.035
		text_size_min 14

init python:
	def code__show(text):
		if not has_screen('code'):
			show_screen('code')
		
		data = SimpleObject()
		data.show_time = get_game_time()
		data.hide_time = None
		data.text = code_coloring.colorize(text)
		data.count_lines = data.text.count('\n') + 1
		
		code.old_data, code.new_data = code.new_data, data
		if code.old_data:
			code.old_data.hide_time = get_game_time()
	
	def code__hide():
		code.old_data, code.new_data = code.new_data, None
		if code.old_data:
			code.old_data.hide_time = get_game_time()
	
	def code__showed():
		return (code.old_data is not None) or (code.new_data is not None)
	
	
	def code__get_bg_ysize():
		line_ysize = style.code_text.get_current('text_size') * 14 // 10
		
		old_data, new_data = code.old_data, code.new_data
		if old_data and new_data:
			old_ysize = old_data.count_lines * line_ysize
			new_ysize = new_data.count_lines * line_ysize
			
			dtime = get_game_time() - new_data.show_time
			k = min(dtime / code.appearance_time, 1)
			return interpolate(old_ysize, new_ysize, k)
		
		data = old_data or new_data
		return (data.count_lines if data else 0) * line_ysize
	
	def code__get_bg_alpha():
		if code.old_data and code.new_data:
			return 1
		
		if code.old_data:
			return code.get_text_alpha(code.old_data)
		if code.new_data:
			return code.get_text_alpha(code.new_data)
		return 0
	
	
	def code__get_text_alpha(data):
		if db.hide_interface:
			return 0
		
		if data.hide_time is None:
			dtime = get_game_time() - data.show_time
			return dtime / code.appearance_time
		else:
			dtime = get_game_time() - data.hide_time
			if dtime > code.appearance_time and code.old_data:
				code.old_data = None
				if not code.new_data:
					hide_screen('code')
			return 1 - dtime / code.appearance_time
	
	
	code = SimpleObject()
	build_object('code')
	
	code.appearance_time = 0.25
	
	code.old_data = None
	code.new_data = None
	
	code.bg = im.round_rect('#111C', 50, 50, 16)


screen code:
	image code.bg:
		corner_sizes -1
		xpos 0.72
		xanchor 0.5
		ypos 0.35
		yanchor 0.5
		
		clipping True
		xsize 0.5
		ysize code.get_bg_ysize()
		alpha code.get_bg_alpha()
		
		for data in (code.old_data, code.new_data):
			if data:
				text data.text:
					style 'code_text'
					alpha code.get_text_alpha(data)
