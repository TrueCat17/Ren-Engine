init 100 python:
	set_can_mouse_hide(False)
	config.has_autosave = False
	
	start_screens = ['hotkeys', 'all_locations', 'menu', 'points_list']
	
	gate_right = get_location_objects("enter", "before_gates", "gate_right", 1)[0]
	gate_right.start_animation("open")
	gate_right.update()
