init -1000 python:
	def pause_screen__init(screen_name):
		if screen_name != 'pause': return
		signals.remove('show_screen', pause_screen__init)
		
		ground = im.matrix_color(gui + 'menu/pause/button.png', im.matrix.invert() * im.matrix.tint(  0, 0.5, 1))
		hover  = im.matrix_color(gui + 'menu/pause/button.png', im.matrix.invert() * im.matrix.tint(0.2, 0.6, 1))
		
		ground = get_back_with_color(ground, alpha = 0.5)
		hover  = get_back_with_color(hover,  alpha = 0.2)
		
		style.pause_screen_button = Style(style.textbutton)
		style.pause_screen_button.ground = ground
		style.pause_screen_button.hover  = hover
		style.pause_screen_button.xsize = 0.2
		style.pause_screen_button.ysize = 0.1
		style.pause_screen_button.text_size = 20
	signals.add('show_screen', pause_screen__init)
	
	
	def pause_screen__show(ignore_checks = False):
		if not ignore_checks and pause_screen.disable: return
		if has_screen('pause'): return
		if get_game_time() - pause_screen.hided_time < pause_screen.rotate_time + pause_screen.disappearance_time: return
		
		pause_screen.showed_time = 0
		pause_screen.start_hided_time = 0
		pause_screen.before_fps = get_fps()
		set_fps(pause_screen.fps)
		show_screen('pause')
	
	def pause_screen__close():
		if get_game_time() - pause_screen.start_hided_time < pause_screen.rotate_time + pause_screen.disappearance_time: return
		if not pause_screen.showed_time: return
		if get_game_time() - pause_screen.showed_time < pause_screen.appearance_time: return
		
		pause_screen.start_hided_time = get_game_time()
	
	build_object('pause_screen')
	pause_screen.inited = False
	pause_screen.disable = False
	
	pause_screen.showed_time = 0
	pause_screen.start_hided_time = 0
	pause_screen.hided_time = 0
	
	pause_screen.before_fps = None
	pause_screen.fps = 60
	
	pause_screen.x, pause_screen.y = 0, 0
	pause_screen.rotate = 0
	pause_screen.hide_rotate = 15
	pause_screen.rotate_time = 0.2
	pause_screen.disappearance_time = 0.2
	pause_screen.appearance_time = 0.4


screen pause:
	zorder 10000
	modal  True
	
	if not has_screen('choose_menu'):
		use hotkeys
	
	python:
		if pause_screen.showed_time:
			dtime = get_game_time() - pause_screen.showed_time
		else:
			dtime = 0
			signals.add('enter_frame', Exec('pause_screen.showed_time = get_game_time()'), times=1)
		
		if dtime > pause_screen.appearance_time:
			pause_screen.y = 0
			if pause_screen.start_hided_time:
				if get_game_time() - pause_screen.start_hided_time < pause_screen.rotate_time:
					pause_screen.rotate = pause_screen.hide_rotate * (get_game_time() - pause_screen.start_hided_time) / pause_screen.rotate_time
				else:
					pause_screen.rotate = pause_screen.hide_rotate
					dtime = get_game_time() - pause_screen.start_hided_time
					pause_screen.x = pause_screen.y = (dtime - pause_screen.rotate_time) / pause_screen.disappearance_time
					
					if pause_screen.x >= 1:
						pause_screen.x, pause_screen.y = 0, 0
						pause_screen.rotate = 0
						
						pause_screen.start_hided_time = 0
						pause_screen.hided_time = get_game_time()
						
						set_fps(pause_screen.before_fps)
						hide_screen('pause')
		else:
			pause_screen.y = float(dtime - pause_screen.appearance_time) / pause_screen.appearance_time
	
	xpos   pause_screen.x
	ypos   pause_screen.y
	rotate pause_screen.rotate
	
	
	key 'ESCAPE' action pause_screen.close
	
	if not save_screenshotting:
		image 'images/bg/black.jpg':
			alpha 0.4
			pos   0
			size  1.0
			
		vbox:
			align (0.5, 0.5)
			spacing 5
			
			textbutton _('Continue')     style 'pause_screen_button' action pause_screen.close
			textbutton _('Load')         style 'pause_screen_button' action ShowMenu('load')
			textbutton _('Save')         style 'pause_screen_button' action ShowMenu('save')
			textbutton _('Preferences')  style 'pause_screen_button' action ShowMenu('preferences')
			textbutton _('Exit to menu') style 'pause_screen_button' action Function(start_mod, 'main_menu')

