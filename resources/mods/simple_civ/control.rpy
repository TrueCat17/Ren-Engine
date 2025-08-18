init -1 python:
	
	def control__on_click():
		x, y = get_mouse()
		cell_x = int((x - sc_map.x) / sc_map.zoom / sc_map.image_size)
		cell_y = int((y - sc_map.y) / sc_map.zoom / sc_map.image_size)
		
		if control.picking:
			control.picking = False
			hide_screen('picking')
			info.set_msg('')
			
			unit = control.selected_unit
			if unit.x == cell_x and unit.y == cell_y:
				info.set_msg('Cancel selection')
				return
			
			unit.selected_cell(cell_x, cell_y)
			sc_map.draw_units()
		else:
			control.select_cell(cell_x, cell_y)
	
	def control__select_cell(cell_x, cell_y):
		control.selected_cell = sc_map.map[cell_y][cell_x]
		
		cell_units = [unit for unit in sc_map.player.units if unit.x == cell_x and unit.y == cell_y]
		free_units = []
		not_free_units = []
		for UnitType in [Builder, Worker]:
			for unit in cell_units:
				if isinstance(unit, UnitType):
					(not_free_units if unit.get_symbol() else free_units).append(unit)
		
		units = control.selected_unit_array = free_units + not_free_units
		control.select_unit(units[0] if units else None)
	
	def control__select_unit(unit):
		control.selected_unit_index = control.selected_unit_array.index(unit) if unit else 0
		control.selected_unit = unit
		control.set_visible_units()
		
		control.selected_unit_menu_items = unit.get_menu() if unit else []
	
	def control__set_visible_units():
		l = len(control.selected_unit_array)
		if l < 5:
			s = 0
		else:
			s = in_bounds(control.selected_unit_index - 2, 0, l - 5)
		
		control.selected_unit_array_start = s
		control.selected_unit_array_end = min(s + 5, l)
	
	
	def control__next_unit():
		control.selected_unit_index = control.selected_unit_index + 1
		control.select_unit(control.selected_unit_array[control.selected_unit_index])
	
	def control__prev_unit():
		control.selected_unit_index = control.selected_unit_index - 1
		control.select_unit(control.selected_unit_array[control.selected_unit_index])
	
	
	def control__pick():
		control.picking = True
		show_screen('picking')
		info.set_msg('Cell selection')
	def control__unpick():
		control.picking = False
		hide_screen('picking')
		info.set_msg('Cancel selection')
	
	
	
	build_object('control')
	
	control.step = 1
	
	control.indent = 15
	control.text_size = 25
	control.xsize = 250
	control.back = im.rect('#CCC')
	control.selected_unit_bg = im.rect('#0A0')
	control.unselected_unit_bg = im.rect('#999')
	
	control.picking = False
	
	control.selected_cell = None
	control.selected_unit_array = []
	control.selected_unit = None
	control.selected_unit_menu_items = []
	
	control.unit_back = im.rect('#AAA')
	control.unit_size = 32



init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('picking')

# empty screen, just to disable <pause> menu
screen picking:
	pass
