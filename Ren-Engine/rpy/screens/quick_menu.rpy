init -995 python:
	quick_menu = True # set to False to disable quick_menu
	
	style.quick_hbox = Style(style.hbox)
	style.quick_hbox.align = (0.5, 1.0)
	
	quick_menu_screen = Object()
	quick_menu_screen.items = [
		# text, action, [condition function (not lambda!)]
		['History', ShowScreen('history')],
		['Skip',    ToggleDict(db, 'skip_tab')],
		['Save',    ShowScreen('save')],
		['Q.Save',  QuickSave(), Eval('config.has_quicksave')],
		['Q.Load',  QuickLoad(), Eval('config.has_quicksave')],
		['Prefs',   ShowScreen('preferences')],
	]


screen quick_menu:
	has hbox
	style 'quick_hbox'
	
	for item in quick_menu_screen.items:
		python:
			if len(item) == 2:
				text, action = item
				condition = True
			else:
				text, action, condition = item
				condition = condition()
		
		if condition:
			textbutton _(text):
				font      gui.quick_button_text_font
				text_size gui.get_int('quick_button_text_size')
				color        gui.get_int('quick_button_text_color')
				outlinecolor gui.get_int('quick_button_text_outlinecolor')
				text_align gui.quick_button_text_xalign
				
				xsize gui.get_int('quick_button_width', default=get_text_width(_(text), gui.get_int('quick_button_text_size')))
				ysize gui.get_int('quick_button_height')
				
				ground gui.quick_button_ground
				hover  gui.quick_button_hover
				action action
