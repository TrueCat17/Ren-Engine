init -1 python:
	save_background = gui + 'menu/main/back.png'


screen save:
	zorder 10001
	modal  True
	
	if not has_screen('pause') and not has_screen('choose_menu'):
		use hotkeys
	
	python:
		if not sl_inited:
			init_sl()
		if random.random() < 4.0 / get_fps():
			sl_update_table_saves()
	
	if not save_screenshotting:
		image save_background:
			size (1.0, 1.0)
			
			text ('{outlinecolor=0}' + _('Save')):
				align (0.5, 0.02)
				
				color 0xFFFFFF
				text_size get_stage_height() / 10
			
			
			hbox:
				align (0.5, 0.5)
				spacing 20
				
				vbox:
					yalign 0.5
					spacing 5
					
					for i in sl_tables:
						textbutton i:
							xsize  0.10
							ysize  0.05
							action [SetVariable('sl_cur_table', i), sl_update_table_saves]
				
				vbox:
					yalign 0.5
					spacing 20
				
					python:
						x_count = 4
						y_count = int(math.ceil(len(sl_table_saves) / 4))
						
						xsize = int(0.15 * get_stage_width())
						ysize = int(0.15 * get_stage_height())
						
						spacing = 7
						
						buttons = []
						
						h = 0
						for i in xrange(y_count):
							w = 0
							for j in xrange(x_count):
								index = i * x_count + j
								if index < len(sl_table_saves):
									name = sl_table_saves[index]
									save_exists = sl_table_saves_exists[name]
									
									button = Object()
									button.pos   = (w, h)
									button.size  = (xsize, ysize)
									button.ground = sl_get_screenshot(sl_cur_table, name, save_exists)
									button.action = [SetVariable('sl_cur_save', name), sl_update_table_saves]
									buttons.append(button)
									
									if save_exists:
										button.desc = sl_get_datetime(sl_cur_table, name)
									else:
										button.desc = ''
									
									w += xsize + spacing
							h += ysize + spacing
						
						w = xsize * x_count + spacing * (x_count - 1)
						h = ysize * y_count + spacing * (y_count - 1)
					
					null size (w, h):
						for button in buttons:
							null:
								pos  button.pos
								size button.size
								
								button:
									size   button.size
									ground button.ground
									action button.action
								
								text button.desc:
									align 0.1
					
					hbox:
						xalign 0.5
						spacing 5
						
						$ sl_cur_save_exists = sl_table_saves_exists.get(sl_cur_save, False)
						
						textbutton _('Save game'):
							action Function(sl_save, sl_cur_table, sl_cur_save)
						
						textbutton _('Delete'):
							alpha 1 if sl_cur_save_exists else 0.7
							
							mouse   sl_cur_save_exists
							action (Function(sl_delete_save, sl_cur_table, sl_cur_save) if sl_cur_save_exists else None)
			
			textbutton _('Return'):
				align (0.95, 0.95)
				action HideMenu('save')
	
	key 'ESCAPE' action HideMenu('save')

