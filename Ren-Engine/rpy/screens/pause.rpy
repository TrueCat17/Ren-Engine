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
	
	# in seconds:
	pause_screen.disappearance_time = [0.2, 0.2]
	pause_screen.appearance_time = 0.4
	
	pause_screen.pre_start_props = {
		'xpos': 0.0,
		'xanchor': 0.0,
		'ypos': 0.0,
		'yanchor': 1.0,
		'alpha': 1,
		'rotate': 0,
	}
	pause_screen.start_props = {
		'xpos': 0.0,
		'xanchor': 0.0,
		'ypos': 1.0,
		'yanchor': 1.0,
		'alpha': 1,
		'rotate': 0,
	}
	
	pause_screen.end_props = [
		{
			'xpos': 0.0,
			'xanchor': 0.0,
			'ypos': 1.0,
			'yanchor': 1.0,
			'alpha': 1,
			'rotate': 15,
		},
		{
			'xpos': 1.0,
			'xanchor': 0.0,
			'ypos': 2.0 + _sin(15),
			'yanchor': 1.0,
			'alpha': 1,
			'rotate': 15,
		},
	]
	
	
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
	def pause_screen__show(ignore_checks = False):
		if not ignore_checks and pause_screen.disable: return
		if has_screen('pause'): return
		
		smooth_changes.start(
			'pause',
			pause_screen.pre_start_props, pause_screen.start_props, pause_screen.cur_props,
			pause_screen.appearance_time, None,
		)
		
		pause_screen.before_fps = get_fps()
		set_fps(pause_screen.fps)
		show_screen('pause')
	
	def pause_screen__close():
		if not smooth_changes.ended('pause'): return
		
		smooth_changes.start(
			'pause',
			pause_screen.start_props, pause_screen.end_props, pause_screen.cur_props,
			pause_screen.disappearance_time, pause_screen.real_close,
		)
	
	def pause_screen__real_close():
		hide_screen('pause')
		set_fps(pause_screen.before_fps)
	
	
	build_object('pause_screen')
	pause_screen.cur_props = {}
	pause_screen.before_fps = None
	pause_screen.fps = 60


screen pause:
	zorder 10000
	modal  True
	save   False
	
	$ smooth_changes.update('pause')
	$ screen_tmp = SimpleObject()
	$ screen_tmp.__dict__.update(pause_screen.cur_props)
	xpos   screen_tmp.xpos
	ypos   screen_tmp.ypos
	xanchor screen_tmp.xanchor
	yanchor screen_tmp.yanchor
	alpha  screen_tmp.alpha
	rotate screen_tmp.rotate
	
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
