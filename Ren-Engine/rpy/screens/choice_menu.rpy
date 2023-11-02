screen choice_menu:
	modal True
	zorder 100
	
	image gui.bg('choice_buttons_bg'):
		style 'choice_buttons_bg'
	
	vbox:
		style 'choice_buttons_vbox'
		
		python:
			tmp_style = style.choice_button
			if tmp_style.xsize > 0:
				choice_max_width = tmp_style.get_current('xsize')
			else:
				choice_text_size = tmp_style.get_current('text_size')
				if tmp_style.hover_text_size:
					choice_text_size = max(choice_text_size, tmp_style.get_current('hover_text_size'))
				
				choice_max_width = max(0, tmp_style.get_current('xsize_min'))
				for variant in choice_menu_variants:
					if variant:
						choice_max_width = max(choice_max_width, get_text_width(_(variant), choice_text_size))
				if tmp_style.xsize_max > 0:
					choice_max_width = min(choice_max_width, tmp_style.get_current('xsize_max'))
		
		for i, text in enumerate(choice_menu_variants):
			if text:
				textbutton _(text):
					style tmp_style
					xsize choice_max_width
					ground tmp_style.get_ground(choice_max_width)
					hover  tmp_style.get_hover(choice_max_width)
					action Return(i)
			elif text is not None:
				null style tmp_style
	
	use dialogue_box_buttons(disable_next_btn = True)
