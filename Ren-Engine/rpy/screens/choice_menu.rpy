screen choice_menu:
	modal True
	zorder 100
	
	image 'images/bg/black.jpg':
		size  1.0
		alpha 0.05
	
	vbox:
		align (0.5, 0.5)
		spacing gui.get_int('choice_spacing')
		
		python:
			if gui.choice_button_width is not None:
				choice_max_width = gui.choice_button_width
			else:
				choice_max_width = 0
				for variant in choice_menu_variants:
					if variant:
						choice_max_width = max(choice_max_width, utf8.width(_(variant), gui.choice_button_text_size))
			choice_max_width = max(choice_max_width, gui.choice_button_width_min)
		
		for i in xrange(len(choice_menu_variants)):
			if choice_menu_variants[i]:
				textbutton _(choice_menu_variants[i]):
					font      gui.choice_button_text_font
					text_size gui.get_int('choice_button_text_size')
					color        gui.get_int('choice_button_text_color')
					outlinecolor gui.get_int('choice_button_text_outlinecolor')
					text_align gui.choice_button_text_xalign
					
					xsize choice_max_width
					ysize gui.get_int('choice_button_height')
					
					ground gui.bg('choice_button_ground')
					hover  gui.bg('choice_button_hover')
					action Return(i)
			elif choice_menu_variants[i] is not None:
				null ysize gui.choice_button_height
	
	use dialogue_box_buttons(disable_next_btn = True)
