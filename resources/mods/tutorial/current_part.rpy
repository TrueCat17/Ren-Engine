init python:
	def current_part__show(text):
		current_part.last_update_time = get_game_time()
		if current_part.show_time is None:
			current_part.show_time = get_game_time()
		current_part.hide_time = None
		current_part.text = text
		show_screen('current_part')
	
	def current_part__hide():
		current_part.show_time = None
		current_part.hide_time = get_game_time()
	
	
	def current_part__get_alpha():
		if not current_part.hide_time:
			dtime = get_game_time() - current_part.show_time
			return dtime / current_part.appearance_time
		else:
			dtime = get_game_time() - current_part.hide_time
			if dtime > current_part.appearance_time:
				hide_screen('current_part')
			return 1 - dtime / current_part.appearance_time
	
	def current_part__get_text():
		text = current_part.text
		if get_game_time() - current_part.last_update_time < current_part.selected_time:
			text = '{color=%s}%s{/color}' % (current_part.selected_color, text)
		
		return _(current_part.text_template) % text
	
	
	current_part = SimpleObject()
	build_object('current_part')
	
	current_part.hide()
	
	current_part.appearance_time = 0.5
	current_part.bg = im.round_rect('#111C', 50, 50, 0, 16, 0, 16)
	
	current_part.selected_color = '#F80'
	current_part.selected_time = 1.0
	
	current_part.text_template = 'Part:\n[%s]'


screen current_part:
	alpha current_part.get_alpha()
	
	image current_part.bg:
		corner_sizes -1
		size (0.2, 0.12)
		size_min (200,  75)
		size_max (280, 100)
		
		text current_part.get_text():
			align 0.5
			text_align 'center'
			
			font 'Fregat'
			color '#08F'
			
			text_size 0.035
			text_size_min 18
			text_size_max 26
