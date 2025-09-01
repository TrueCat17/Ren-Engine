init 1 python:
	tower_defence.adding_good_image = im.rect('#0F0')
	tower_defence.adding_bad_image  = im.rect('#F00')

init -1 python:
	def tower_defence__add_tower(tower_type):
		if tower_defence.moneys < tower_defence.tower_types[tower_type]['cost']:
			tower_defence.alarm_moneys = get_game_time()
			return
		
		tower_defence.selected_tower_type = tower_type
		show_screen('tower_defence_add_tower')
	
	def tower_defence__tower_adding_update():
		size = screen_tmp.size = tower_defence.cell_size * tower_defence.zoom
		
		mouse_x, mouse_y = get_mouse()
		sw, sh = get_stage_size()
		fw, fh = tower_defence.map_w * size, tower_defence.map_h * size # f = field
		
		fx = int((sw - fw) * 0.5)
		fy = int(sh * 0.4 - fh * 0.5)
		
		cell_x = screen_tmp.cell_x = (mouse_x - fx) // size
		cell_y = screen_tmp.cell_y = (mouse_y - fy) // size
		
		screen_tmp.xpos = fx + cell_x * size
		screen_tmp.ypos = fy + cell_y * size
		
		if cell_x < 0 or cell_y < 0 or cell_x >= tower_defence.map_w or cell_y >= tower_defence.map_h:
			screen_tmp.outside = True
			return
		
		screen_tmp.outside = False
		
		screen_tmp.good_place = True
		for x, y in tower_defence.path:
			if x == cell_x and y == cell_y:
				screen_tmp.good_place = False
				return
		for tower in tower_defence.towers:
			if int(tower.x) == cell_x and int(tower.y) == cell_y:
				screen_tmp.good_place = False
				return
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('tower_defence_add_tower')


screen tower_defence_add_tower:
	zorder 1
	modal True
	
	key 'ESCAPE' action hide_screen('tower_defence_add_tower')
	
	button:
		ground 'images/bg/black.jpg'
		hover  'images/bg/black.jpg'
		
		mouse False
		alpha 0.1
		size 1.0
		
		action hide_screen('tower_defence_add_tower')
	
	$ screen_tmp = SimpleObject()
	$ tower_defence.tower_adding_update()
	
	if not screen_tmp.outside:
		if screen_tmp.good_place:
			button:
				ground tower_defence.adding_good_image
				hover  tower_defence.adding_good_image
				
				xpos screen_tmp.xpos
				ypos screen_tmp.ypos
				size screen_tmp.size
				
				action [tower_defence.make_tower(screen_tmp.cell_x, screen_tmp.cell_y), hide_screen('tower_defence_add_tower')]
		
		else:
			image tower_defence.adding_bad_image:
				xpos screen_tmp.xpos
				ypos screen_tmp.ypos
				size screen_tmp.size
