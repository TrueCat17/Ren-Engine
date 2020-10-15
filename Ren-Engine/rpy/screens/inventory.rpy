init python:
	inventory_row = min(10, inventory_size)
	inventory_full_rows = int(inventory_size / inventory_row)
	inventory_last_row = inventory_size % inventory_row
	
	inventory_visible = False
	
	inventory_background = im.Rect('#FED')
	inventory_cell_usual = gui + 'save_load/hover.png'
	inventory_cell_selected = gui + 'save_load/selected.png'
	inventory_cell_size = 50
	inventory_cell_spacing = 10
	
	inventory_xsize = (inventory_cell_size + inventory_cell_spacing) * inventory_row + inventory_cell_spacing
	inventory_ysize = (inventory_cell_size + inventory_cell_spacing) * (inventory_full_rows + (1 if inventory_last_row else 0)) + inventory_cell_spacing + 100
	
	inventory_selected = [0, 0]
	
	
	inventory_close_size = 25
	inventory_inited = False
	def init_inventory():
		global inventory_inited, inventory_close
		inventory_inited = True
		inventory_close = get_back_with_color(gui + 'menu/pause/close.png')
	
	
	def inventory_get_cell_image(row, column):
		hover = inventory_cell_selected if inventory_selected == [row, column] else inventory_cell_usual
		hover = im.Scale(hover, inventory_cell_size, inventory_cell_size)
		hover = get_back_with_color(hover)
		
		index = row * inventory_row + column
		element = inventory[index]
		if element is None:
			return hover
		
		obj_name = element[0]
		obj = location_objects[obj_name]
		main_frame = obj['animations'][None]
		image = main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext
		
		w, h = get_image_size(image)
		k = 1.5 * max(w, h) / inventory_cell_size
		w, h = int(w / k), int(h / k)
		x, y = (inventory_cell_size - w) / 2, (inventory_cell_size - h) / 2
		
		return im.Composite((inventory_cell_size, inventory_cell_size),
			(x, y), im.Scale(image, w, h),
			(0, 0), hover)
	
	def inventory_select(row, column):
		global inventory_selected
		inventory_selected = [row, column]
	
	def inventory_get_selected_object():
		row, column = inventory_selected
		index = row * inventory_row + column
		element = inventory[index]
		if element is None:
			return None
		
		obj = location_objects[element[0]]
		return obj
	
	inventory_action = None
	inventory_action_object = None
	def inventory_do_action(action, obj = None):
		global inventory_action, inventory_action_object
		inventory_action = action
		inventory_action_object = obj or inventory_get_selected_object()
	
	def inventory_remove_selected():
		row, column = inventory_selected
		index = row * inventory_row + column
		element = inventory[index]
		if element is None:
			return
		
		inventory_do_action('remove')
		
		obj_name = element[0]
		obj = location_objects[obj_name]
		if obj['remove_to_location']:
			add_location_object(cur_location.name, me, obj_name)
		
		element[1] -= 1
		if element[1] == 0:
			inventory[index] = None


screen inventory:
	key 'I' action SetVariable('inventory_visible', not(inventory_visible))
	
	if inventory_visible and draw_location:
		image inventory_background:
			size (inventory_xsize, inventory_ysize)
			align (0.5, 0.5)
			
			vbox:
				spacing inventory_cell_spacing
				xalign 0.5
				ypos inventory_cell_spacing
				
				for i in xrange(inventory_full_rows):
					hbox:
						spacing inventory_cell_spacing
						
						for j in xrange(inventory_row):
							button:
								size (inventory_cell_size, inventory_cell_size)
								
								ground inventory_get_cell_image(i, j)
								action inventory_select(i, j)
				if inventory_last_row:
					for j in xrange(inventory_last_row):
						button:
							size (inventory_cell_size, inventory_cell_size)
							
							ground inventory_get_cell_image(inventory_full_rows, j)
							action inventory_select(inventory_full_rows, j)
				
				null ysize 30
				
				$ inventory_selected_object = inventory_get_selected_object()
				if inventory_selected_object:
					hbox:
						spacing 10
						xalign 0.5
						
						textbutton 'Использовать':
							action inventory_do_action('using')
							
							color 0x00FF00
							text_size 20
						textbutton ('Выложить' if inventory_selected_object['remove_to_location'] else 'Выбросить'):
							action inventory_remove_selected
							
							color 0xFF0000
							text_size 20
			
			if not inventory_inited:
				$ init_inventory()
			button:
				pos    (inventory_xsize + 10 + inventory_close_size / 2, -10 - inventory_close_size / 2)
				anchor (0.5, 0.5)
				size   (inventory_close_size, inventory_close_size)
				
				ground inventory_close
				action SetVariable('inventory_visible', False)

