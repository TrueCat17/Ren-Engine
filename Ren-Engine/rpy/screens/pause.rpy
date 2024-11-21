init -100:
	style pause_screen_button is textbutton:
		corner_sizes 0
		ground 'images/gui/menu/pause/btn_ground.png'
		hover  'images/gui/menu/pause/btn_hover.png'
		size     (0.2, 0.1)
		text_size 0.03
		text_size_min 14

init -100 python:
	pause_screen.disable = False
	
	pause_screen.rotate_before_hiding = 15 # in degrees
	# in seconds:
	pause_screen.rotate_time = 0.2
	pause_screen.disappearance_time = 0.2
	pause_screen.appearance_time = 0.4
	
	# item: [text, action]
	#  text will be translated
	pause_screen.items = [
		['Continue', pause_screen.close],
		['Load', ShowMenu('load')],
		['Save', ShowMenu('save')],
		['Preferences', ShowMenu('preferences')],
		['Exit to menu', Function(start_mod, 'main_menu')],
	]
	# adding buttons:
	#  pause_screen.items.insert(-1, ['Galery', ShowMenu('gallery')])
	#  pause_screen.items.append(['Exit', exit_from_game])


init -1000 python:
	def pause_screen__update():
		if pause_screen.showed_time:
			dtime = get_game_time() - pause_screen.showed_time
		else:
			dtime = 0
			signals.add('enter_frame', Exec('pause_screen.showed_time = get_game_time()'), times=1)
			# set <showed_time> in the next frame, because mb freezing (because loading image(s) for pause menu)
		
		if dtime > pause_screen.appearance_time:
			pause_screen.y = 0
			if pause_screen.hided_time:
				dtime = get_game_time() - pause_screen.hided_time
				if dtime < pause_screen.rotate_time:
					pause_screen.rotate = pause_screen.rotate_before_hiding * dtime / pause_screen.rotate_time
				else:
					pause_screen.rotate = pause_screen.rotate_before_hiding
					pause_screen.x = pause_screen.y = (dtime - pause_screen.rotate_time) / pause_screen.disappearance_time
					
					if pause_screen.x >= 1:
						pause_screen.x, pause_screen.y = 0, 0
						pause_screen.rotate = 0
						pause_screen.hided_time = 0
						
						set_fps(pause_screen.before_fps)
						hide_screen('pause')
		else:
			pause_screen.y = float(dtime - pause_screen.appearance_time) / pause_screen.appearance_time
	
	
	def pause_screen__show(ignore_checks = False):
		if not ignore_checks and pause_screen.disable: return
		if has_screen('pause'): return
		
		pause_screen.showed_time = 0
		pause_screen.hided_time = 0
		pause_screen.before_fps = get_fps()
		set_fps(pause_screen.fps)
		show_screen('pause')
	
	def pause_screen__close():
		if get_game_time() - pause_screen.hided_time < pause_screen.rotate_time + pause_screen.disappearance_time: return
		if not pause_screen.showed_time: return
		if get_game_time() - pause_screen.showed_time < pause_screen.appearance_time: return
		
		pause_screen.hided_time = get_game_time()
	
	
	build_object('pause_screen')
	pause_screen.showed_time = 0
	pause_screen.hided_time = 0
	pause_screen.before_fps = None
	pause_screen.fps = 60
	
	pause_screen.x, pause_screen.y = 0, 0
	pause_screen.rotate = 0


screen pause:
	zorder 10000
	modal  True
	save   False
	
	$ pause_screen.update()
	xpos   pause_screen.x
	ypos   pause_screen.y
	rotate pause_screen.rotate
	
	key 'ESCAPE' action pause_screen.close
	
	
	button:
		ground 'images/bg/black.jpg'
		hover  'images/bg/black.jpg'
		
		size  1.0
		alpha 0.4
		mouse False
		alternate pause_screen.close
	
	vbox:
		align (0.5, 0.5)
		spacing 5
		
		for text, action in pause_screen.items:
			textbutton _(text):
				style 'pause_screen_button'
				action action

