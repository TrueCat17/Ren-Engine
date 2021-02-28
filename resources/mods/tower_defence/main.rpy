init python:
	show_screen('tower_defence')
	
	td_frame_time = 1.0 / 60
	
	td_zoom = 1
	
	td_result = ''
	td_alarm_moneys = 0
	
	
	def td_init():
		global td_tanks, td_towers, td_bullets
		td_tanks = []
		td_towers = []
		td_bullets = []
		
		global td_moneys, td_hp
		td_moneys = 100
		td_hp = 100
		
		global td_last_update_time, td_to_update_time
		td_last_update_time = 0
		td_to_update_time = 0
		
		set_level(0)
	
	def set_level(level):
		global td_result
		if level == len(td_levels):
			td_result = '{color=#00FF00}' + _('WIN')
			return
		
		global td_level, td_pause, td_tanks_created, td_to_update_time, td_bullets
		td_level = level
		td_pause = True
		td_tanks_created = 0
		td_to_update_time = 0
		td_bullets = []
	
	def td_update():
		global td_last_update_time, td_to_update_time, td_zoom
		td_zoom = min(get_stage_width() / td_map_w / td_cell_size, int(get_stage_height() * 0.7 / td_map_h / td_cell_size))
		if td_zoom < 1:
			td_zoom = 1
		
		global td_result
		td_result = '' if td_hp else '{color=#FF0000}' + _('FAIL')
		
		if td_pause or td_hp == 0:
			td_last_update_time = 0
			return
		
		td_to_update_time += get_last_tick()
		while td_to_update_time > td_frame_time:
			td_to_update_time -= td_frame_time
			
			td_check_tank_add()
			td_update_tanks()
			td_update_towers()
			td_update_bullets()
		
		if not td_tanks and td_tanks_created == td_levels[td_level][1]:
			set_level(td_level + 1)
	
	td_init()


screen tower_defence:
	key 'ESCAPE' action SetVariable('td_pause', True)
	
	image im.Rect('#FB4'):
		size (1.0, 0.8)
	
	$ td_update()
	
	image td_back:
		anchor (0.5, 0.5)
		pos    (0.5, 0.4)
		size   (td_map_w * td_cell_size, td_map_h * td_cell_size)
		zoom    td_zoom
		
		for obj in td_tanks:
			image obj.image:
				anchor (0.5, 0.5)
				xpos int((obj.x + 0.5) * td_cell_size)
				ypos int((obj.y + 0.5) * td_cell_size)
				size   obj.size 
				rotate obj.rotation
		
		for obj in td_bullets:
			image obj.image:
				anchor (0.5, 0.5)
				xpos int((obj.x + 0.5) * td_cell_size)
				ypos int((obj.y + 0.5) * td_cell_size)
				
				size obj.size
		
		$ td_to_delete_tower = None
		for obj in td_towers:
			button:
				ground obj.image
				action SetVariable('td_to_delete_tower', obj)
				
				anchor (0.5, 0.5)
				xpos int((obj.x + 0.5) * td_cell_size)
				ypos int((obj.y + 0.5) * td_cell_size)
				
				size   obj.size
				rotate obj.rotation
		python:
			if td_to_delete_tower is not None:
				td_moneys += td_tower_types[td_to_delete_tower.tower_type]['cost'] / 2
				td_towers.remove(td_to_delete_tower)
		
		image im.Rect('#FB4'):
			xpos 0
			xanchor 1.0
			size (td_cell_size, 1.0)
	
	if td_result:
		image im.Rect('333'):
			xalign 0.5
			ypos 10
			size (200, 32)
			
			text td_result:
				align (0.5, 0.5)
				text_size 30
	
	image im.Rect('#555'):
		ypos 0.8
		size (1.0, 0.2)
		
		hbox:
			spacing 10
			align (0.05, 0.5)
			
			textbutton _('Restart'):
				xsize 100
				yalign 0.5
				action td_init
			
			textbutton _('Play' if td_pause else 'Pause'):
				xsize 100
				yalign 0.5
				action SetVariable('td_pause', not td_pause)
		
		hbox:
			spacing 10
			align (0.5, 0.5)
			
			for tower_type in ('usual', 'fast', 'frost'):
				vbox:
					button:
						ground td_tower_types[tower_type]['image']
						size (td_cell_size, td_cell_size)
						action add_tower(tower_type)
					text str(td_tower_types[tower_type]['cost']):
						xalign 0.5
						text_size 20
						color 0
		
		hbox:
			spacing 10
			align (0.95, 0.5)
			
			text (_('Level') + ': ' + str(td_level + 1)):
				text_size 18
				color 0
			text (_('Money') + ': ' + str(td_moneys)):
				text_size 18
				color 0xFF0000 if get_game_time() - td_alarm_moneys < 0.3 else 0xFFFF00
			text (_('HP') + ': ' + str(td_hp)):
				text_size 18
				color 0x00FF00

