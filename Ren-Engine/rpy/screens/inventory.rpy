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
	gui.inventory_cell_width = 0.05
	gui.inventory_cell_height = None # None = auto (width)
	
	gui.inventory_cell_image_width = 0.75 # float - relatively cell width
	gui.inventory_cell_image_height = None # None = auto (width)
	
	gui.inventory_cell_text_ypos = 2 # spacing between image and text
	
	gui.inventory_button_spacing = 10
	
	
	def inventory__get_cell_image_size(image):
		w, h = get_image_size(image)
		k = min(screen_tmp.cell_image_xsize / w, screen_tmp.cell_image_ysize / h)
		return int(w * k), int(h * k)
	
	def inventory__get_cell_image(index, is_selected):
		obj_name, _obj_count = screen_tmp.inv[index]
		
		cache = inventory__get_cell_image.__dict__
		key = (obj_name, screen_tmp.cell_xsize, screen_tmp.cell_ysize, is_selected)
		if key in cache:
			return cache[key]
		
		hover = screen_tmp['cell_selected_over' if is_selected else 'cell_usual_over']
		hover = im.scale_without_borders(hover, screen_tmp.cell_xsize, screen_tmp.cell_ysize, need_scale = True)
		hover = get_back_with_color(hover)
		
		if not obj_name:
			cache[key] = hover
			return hover
		
		obj = location_objects[obj_name]
		main_frame = obj.animations[None]
		image = get_file_with_ext(main_frame.directory + main_frame.main_image) or im.rect('#888')
		
		w, h = inventory.get_cell_image_size(image)
		x, y = (screen_tmp.cell_xsize - w) // 2, (screen_tmp.cell_ysize - h) // 2
		
		cache[key] = im.composite(
			(screen_tmp.cell_xsize, screen_tmp.cell_ysize),
			(x, y), im.scale(image, w, h),
			(0, 0), hover,
		)
		return cache[key]
	
	def inventory__select(index):
		inventory.selected_cell = index
	
	def inventory__get_selected_object():
		obj_name, _obj_count = screen_tmp.inv[inventory.selected_cell]
		if obj_name:
			return location_objects[obj_name]
		return None
	
	def inventory__add_event(event, obj_name):
		rpg_events.add((event, obj_name))
		signals.send('inventory-' + event, obj_name)
	
	def inventory__remove_selected():
		element = screen_tmp.inv[inventory.selected_cell]
		obj_name = element[0]
		
		obj = location_objects[obj_name]
		if obj.remove_to_location:
			r = inventory.throw_radius
			dx, dy = random.randint(-r, r), random.randint(-r, r)
			add_location_object(cur_location.name, { 'x': me.x + dx, 'y': me.y + dy }, obj_name)
		
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
		if screen_tmp.current == '1':
			inventory.hovered_time = get_game_time()
		inventory.dnd_inv = screen_tmp.current
		inventory.dnd_index = index
	
	def inventory__on_unhovered(index):
		if inventory.dnd_inv == screen_tmp.current and inventory.dnd_index == index:
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
		inv[index][:] = ['', 0]
		inventory.dnd_obj_name = obj_name
		inventory.dnd_obj_count = obj_count
		inventory.dnd_mouse_button = 'left'
		inventory.dnd_from = inv, index
	
	def inventory__dnd_left_end(inv, index, obj_name, obj_count):
		element = inv[index]
		
		if obj_name == inventory.dnd_obj_name:
			max_count = location_objects[obj_name].max_in_inventory_cell
			d = min(max_count - obj_count, inventory.dnd_obj_count)
			if d > 0:
				obj_count += d
				element[1] = obj_count
				inventory.dnd_obj_count -= d
			if inventory.dnd_obj_count == 0:
				inventory.dnd_obj_name = None
		else:
			element[:] = [inventory.dnd_obj_name, inventory.dnd_obj_count]
			inventory.dnd_obj_name, inventory.dnd_obj_count = obj_name, obj_count
		
		if not inventory.dnd_obj_name:
			inventory.dnd_mouse_button = None
		if screen_tmp.current == '1':
			inventory.select(index)
	
	
	def inventory__on_right_down(index = None):
		if index is None:
			if inventory.dnd_inv and (inventory.dnd_index is not None):
				index = inventory.dnd_index
			else:
				return
		else:
			inventory.on_hovered(index)
		
		inv = inventory['inv' + inventory.dnd_inv]
		obj_name, obj_count = inv[index]
		
		if not inventory.dnd_obj_name:
			inventory.dnd_right_start(inv, index, obj_name, obj_count)
		else:
			inventory.dnd_right_end(inv, index, obj_name, obj_count)
	
	def inventory__dnd_right_start(inv, index, obj_name, obj_count):
		take_count = math.ceil(obj_count / 2)
		element = inv[index]
		element[1] -= take_count
		if element[1] == 0:
			element[0] = ''
		
		inventory.dnd_obj_name = obj_name
		inventory.dnd_obj_count = take_count
		inventory.dnd_mouse_button = 'right'
		inventory.dnd_from = inv, index
	
	def inventory__dnd_right_end(inv, index, obj_name, obj_count):
		element = inv[index]
		
		if not obj_name or obj_name == inventory.dnd_obj_name:
			obj_name = inventory.dnd_obj_name
			max_count = location_objects[obj_name].max_in_inventory_cell
			if obj_count < max_count:
				element[0] = obj_name
				element[1] += 1
				inventory.dnd_obj_count -= 1
				if inventory.dnd_obj_count == 0:
					inventory.dnd_obj_name = None
		else:
			element[:] = [inventory.dnd_obj_name, inventory.dnd_obj_count]
			inventory.dnd_obj_name, inventory.dnd_obj_count = obj_name, obj_count
		
		if not inventory.dnd_obj_name:
			inventory.dnd_mouse_button = None
		if inventory.dnd_inv == '1':
			inventory.select(index)
	
	def inventory__update_dnd_image():
		if not inventory.dnd_obj_name:
			screen_tmp.dnd_image = None
			return
		
		obj = location_objects[inventory.dnd_obj_name]
		main_frame = obj.animations[None]
		image = get_file_with_ext(main_frame.directory + main_frame.main_image) or im.rect('#888')
		
		screen_tmp.dnd_image = image
		screen_tmp.dnd_image_xsize, screen_tmp.dnd_image_ysize = inventory.get_cell_image_size(image)
	
	def inventory__on_esc():
		if not inventory.dnd_obj_name:
			inventory.close()
			return
		
		inv, index = inventory.dnd_from
		element = inv[index]
		obj_name = element[0]
		
		if not obj_name or obj_name == inventory.dnd_obj_name:
			element[0] = inventory.dnd_obj_name
			element[1] += inventory.dnd_obj_count
		else:
			lost = inventory.add(inventory.dnd_obj_name, inventory.dnd_obj_count)
			obj = location_objects[inventory.dnd_obj_name]
			if obj.remove_to_location:
				r = inventory.throw_radius
				for i in range(lost):
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


init:
	style inventory_cell_text is text:
		color '#FFF'
		outlinecolor 0
		text_size 16
		xalign 0.5
	
	style inventory_use_button is textbutton:
		xsize 180
		ysize 30
		color '#0F0'
		text_size 20
	
	style inventory_throw_button is inventory_use_button:
		color '#F00'


screen inventory_cells:
	has vbox
	spacing screen_tmp.cell_yspacing
	xalign 0.5
	
	python:
		screen_tmp.inv = inventory['inv' + screen_tmp.current]
		screen_tmp.full_rows = len(screen_tmp.inv) // gui.inventory_xcount
		screen_tmp.last_row = len(screen_tmp.inv) - screen_tmp.full_rows * gui.inventory_xcount
		
		if inventory.selected_cell >= len(inventory.inv1):
			inventory.selected_cell = len(inventory.inv1) - 1
	
	for i in range(screen_tmp.full_rows + 1):
		hbox:
			spacing screen_tmp.cell_xspacing
			xalign gui.inventory_cell_xalign
			
			for j in range(gui.inventory_xcount if i != screen_tmp.full_rows else screen_tmp.last_row):
				$ index = i * gui.inventory_xcount + j
				
				vbox:
					spacing screen_tmp.cell_text_ypos
					
					button:
						xsize screen_tmp.cell_xsize
						ysize screen_tmp.cell_ysize
						
						ground inventory.get_cell_image(index, screen_tmp.current == '1' and inventory.selected_cell == index)
						
						hovered   inventory.on_hovered(index)
						unhovered inventory.on_unhovered(index)
						action    inventory.on_left_down(index)
						alternate inventory.on_right_down(index)
					
					$ screen_tmp.obj_name, screen_tmp.obj_count = screen_tmp.inv[index]
					text (str(screen_tmp.obj_count) if screen_tmp.obj_name else ' '):
						style 'inventory_cell_text'

screen inventory:
	python:
		screen_tmp = SimpleObject()
		
		screen_tmp.alpha = min((get_game_time() - inventory.showed_time) / inventory.appearance_time, 1)
		screen_tmp.hiding = inventory.hided_time is not None
		if screen_tmp.hiding:
			screen_tmp.alpha = 1 - min((get_game_time() - inventory.hided_time) / inventory.disappearance_time, 1)
			if screen_tmp.alpha == 0:
				hide_screen('inventory')
		
		if inventory.hovered_time and get_game_time() - inventory.hovered_time > inventory.selection_time:
			inventory.select(inventory.dnd_index)
	
	alpha screen_tmp.alpha
	modal True
	
	
	if screen_tmp.alpha == 1:
		key 'I' action inventory.on_esc
		
		key 'MENU' action inventory.on_right_down
	
	if not screen_tmp.hiding:
		key 'ESCAPE' action inventory.on_esc
		
		button:
			ground gui.bg('inventory_fog_bg')
			hover  gui.bg('inventory_fog_bg')
			alpha  gui.inventory_fog_alpha
			mouse  False
			size   1.0
			action inventory.on_esc
	
	
	python:
		screen_tmp.cell_xsize = gui.get_int('inventory_cell_width')
		if gui.inventory_cell_height:
			screen_tmp.cell_ysize = gui.get_int('inventory_cell_height')
		else:
			screen_tmp.cell_ysize = screen_tmp.cell_xsize
		
		screen_tmp.cell_image_xsize = gui.get_int('inventory_cell_image_width', max_value = screen_tmp.cell_xsize)
		if gui.inventory_cell_image_height:
			screen_tmp.cell_image_ysize = gui.get_int('inventory_cell_image_height', max_value = screen_tmp.cell_ysize)
		else:
			screen_tmp.cell_image_ysize = screen_tmp.cell_image_xsize
		
		screen_tmp.cell_selected_over = gui.bg('inventory_cell_selected_over')
		screen_tmp.cell_usual_over    = gui.bg('inventory_cell_usual_over')
		
		screen_tmp.cell_xspacing = gui.get_int('inventory_cell_xspacing')
		screen_tmp.cell_yspacing = gui.get_int('inventory_cell_yspacing')
		
		screen_tmp.cell_text_ypos = gui.get_int('inventory_cell_text_ypos')
		
		screen_tmp.separator_xsize = gui.get_int('inventory_separator_width')
		screen_tmp.separator_ysize = gui.get_int('inventory_separator_height')
		
		screen_tmp.spacing = gui.get_int('inventory_spacing')
		screen_tmp.edge_spacing = gui.get_int('inventory_edge_spacing')
		
		screen_tmp.default_xsize = (screen_tmp.cell_xsize + screen_tmp.cell_xspacing) * gui.inventory_xcount - screen_tmp.cell_xspacing
		screen_tmp.default_xsize += screen_tmp.edge_spacing * 2
		
		screen_tmp.ycount1 = math.ceil(len(inventory.inv1) / gui.inventory_xcount)
		screen_tmp.ycount2 = math.ceil(len(inventory.inv2) / gui.inventory_xcount) if inventory.inv2 else 0
		screen_tmp.full_cell_ysize = screen_tmp.cell_ysize + screen_tmp.cell_text_ypos + style.inventory_cell_text.get_current('text_size') + screen_tmp.cell_yspacing
		
		screen_tmp.default_ysize  = screen_tmp.edge_spacing
		screen_tmp.default_ysize += screen_tmp.full_cell_ysize * screen_tmp.ycount2 - (screen_tmp.cell_yspacing if screen_tmp.ycount2 else 0)
		if inventory.inv2:
			screen_tmp.default_ysize += screen_tmp.spacing + screen_tmp.separator_ysize + screen_tmp.spacing
		screen_tmp.default_ysize += screen_tmp.full_cell_ysize * screen_tmp.ycount1 - screen_tmp.cell_yspacing
		screen_tmp.default_ysize += screen_tmp.spacing
		screen_tmp.default_ysize += max(style.inventory_use_button.get_current('ysize'), style.inventory_throw_button.get_current('ysize'))
		screen_tmp.default_ysize += screen_tmp.edge_spacing
		
		screen_tmp.xsize = gui.get_int('inventory_width',  default = screen_tmp.default_xsize)
		screen_tmp.ysize = gui.get_int('inventory_height', default = screen_tmp.default_ysize)
	
	image gui.bg('inventory_bg'):
		xsize screen_tmp.xsize
		ysize screen_tmp.ysize
		xalign gui.inventory_xalign
		yalign gui.inventory_yalign
		
		vbox:
			spacing screen_tmp.spacing
			align 0.5
			
			if inventory.inv2:
				$ screen_tmp.current = '2'
				use inventory_cells
				
				image gui.bg('inventory_separator_bg'):
					xalign 0.5
					xsize screen_tmp.separator_xsize
					ysize screen_tmp.separator_ysize
			
			$ screen_tmp.current = '1'
			use inventory_cells
			
			$ screen_tmp.selected_object = inventory.get_selected_object()
			hbox:
				alpha 1 if screen_tmp.selected_object and not inventory.dnd_obj_name else 0
				spacing gui.get_int('inventory_button_spacing')
				xalign 0.5
				
				$ screen_tmp.buttons = (
					(
						'Use',
						'inventory_use_button',
						'inventory.add_event("use", screen_tmp.selected_object.name)',
					),
					(
						'Lay out' if screen_tmp.selected_object and screen_tmp.selected_object.remove_to_location else 'Throw away',
						'inventory_throw_button',
						inventory.remove_selected,
					),
				)
				
				for text, style_name, action in screen_tmp.buttons:
					textbutton _(text):
						style style_name
						action action
	
	$ inventory.update_dnd_image()
	if screen_tmp.dnd_image:
		vbox:
			xsize screen_tmp.dnd_image_xsize
			spacing screen_tmp.cell_text_ypos
			
			pos get_mouse()
			xanchor screen_tmp.dnd_image_xsize // 2
			yanchor screen_tmp.dnd_image_ysize // 2
			skip_mouse True
			
			image screen_tmp.dnd_image:
				xsize screen_tmp.dnd_image_xsize
				ysize screen_tmp.dnd_image_ysize
			
			text str(inventory.dnd_obj_count):
				style 'inventory_cell_text'
	
	
	button:
		ground 'images/bg/black.jpg'
		hover  'images/bg/black.jpg'
		corner_sizes 0
		
		size 1.0
		alpha 0.01
		
		mouse False
		skip_mouse screen_tmp.alpha == 1
