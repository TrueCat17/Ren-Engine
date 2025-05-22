screen choice_menu:
	modal True
	zorder 100
	
	image gui.bg('choice_buttons_bg'):
		style 'choice_buttons_bg'
	
	vbox:
		style 'choice_buttons_vbox'
		
		python:
			screen_tmp = SimpleObject()
			
			tmp_style = style.choice_button
			if tmp_style.xsize > 0:
				screen_tmp.max_width = tmp_style.get_current('xsize')
			else:
				screen_tmp.text_size = tmp_style.get_current('text_size')
				if tmp_style.hover_text_size:
					screen_tmp.text_size = max(screen_tmp.text_size, tmp_style.get_current('hover_text_size'))
				
				screen_tmp.max_width = max(0, tmp_style.get_current('xsize_min'))
				for variant in choice_menu_variants:
					if variant:
						screen_tmp.max_width = max(screen_tmp.max_width, get_text_width(_(variant), screen_tmp.text_size))
				if tmp_style.xsize_max > 0:
					screen_tmp.max_width = min(screen_tmp.max_width, tmp_style.get_current('xsize_max'))
		
		for i, text in enumerate(choice_menu_variants):
			if text:
				textbutton _(text):
					style 'choice_button'
					xsize screen_tmp.max_width
					action Return(i)
			elif text is not None:
				null style 'choice_button'
	
	use dialogue_box_buttons(disable_next_btn = True)
