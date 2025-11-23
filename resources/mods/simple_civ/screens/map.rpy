screen sc_map:
	$ sc_map.execute_bots()
	$ sc_map.check_update_forces()
	
	if sc_map.hovered:
		python:
			pos = sc_map.get_pos_under_mouse()
			if pos:
				sc_control.select_cell(pos[0], pos[1], True)
			else:
				sc_control.selected_cell = None
				sc_control.menu_items = []
		
		if not hotkeys.shift:
			key 'RETURN' action sc_control.show_menu(key_enter = True)
		key 'SPACE' action sc_control.show_menu
	
	image sc_map.bg:
		size 1.0
	
	key 'W' action sc_map.move( 0, -1)
	key 'A' action sc_map.move(-1,  0)
	key 'S' action sc_map.move( 0, +1)
	key 'D' action sc_map.move(+1,  0)
	
	key '-' action sc_map.add_zoom(-1)
	key '=' action sc_map.add_zoom(+1)
	
	key 'F5' action ToggleVariable('sc_map.mark_disabled')
	key 'F6' action ToggleVariable('sc_map.mark_spending')
	
	for obj in sc_map.to_draw():
		image obj['image']:
			xpos obj['xpos']
			ypos obj['ypos']
			size  obj['size']
			alpha obj['alpha']
	
	button:
		ground sc_black_bg
		hover  sc_black_bg
		size   1.0
		alpha  0.01
		mouse  False
		action    sc_control.show_menu
		alternate sc_control.show_menu
		
		hovered   'sc_map.hovered = True'
		unhovered 'sc_map.hovered = False'
