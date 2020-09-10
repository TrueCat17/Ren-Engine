init python:
	def add_tower(tower_type):
		global td_alarm_moneys
		if td_moneys < td_tower_types[tower_type]['cost']:
			td_alarm_moneys = time.time()
			return
		
		global selected_tower_type
		selected_tower_type = tower_type
		show_screen('add_tower')
	
	def update_tower_select_color():
		global td_fx, td_fy, cell_x, cell_y
		
		mouse_x, mouse_y = get_mouse()
		sw, sh = get_stage_width(), get_stage_height()
		fw, fh = td_map_w * td_cell_size * td_zoom, td_map_h * td_cell_size * td_zoom
		
		td_fx, td_fy = int((sw - fw) * 0.5), int(sh * 0.4 - fh * 0.5)
		cell_x = (mouse_x - td_fx) / td_cell_size / td_zoom
		cell_y = (mouse_y - td_fy) / td_cell_size / td_zoom
		
		if cell_x < 0 or cell_y < 0 or cell_x >= td_map_w or cell_y >= td_map_h:
			return None
		
		td_select_color = '#0F0'
		for x, y in td_path:
			if x == cell_x and y == cell_y:
				return '#F00'
		for tower in td_towers:
			if tower.x == cell_x and tower.y == cell_y:
				return '#F00'
		return td_select_color

screen add_tower:
	zorder 1
	modal True
	
	key 'ESCAPE' action hide_screen('add_tower')
	
	button:
		ground 'images/bg/black.jpg'
		hover  'images/bg/black.jpg'
		
		mouse False
		alpha 0.1
		size (1.0, 1.0)
		
		action hide_screen('add_tower')
	
	$ td_select_color = update_tower_select_color()
	if td_select_color == '#0F0':
		button:
			ground im.Rect(td_select_color)
			hover  im.Rect(td_select_color)
			
			xpos td_fx + cell_x * td_cell_size * td_zoom
			ypos td_fy + cell_y * td_cell_size * td_zoom
			size (td_cell_size * td_zoom, td_cell_size * td_zoom)
			
			action [make_tower(cell_x, cell_y), hide_screen('add_tower')]
	elif td_select_color == '#F00':
		image im.Rect(td_select_color):
			xpos td_fx + cell_x * td_cell_size * td_zoom
			ypos td_fy + cell_y * td_cell_size * td_zoom
			size (td_cell_size * td_zoom, td_cell_size * td_zoom)

