init -101 python:
	gui.inventory_fog_bg = im.rect('#000')
	gui.inventory_fog_alpha = 0.3
	
	gui.inventory_bg = im.rect('#FED')
	
	gui.inventory_xalign = 0.5
	gui.inventory_yalign = 0.5
	
	gui.inventory_spacing = 20
	gui.inventory_edge_spacing = 50
	gui.inventory_width_min = 450
	gui.inventory_width_max = 0.85
	gui.inventory_width = None # None = auto
	gui.inventory_height = None # None = auto
	gui.inventory_height_max = 0.9
	gui.inventory_xcount = 8 # cells in row
	
	gui.inventory_separator_bg = im.rect('#000')
	gui.inventory_separator_width = 0.2
	gui.inventory_separator_height = 1
	
	
	gui.inventory_cell_xalign = 0.5
	gui.inventory_cell_xspacing = 10
	gui.inventory_cell_yspacing = 10
	
	gui.inventory_cell_usual_over    = 'images/gui/save_load/hover.png'
	gui.inventory_cell_selected_over = 'images/gui/save_load/selected.png'
	gui.inventory_cell_width = 1.0 / 20
	gui.inventory_cell_height = None # None = auto (width)
	
	gui.inventory_cell_image_width = 0.75 # float - relatively cell width
	gui.inventory_cell_image_height = None # None = auto (width)
	
	gui.inventory_cell_text_ypos = 2 # spacing between image and text
	gui.inventory_cell_text_font = 'Calibri'
	gui.inventory_cell_text_color = 0xFFFFFF
	gui.inventory_cell_text_outlinecolor = 0x000000 # None = disabled
	gui.inventory_cell_text_size = 16
	gui.inventory_cell_text_xalign = 0.5
	
	
	gui.inventory_button_spacing = 10
	gui.inventory_button_width = 180
	gui.inventory_button_height = 30
	gui.inventory_button_ground = style.textbutton.ground
	gui.inventory_button_hover  = style.textbutton.hover
	gui.inventory_button_text_font = 'Calibri'
	gui.inventory_button_text_size = 20
	gui.inventory_button_text_xalign = 0.5
	
	gui.inventory_use_button_text_color = 0x00FF00
	gui.inventory_use_button_text_outlinecolor = None # None = disabled
	gui.inventory_throw_button_text_color = 0xFF0000
	gui.inventory_throw_button_text_outlinecolor = None
	
	
	def inventory__get_cell_image_size(image):
		w, h = get_image_size(image)
		k = min(inventory.cell_image_xsize / float(w), inventory.cell_image_ysize / float(h))
		return int(w * k), int(h * k)
	
	def inventory__get_cell_image(index, is_selected):
		obj_name, obj_count = inventory.inv[index]
		
		cache = inventory__get_cell_image.__dict__
		key = (obj_name, inventory.cell_xsize, inventory.cell_ysize, is_selected)
		if key in cache:
			return cache[key]
		
		hover = inventory['cell_selected_over' if is_selected else 'cell_usual_over']
		hover = im.scale(hover, inventory.cell_xsize, inventory.cell_ysize)
		hover = get_back_with_color(hover)
		
		if not obj_name:
			cache[key] = hover
			return hover
		
		obj = location_objects[obj_name]
		main_frame = obj['animations'][None]
		image = main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext
		
		w, h = inventory.get_cell_image_size(image)
		x, y = (inventory.cell_xsize - w) / 2, (inventory.cell_ysize - h) / 2
		
		res = im.composite((inventory.cell_xsize, inventory.cell_ysize),
		          (x, y), im.scale(image, w, h),
		          (0, 0), hover)
		
		cache[key] = res
		return res
	
	def inventory__select(index):
		inventory.selected_cell = index
	
	def inventory__get_selected_object():
		obj_name, obj_count = inventory.inv[inventory.selected_cell]
		if obj_name:
			return location_objects[obj_name]
		return None
	
	def inventory__add_event(event, obj_name):
		rpg_events.add((event, obj_name))
		signals.send('inventory-' + event, obj_name)
	
	def inventory__remove_selected():
		element = inventory.inv[inventory.selected_cell]
		obj_name = element[0]
		
		obj = location_objects[obj_name]
		if obj['remove_to_location']:
			r = inventory.throw_radius
			dx, dy = random.randint(-r, r), random.randint(-r, r)
			add_location_object(cur_location.name, {'x': me.x + dx, 'y': me.y + dy}, obj_name)
		
		element[1] -= 1
		if element[1] == 0:
			element[0] = ''
		
		inventory.add_event('remove', obj_name)
	
	
	def inventory__show(first = None, second = None):
		first = first or me
		inv1 = first if type(first) is list else first.inventory
		second = second or get_near_location_object_with_inventory()
		inv2 = second if type(second) is list or second is None else second.inventory
		inventory.inv1, inventory.inv2 = inv1, inv2
		
		inventory.showed_time = get_game_time()
		inventory.hided_time  = None
		show_screen('inventory')
	
	def inventory__close():
		inventory.hided_time = get_game_time()
	
	
	# dnd = drag-and-drop
	
	def inventory__on_hovered(index):
		if inventory.current == '1':
			inventory.hovered_time = get_game_time()
		inventory.dnd_inv = inventory.current
		inventory.dnd_index = index
	
	def inventory__on_unhovered(index):
		if inventory.dnd_inv == inventory.current and inventory.dnd_index == index:
			inventory.hovered_time = None
			inventory.dnd_inv = None
			inventory.dnd_index = None
	
	def inventory__on_left_down(index):
		inventory.on_hovered(index)
		
		inv = inventory['inv' + inventory.dnd_inv]
		obj_name, obj_count = inv[index]
		
		if not inventory.dnd_obj_name:
			inventory.dnd_left_start(inv, index, obj_name, obj_count)
		else:
			inventory.dnd_left_end(inv, index, obj_name, obj_count)
	
	def inventory__dnd_left_start(inv, index, obj_name, obj_count):
		inv[index] = ['', 0]
		inventory.dnd_obj_name = obj_name
		inventory.dnd_obj_count = obj_count
		inventory.dnd_mouse_button = 'left'
		inventory.dnd_from = inv, index
	
	def inventory__dnd_left_end(inv, index, obj_name, obj_count):
		if obj_name == inventory.dnd_obj_name:
			max_count = location_objects[obj_name]['max_in_inventory_cell']
			d = min(max_count - obj_count, inventory.dnd_obj_count)
			if d > 0:
				obj_count += d
				inv[index][1] = obj_count
				inventory.dnd_obj_count -= d
			if inventory.dnd_obj_count == 0:
				inventory.dnd_obj_name = None
		else:
			inv[index] = [inventory.dnd_obj_name, inventory.dnd_obj_count]
			inventory.dnd_obj_name, inventory.dnd_obj_count = obj_name, obj_count
		
		if not inventory.dnd_obj_name:
			inventory.dnd_mouse_button = None
		if inventory.current == '1':
			inventory.select(index)
	
	
	def inventory__on_right_down(index):
		inventory.on_hovered(index)
		
		inv = inventory['inv' + inventory.dnd_inv]
		obj_name, obj_count = inv[index]
		
		if not inventory.dnd_obj_name:
			inventory.dnd_right_start(inv, index, obj_name, obj_count)
		else:
			inventory.dnd_right_end(inv, index, obj_name, obj_count)
	
	def inventory__dnd_right_start(inv, index, obj_name, obj_count):
		take_count = int(math.ceil(obj_count / 2.0))
		if take_count == obj_count:
			inv[index] = ['', 0]
		else:
			inv[index][1] -= take_count
		
		inventory.dnd_obj_name = obj_name
		inventory.dnd_obj_count = take_count
		inventory.dnd_mouse_button = 'right'
		inventory.dnd_from = inv, index
	
	def inventory__dnd_right_end(inv, index, obj_name, obj_count):
		if not obj_name or obj_name == inventory.dnd_obj_name:
			obj_name = inventory.dnd_obj_name
			max_count = location_objects[obj_name]['max_in_inventory_cell']
			if obj_count < max_count:
				inv[index][0] = obj_name
				inv[index][1] += 1
				inventory.dnd_obj_count -= 1
				if inventory.dnd_obj_count == 0:
					inventory.dnd_obj_name = None
		else:
			inv[index] = [inventory.dnd_obj_name, inventory.dnd_obj_count]
			inventory.dnd_obj_name, inventory.dnd_obj_count = obj_name, obj_count
		
		if not inventory.dnd_obj_name:
			inventory.dnd_mouse_button = None
		if inventory.current == '1':
			inventory.select(index)
	
	def inventory__update_dnd_image():
		if not inventory.dnd_obj_name:
			inventory.dnd_image = None
			return
		
		obj = location_objects[inventory.dnd_obj_name]
		main_frame = obj['animations'][None]
		image = main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext
		
		inventory.dnd_image = image
		inventory.dnd_image_xsize, inventory.dnd_image_ysize = inventory.get_cell_image_size(image)
	
	def inventory__on_esc():
		if not inventory.dnd_obj_name:
			inventory.close()
			return
		
		inv, index = inventory.dnd_from
		obj_name = inv[index][0]
		if not obj_name or obj_name == inventory.dnd_obj_name:
			inv[index][0] = inventory.dnd_obj_name
			inv[index][1] += inventory.dnd_obj_count
		else:
			lost = inventory.add(inventory.dnd_obj_name, inventory.dnd_obj_count)
			obj = location_objects[inventory.dnd_obj_name]
			if obj['remove_to_location']:
				r = inventory.throw_radius
				for i in xrange(lost):
					dx, dy = random.randint(-r, r), random.randint(-r, r)
					add_location_object(cur_location.name, {'x': me.x + dx, 'y': me.y + dy}, inventory.dnd_obj_name)
		
		inventory.dnd_obj_name = None
		inventory.dnd_mouse_button = None
	
	
	build_object('inventory')
	inventory.selected_cell = 0
	inventory.selection_time = 0.5
	inventory.appearance_time = 0.2
	inventory.disappearance_time = 0.2
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('inventory')


screen inventory_cells:
	has vbox
	spacing inventory.cell_yspacing
	xalign 0.5
	
	python:
		inventory.inv = inventory['inv' + inventory.current]
		inventory.full_rows = len(inventory.inv) / gui.inventory_xcount
		inventory.last_row = len(inventory.inv) - inventory.full_rows * gui.inventory_xcount
		
		if inventory.selected_cell >= len(inventory.inv1):
			inventory.selected_cell = len(inventory.inv1) - 1
	
	for i in xrange(inventory.full_rows + 1):
		hbox:
			spacing inventory.cell_xspacing
			xalign gui.inventory_cell_xalign
			
			for j in xrange(gui.inventory_xcount if i != inventory.full_rows else inventory.last_row):
				$ index = i * gui.inventory_xcount + j
				
				vbox:
					spacing inventory.cell_text_ypos
					
					button:
						xsize inventory.cell_xsize
						ysize inventory.cell_ysize
						
						ground inventory.get_cell_image(index, inventory.current == '1' and inventory.selected_cell == index)
						mouse inventory.full_alpha
						
						hovered   inventory.on_hovered(index)   if inventory.full_alpha else None
						unhovered inventory.on_unhovered(index) if inventory.full_alpha else None
						action    inventory.on_left_down(index)  if inventory.full_alpha else None
						alternate inventory.on_right_down(index) if inventory.full_alpha else None
					
					$ inventory.obj_name, inventory.obj_count = inventory.inv[index]
					text (str(inventory.obj_count) if inventory.obj_name else ' '):
						xalign gui.inventory_cell_text_xalign
						font      gui.inventory_cell_text_font
						text_size inventory.cell_text_size
						color        inventory.cell_text_color
						outlinecolor inventory.cell_text_outlinecolor

screen inventory:
	python:
		inventory.alpha = min((get_game_time() - inventory.showed_time) / inventory.appearance_time, 1)
		inventory.hiding = inventory.hided_time is not None
		if inventory.hiding:
			inventory.alpha = 1 - min((get_game_time() - inventory.hided_time) / inventory.disappearance_time, 1)
			if inventory.alpha == 0:
				hide_screen('inventory')
		inventory.full_alpha = inventory.alpha == 1
		
		if inventory.hovered_time and get_game_time() - inventory.hovered_time > inventory.selection_time:
			inventory.select(inventory.dnd_index)
	
	alpha inventory.alpha
	modal True
	
	if not inventory.hiding:
		key 'ESCAPE' action inventory.on_esc
		
		button:
			ground gui.bg('inventory_fog_bg')
			hover  gui.bg('inventory_fog_bg')
			alpha  gui.inventory_fog_alpha
			mouse  False
			size   1.0
			action inventory.on_esc
	
	python:
		inventory.cell_xsize = gui.get_int('inventory_cell_width')
		if gui.inventory_cell_height:
			inventory.cell_ysize = gui.get_int('inventory_cell_height')
		else:
			inventory.cell_ysize = inventory.cell_xsize
		
		inventory.cell_image_xsize = gui.get_int('inventory_cell_image_width', max_value = inventory.cell_xsize)
		if gui.inventory_cell_image_height:
			inventory.cell_image_ysize = gui.get_int('inventory_cell_image_height', max_value = inventory.cell_ysize)
		else:
			inventory.cell_image_ysize = inventory.cell_image_xsize
		
		inventory.cell_selected_over = gui.bg('inventory_cell_selected_over')
		inventory.cell_usual_over = gui.bg('inventory_cell_usual_over')
		
		inventory.cell_xspacing = gui.get_int('inventory_cell_xspacing')
		inventory.cell_yspacing = gui.get_int('inventory_cell_yspacing')
		
		inventory.cell_text_ypos = gui.get_int('inventory_cell_text_ypos')
		inventory.cell_text_size = gui.get_int('inventory_cell_text_size')
		inventory.cell_text_color        = gui.get_int('inventory_cell_text_color')
		inventory.cell_text_outlinecolor = gui.get_int('inventory_cell_text_outlinecolor')
		
		inventory.separator_xsize = gui.get_int('inventory_separator_width')
		inventory.separator_ysize = gui.get_int('inventory_separator_height')
		
		inventory.spacing = gui.get_int('inventory_spacing')
		inventory.edge_spacing = gui.get_int('inventory_edge_spacing')
		
		inventory.default_xsize = (inventory.cell_xsize + inventory.cell_xspacing) * gui.inventory_xcount - inventory.cell_xspacing
		inventory.default_xsize += inventory.edge_spacing * 2
		
		inventory.ycount1 = math.ceil(1.0 * len(inventory.inv1) / gui.inventory_xcount)
		inventory.ycount2 = math.ceil(1.0 * len(inventory.inv2) / gui.inventory_xcount) if inventory.inv2 else 0
		inventory.full_cell_ysize = inventory.cell_ysize + inventory.cell_text_ypos + inventory.cell_text_size + inventory.cell_yspacing
		
		inventory.default_ysize  = inventory.edge_spacing
		inventory.default_ysize += inventory.full_cell_ysize * inventory.ycount2 - (inventory.cell_yspacing if inventory.ycount2 else 0)
		if inventory.inv2:
			inventory.default_ysize += inventory.spacing + inventory.separator_ysize + inventory.spacing
		inventory.default_ysize += inventory.full_cell_ysize * inventory.ycount1 - inventory.cell_yspacing
		inventory.default_ysize += inventory.spacing
		inventory.default_ysize += gui.get_int('inventory_button_height') # <use> and <throw> buttons in the end
		inventory.default_ysize += inventory.edge_spacing
		
		inventory.xsize = gui.get_int('inventory_width',  default = int(inventory.default_xsize))
		inventory.ysize = gui.get_int('inventory_height', default = int(inventory.default_ysize))
	
	image gui.bg('inventory_bg'):
		xsize inventory.xsize
		ysize inventory.ysize
		xalign gui.inventory_xalign
		yalign gui.inventory_yalign
		
		vbox:
			spacing inventory.spacing
			align 0.5
			
			if inventory.inv2:
				$ inventory.current = '2'
				use inventory_cells
				
				image gui.bg('inventory_separator_bg'):
					xalign 0.5
					xsize inventory.separator_xsize
					ysize inventory.separator_ysize
			
			$ inventory.current = '1'
			use inventory_cells
			
			$ inventory.selected_object = inventory.get_selected_object()
			hbox:
				alpha 1 if inventory.selected_object and not inventory.dnd_obj_name else 0
				spacing gui.get_int('inventory_button_spacing')
				xalign 0.5
				
				python:
					inventory.buttons = (
						(
							'Use',
							gui.get_int('inventory_use_button_text_color'),
							gui.get_int('inventory_use_button_text_outlinecolor'),
							'inventory.add_event("using", inventory.selected_object.name)',
						),
						(
							'Lay out' if inventory.selected_object and inventory.selected_object['remove_to_location'] else 'Throw away',
							gui.get_int('inventory_throw_button_text_color'),
							gui.get_int('inventory_throw_button_text_outlinecolor'),
							inventory.remove_selected,
						),
					)
				
				for text, color, outlinecolor, action in inventory.buttons:
					textbutton _(text):
						xsize gui.get_int('inventory_button_width')
						ysize gui.get_int('inventory_button_height')
						ground gui.bg('inventory_button_ground')
						hover  gui.bg('inventory_button_hover')
						font      gui.inventory_button_text_font
						text_size gui.get_int('inventory_button_text_size')
						text_align gui.inventory_button_text_xalign
						color        color
						outlinecolor outlinecolor
						mouse  inventory.full_alpha
						action action if inventory.full_alpha else None
	
	$ inventory.update_dnd_image()
	if inventory.dnd_image:
		vbox:
			xsize inventory.dnd_image_xsize
			spacing inventory.cell_text_ypos
			
			pos get_mouse()
			xanchor inventory.dnd_image_xsize / 2
			yanchor inventory.dnd_image_ysize / 2
			skip_mouse True
			
			image inventory.dnd_image:
				xsize inventory.dnd_image_xsize
				ysize inventory.dnd_image_ysize
			
			text str(inventory.dnd_obj_count):
				xalign gui.inventory_cell_text_xalign
				font      gui.inventory_cell_text_font
				text_size inventory.cell_text_size
				color        inventory.cell_text_color
				outlinecolor inventory.cell_text_outlinecolor
