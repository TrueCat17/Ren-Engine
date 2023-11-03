init -995 python:
	quick_menu = True # set to False to disable quick_menu
	
	def quick_menu_screen__update():
		tmp_style = style.quick_button
		std_xsize = tmp_style.get_current('xsize') if tmp_style.xsize > 0 else 0
		text_size = tmp_style.get_current('text_size')
		if tmp_style.hover_text_size:
			text_size = max(text_size, tmp_style.get_current('hover_text_size'))
		
		spacing = style.quick_buttons_hbox.get_current('spacing', get_stage_width())
		
		quick_menu_screen.full_xsize = 0
		quick_menu_screen.buttons = []
		
		for item in quick_menu_screen.items:
			if len(item) == 2:
				text, action = item
			else:
				text, action, condition = item
				if not condition():
					continue
			
			text = quick_menu_screen.prefixes.get(text, '') + _(text)
			
			xsize = std_xsize if std_xsize > 0 else get_text_width(text, text_size)
			quick_menu_screen.full_xsize += xsize + spacing
			
			quick_menu_screen.buttons.append((text, action, xsize))
		
		if quick_menu_screen.full_xsize > spacing:
			quick_menu_screen.full_xsize -= spacing
		
		tmp_style = style.quick_buttons_bg
		if tmp_style.xsize > 0:
			quick_menu_screen.full_xsize = tmp_style.xsize
	
	build_object('quick_menu_screen')
	
	
	
	quick_menu_screen.items = [
		# text, action, [condition function (not lambda!)]
		['History', ShowScreen('history')],
		['Skip',    ToggleDict(db, 'skip_tab')],
		['Save',    ShowScreen('save')],
		['Q.Save',  QuickSave(), Eval('config.has_quicksave')],
		['Q.Load',  QuickLoad(), Eval('config.has_quicksave')],
		['Prefs',   ShowScreen('preferences')],
	]
	
	# for example: quick_menu_screen.prefixes['Skip'] = '{image=images/icons/skip.png} '
	quick_menu_screen.prefixes = {}



screen quick_menu:
	ysize style.quick_buttons_bg.get_current('ysize')
	
	$ quick_menu_screen.update()
	image gui.bg('quick_buttons_bg' if db.visible else 'quick_buttons_bg_without_window'):
		style 'quick_buttons_bg'
		xsize quick_menu_screen.full_xsize
		
		hbox:
			style 'quick_buttons_hbox'
			
			$ tmp_style = style.quick_button
			for text, action, xsize in quick_menu_screen.buttons:
				textbutton text:
					style tmp_style
					xsize xsize
					ground tmp_style.get_ground(xsize)
					hover  tmp_style.get_hover(xsize)
					action action
