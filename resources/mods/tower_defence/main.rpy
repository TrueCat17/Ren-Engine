init python:
	
	def tower_defence__init():
		tower_defence.init_map()
		
		tower_defence.tanks.clear()
		tower_defence.towers.clear()
		tower_defence.bullets.clear()
		
		tower_defence.moneys = 100
		tower_defence.hp = 100
		
		tower_defence.last_update_time = 0
		tower_defence.to_update_time = 0
		
		set_level(0)
	
	def set_level(level):
		if level == len(tower_defence.levels):
			tower_defence.result = '{color=#0F0}' + _('WIN')
			return
		
		tower_defence.level = level
		tower_defence.pause = True
		tower_defence.result = ''
		tower_defence.tanks_created = 0
		tower_defence.to_update_time = 0
		tower_defence.bullets = []
	
	def tower_defence__update():
		xzoom = get_stage_width()        / tower_defence.map_w / tower_defence.cell_size
		yzoom = get_stage_height() * 0.7 / tower_defence.map_h / tower_defence.cell_size
		tower_defence.zoom = max(int(min(xzoom, yzoom)), 1)
		
		if tower_defence.hp == 0:
			tower_defence.result = '{color=#F00}' + _('FAIL')
		
		if tower_defence.pause or tower_defence.hp == 0:
			tower_defence.last_update_time = 0
			return
		
		tower_defence.to_update_time += get_last_tick()
		while tower_defence.to_update_time > tower_defence.frame_time:
			tower_defence.to_update_time -= tower_defence.frame_time
			
			tower_defence.check_tank_adding()
			tower_defence.update_tanks()
			tower_defence.update_towers()
			tower_defence.update_bullets()
		
		if not tower_defence.tanks and tower_defence.tanks_created == tower_defence.levels[tower_defence.level][1]:
			set_level(tower_defence.level + 1)


init python:
	tower_defence = SimpleObject()
	build_object('tower_defence')
	
	tower_defence.images = 'mods/' + get_current_mod() + '/images/'
	
	tower_defence.bg = im.rect('#FB4')
	tower_defence.panel_bg  = im.rect('#555')
	tower_defence.result_bg = im.rect('#333')
	
	tower_defence.frame_time = 1 / 60
	
	tower_defence.zoom = 1
	
	tower_defence.result = ''
	tower_defence.alarm_moneys = -10

init 10 python:
	tower_defence.init()
	show_screen('tower_defence')


screen tower_defence:
	key 'ESCAPE' action 'tower_defence.pause = True'
	
	image tower_defence.bg:
		size 1.0
	
	$ tower_defence.update()
	$ screen_tmp = SimpleObject()
	
	image tower_defence.field:
		anchor (0.5, 0.5)
		pos    (0.5, 0.4)
		xsize tower_defence.map_w * tower_defence.cell_size
		ysize tower_defence.map_h * tower_defence.cell_size
		zoom  tower_defence.zoom
		
		for obj in tower_defence.tanks:
			image obj.image:
				anchor (0.5, 0.5)
				xpos obj.x * tower_defence.cell_size
				ypos obj.y * tower_defence.cell_size
				size   obj.size
				rotate obj.rotation
		
		for obj in tower_defence.bullets:
			image obj.image:
				anchor (0.5, 0.5)
				xpos obj.x * tower_defence.cell_size
				ypos obj.y * tower_defence.cell_size
				size obj.size
		
		$ screen_tmp.to_delete_tower = None
		for obj in tower_defence.towers:
			button:
				corner_sizes 0
				ground obj.image
				action 'screen_tmp.to_delete_tower = obj'
				
				anchor (0.5, 0.5)
				xpos obj.x * tower_defence.cell_size
				ypos obj.y * tower_defence.cell_size
				
				size   obj.size
				rotate obj.rotation
		python:
			if screen_tmp.to_delete_tower:
				tower_defence.moneys += screen_tmp.to_delete_tower.cost // 2
				tower_defence.towers.remove(screen_tmp.to_delete_tower)
		
		image tower_defence.bg:
			xpos 0
			xanchor 1.0
			xsize tower_defence.cell_size
			ysize 1.0
	
	if tower_defence.result:
		image tower_defence.result_bg:
			xalign 0.5
			ypos 10
			size (0.25, 0.15)
			
			text tower_defence.result:
				align 0.5
				text_size 0.07
	
	image tower_defence.panel_bg:
		yalign 1.0
		size (1.0, 0.2)
		
		vbox:
			spacing 0.01
			xpos 0.03
			yalign 0.5
			
			textbutton _('Restart'):
				style 'tower_defence_button'
				action tower_defence.init
			
			textbutton _('Play' if tower_defence.pause else 'Pause'):
				style 'tower_defence_button'
				action ToggleVariable('tower_defence.pause')
		
		hbox:
			spacing 0.01
			align 0.5
			
			for tower_type, tower_props in tower_defence.tower_types.items():
				vbox:
					spacing 0.01
					
					button:
						corner_sizes 0
						ground tower_props['image']
						size tower_defence.cell_size * min(tower_defence.zoom, 2)
						xalign 0.5
						action tower_defence.add_tower(tower_type)
					
					text str(tower_props['cost']):
						style 'tower_defence_text'
						color '#F80'
						xalign 0.5
		
		hbox:
			spacing 0.01
			
			xpos   0.97
			xanchor 1.0
			yalign 0.5
			
			$ screen_tmp.money_color = '#F00' if get_game_time() - tower_defence.alarm_moneys < 0.3 else '#FF0'
			
			vbox:
				spacing 0.01
				
				text (_('Level') + ':'):
					style 'tower_defence_text'
					color '#F80'
				text (_('Money') + ':'):
					style 'tower_defence_text'
					color screen_tmp.money_color
				text (_('HP') + ':'):
					style 'tower_defence_text'
					color '#0F0'
			
			vbox:
				spacing 0.01
				
				text str(tower_defence.level + 1):
					style 'tower_defence_text'
					color '#F80'
				text str(tower_defence.moneys):
					style 'tower_defence_text'
					color screen_tmp.money_color
				text str(tower_defence.hp):
					style 'tower_defence_text'
					color '#0F0'
