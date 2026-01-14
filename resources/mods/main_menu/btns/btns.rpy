init python:
	
	def btns__show():
		btn_actions = {
			'demos':    demos.show,
			'tutorial': Function(start_mod, 'tutorial'),
			'clicker':  clicker.show,
		}
		btn_coords = {
			'demos':    (   0, 433),
			'tutorial': (1377, 533),
			'clicker':  (1915, 676),
		}
		
		hover_matrix = im.matrix.invert() * im.matrix.tint(0, 0.5, 1.0)
		
		btns.props = {}
		for name in btn_coords:
			btn = btns.props[name] = SimpleObject()
			btn.name = name
			
			btn.action = btn_actions[name]
			btn.ground = 'mods/main_menu/btns/%s.webp' % name
			btn.hover  = im.matrix_color(btn.ground, hover_matrix)
			
			x, y = btn_coords[name]
			btn.pos  = x / 2560, y / 1440
			
			w, h = get_image_size(btn.hover)
			btn.size = w / 2560, h / 1440
		
		show_screen('btns')
	
	
	def btns__get():
		btns.spent += get_last_tick()
		
		backlight_time = 0.5
		backlight_cycle_time = 10
		backlight_work_time = backlight_cycle_time * 4
		
		time_shift = backlight_cycle_time / 2
		
		dtime = (btns.spent + time_shift) % backlight_cycle_time
		index = int(dtime / backlight_time)
		
		for i, btn in enumerate(btns.props.values()):
			backlight = (index == i * 2)
			
			selected = btns.hovered_name == btn.name
			if backlight and not selected and btns.spent < backlight_work_time:
				btn.selected = True
				btn.alpha = 0.33
			else:
				btn.selected = selected
				btn.alpha = 0.1 if selected else 0.01
		
		return btns.props.values()
	
	
	btns = SimpleObject()
	build_object('btns')
	
	btns.spent = 0
	btns.hovered_name = None


screen btns:
	alpha 0 if has_screen('clicker') else 1
	
	for btn in btns.get():
		button:
			ground btn.ground
			hover  btn.hover
			
			pos  btn.pos
			size btn.size
			corner_sizes 0
			
			selected btn.selected
			alpha    btn.alpha
			
			hovered   'btns.hovered_name = btn.name'
			unhovered 'if btns.hovered_name == btn.name: btns.hovered_name = None'
			
			action btn.action
