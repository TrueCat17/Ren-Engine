init python:
	
	def sc_control__set_player(index):
		for player in sc_map.players:
			if player.index != index: continue
			
			sc_map.player = player
			cell = sc_control.selected_cell
			if cell:
				sc_control.select_cell(cell.x, cell.y)
			break
	
	
	def sc_control__show_menu(key_enter = False):
		if key_enter and time.time() - sc_map.step_time < 0.5:
			return
		sc_context_menu.show(sc_control.menu_items)
	
	def sc_control__select_cell(cell_x, cell_y, skip_on_prev_cell = False):
		cell = sc_map.map[cell_y][cell_x]
		if skip_on_prev_cell and cell is sc_control.selected_cell:
			return
		
		sc_control.selected_cell = cell
		sc_info.set_msg('')
		menu = sc_control.menu_items = []
		
		player = sc_map.player
		if cell.player is not player or player.bot:
			return
		
		building = cell.building
		resource = cell.resource
		
		if building:
			upd1 = Function(sc_control.select_cell, cell_x, cell_y)
			upd2 = player.calc_changing_resources
			upd3 = sc_map.update_forces
			upd = [upd1, upd2, upd3]
			if cell.disabled:
				menu.append(SC_MenuItem('Enable', [GetSetAttr('disabled', False, obj = cell)] + upd, 'E'))
			else:
				menu.append(SC_MenuItem('Disable', [GetSetAttr('disabled', True, obj = cell)] + upd, 'E'))
			
			need_technology = building if building in sc_technologies.names else resource
			level = cell.building_level
			if player.technological_progress[need_technology] > level:
				menu.append(SC_MenuItem('Improve building', Function(player.build, cell_x, cell_y, building, level + 1), 'I'))
			menu.append(SC_MenuItem('Remove building', Function(player.unbuild, cell_x, cell_y), 'U', no_delay = True))
		
		else:
			last_building = player.last_building
			if last_building and last_building in sc_buildings.on_resource[resource]:
				need_technology = last_building if last_building in sc_technologies.names else resource
				if player.technological_progress[need_technology] > 0:
					menu.append(SC_MenuItem('Repeat', Function(player.build, cell_x, cell_y, last_building, 1), 'R', no_delay = True))
			
			menu.append(SC_MenuItem('Build'))
			for building in sc_buildings.on_resource[resource]:
				need_technology = building if building in sc_technologies.names else resource
				if player.technological_progress[need_technology] > 0:
					menu.append(SC_MenuItem(building, Function(player.build, cell_x, cell_y, building, 1)))
	
	
	sc_control = SimpleObject()
	build_object('sc_control')
	
	sc_control.back = im.rect('#CCC')
	
	sc_control.selected_cell = None
	sc_control.menu_items = []
