init -101 python:
	inventory_background = im.rect('#FED')
	
	inventory_cell_usual    = 'images/gui/save_load/hover.png'
	inventory_cell_selected = 'images/gui/save_load/selected.png'
	inventory_cell_size = 50
	inventory_cell_text_size = 16
	inventory_cell_spacing = 10
	
	inventory_close_size = 25
	inventory_close = 'images/gui/menu/pause/close.png'
	
	inventory_selected = 0
	
	
	def inventory_get_cell_image(index):
		hover = inventory_cell_selected if inventory_selected == index else inventory_cell_usual
		hover = im.scale(hover, inventory_cell_size, inventory_cell_size)
		hover = get_back_with_color(hover)
		
		obj_name, obj_count = inventory[index]
		if not obj_name:
			return hover
		
		obj = location_objects[obj_name]
		main_frame = obj['animations'][None]
		image = main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext
		
		w, h = get_image_size(image)
		k = 1.5 * max(w, h) / inventory_cell_size
		w, h = int(w / k), int(h / k)
		x, y = (inventory_cell_size - w) / 2, (inventory_cell_size - h) / 2
		
		return im.composite((inventory_cell_size, inventory_cell_size),
			(x, y), im.scale(image, w, h),
			(0, 0), hover)
	
	def inventory_select(index):
		global inventory_selected
		inventory_selected = index
	
	def inventory_get_selected_object():
		obj_name, obj_count = inventory[inventory_selected]
		if not obj_name:
			return None
		
		obj = location_objects[obj_name]
		return obj
	
	def inventory_add_event(event, obj_name):
		rpg_events.add((event, obj_name))
		signals.send('inventory-' + event, obj_name)
	
	def inventory_remove_selected():
		element = inventory[inventory_selected]
		obj_name, obj_count = element
		
		inventory_add_event('remove', obj_name)
		
		obj = location_objects[obj_name]
		if obj['remove_to_location']:
			add_location_object(cur_location.name, me, obj_name)
		
		element[1] -= 1
		if element[1] == 0:
			inventory[inventory_selected] = ['', 0]
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('inventory')


screen inventory:
	modal True
	
	key 'ESCAPE' action HideScreen('inventory')
	
	button:
		ground 'images/bg/black.jpg'
		hover  'images/bg/black.jpg'
		mouse  False
		alpha  0.3
		size   1.0
		action HideScreen('inventory')
	
	image inventory_background:
		size (inventory_xsize, inventory_ysize)
		align (0.5, 0.5)
		
		vbox:
			spacing inventory_cell_spacing
			xalign 0.5
			ypos inventory_cell_spacing
			
			for i in xrange(inventory_full_rows + 1):
				hbox:
					spacing inventory_cell_spacing
					xalign 0.5
					
					for j in xrange(inventory_row if i != inventory_full_rows else inventory_last_row):
						$ index = i * inventory_row + j
						
						vbox:
							button:
								size inventory_cell_size
								
								ground inventory_get_cell_image(index)
								action inventory_select(index)
							
							null ysize 2
							
							$ name, count = inventory[index]
							text (str(count) if name else ' '):
								xalign 0.5
								text_size inventory_cell_text_size
								color 0xFFFFFF
								outlinecolor 0x000000
			
			$ inventory_selected_object = inventory_get_selected_object()
			if inventory_selected_object:
				hbox:
					spacing 10
					xalign 0.5
					
					textbutton _('Use'):
						action inventory_add_event('using', inventory_selected_object.name)
						
						color 0x00FF00
						text_size 20
					textbutton _('Lay out' if inventory_selected_object['remove_to_location'] else 'Throw away'):
						action inventory_remove_selected
						
						color 0xFF0000
						text_size 20
		
		button:
			pos    (inventory_xsize + 10 + inventory_close_size / 2, -10 - inventory_close_size / 2)
			anchor 0.5
			size   inventory_close_size
			
			ground inventory_close
			action HideScreen('inventory')

