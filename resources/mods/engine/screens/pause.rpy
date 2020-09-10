init python:
	
	pause_showed_time = 0
	pause_start_hided_time = 0
	pause_hided_time = 0
	
	pause_before_fps = None
	pause_fps = 60
	
	
	pause_x, pause_y = 0, 0
	pause_rotate = 0
	pause_hide_rotate = 15
	pause_rotate_time = 0.2
	pause_disappearance_time = 0.2
	pause_appearance_time = 0.4
	
	
	pause_inited = False
	def init_pause():
		global pause_inited
		global pause_button, pause_button_hover, pause_button_selected
		
		pause_inited = True
		
		pause_button          = im.MatrixColor(gui + 'menu/pause/button.png', im.matrix.invert() * im.matrix.tint(  0, 0.5, 1))
		pause_button_hover    = im.MatrixColor(gui + 'menu/pause/button.png', im.matrix.invert() * im.matrix.tint(0.4, 0.8, 1))
		pause_button_selected = im.MatrixColor(gui + 'menu/pause/button.png', im.matrix.invert() * im.matrix.tint(  1, 0.5, 0))
		
		pause_button          = get_back_with_color(pause_button,          alpha = 0.5)
		pause_button_hover    = get_back_with_color(pause_button_hover,    alpha = 0.2)
		pause_button_selected = get_back_with_color(pause_button_selected, alpha = 0.5)
		
		style.pause_button = Style(style.textbutton)
		style.pause_button.ground = pause_button
		style.pause_button.hover  = pause_button_hover
		style.pause_button.xsize = 0.2
		style.pause_button.ysize = 0.1
		style.pause_button.text_size = 20
	
	
	def show_pause():
		global pause_showed_time, pause_start_hided_time, pause_before_fps
		if not has_screen('pause') and time.time() - pause_hided_time > pause_rotate_time + pause_disappearance_time:
			pause_showed_time = time.time()
			pause_start_hided_time = 0
			pause_before_fps = get_fps()
			set_fps(pause_fps)
			show_screen('pause')
	def pause_close_func():
		global pause_start_hided_time
		a = time.time() - pause_start_hided_time > pause_rotate_time + pause_disappearance_time
		b = time.time() - pause_showed_time > pause_appearance_time
		if a and b:
			pause_start_hided_time = time.time()


screen pause:
	zorder 10000
	modal  True
	
	if not has_screen('choose_menu'):
		use hotkeys
	
	python:
		if not pause_inited:
			init_pause()
		
		if time.time() - pause_showed_time > pause_appearance_time:
			pause_y = 0
			if pause_start_hided_time:
				if time.time() - pause_start_hided_time < pause_rotate_time:
					pause_rotate = pause_hide_rotate * (time.time() - pause_start_hided_time) / pause_rotate_time
				else:
					pause_rotate = pause_hide_rotate
					pause_x = pause_y = (time.time() - pause_start_hided_time - pause_rotate_time) / pause_disappearance_time
					
					if pause_x >= 1:
						pause_x, pause_y = 0, 0
						pause_rotate = 0
						
						pause_start_hided_time = 0
						pause_hided_time = time.time()
						
						set_fps(pause_before_fps)
						hide_screen('pause')
		else:
			pause_y = float(time.time() - pause_showed_time - pause_appearance_time) / pause_appearance_time
	
	xpos   pause_x
	ypos   pause_y
	rotate pause_rotate
	
	
	key 'ESCAPE' action pause_close_func
	
	if not save_screenshotting:
		image 'images/bg/black.jpg':
			alpha 0.4
			pos  (0, 0)
			size (1.0, 1.0)
			
		vbox:
			align (0.5, 0.5)
			spacing 5
			
			textbutton 'Продолжить'   style pause_button action pause_close_func
			textbutton 'Загрузить'    style pause_button action ShowMenu('load')
			textbutton 'Сохранить'    style pause_button action ShowMenu('save')
			textbutton 'Настройки'    style pause_button action ShowMenu('settings')
			textbutton 'Выход в меню' style pause_button action Function(start_mod, 'main_menu')

