init -1 python:
	
	class Builder(Object):
		image = sc_map.image_dir + 'builder.png'
		symbol_offset = 0.75
		
		no_turn_symbol = 'z'
		no_turn_symbol_offset = 0.25
		
		def __init__(self, player, x, y):
			Object.__init__(self)
			
			self.player = player
			self.x, self.y = x, y
			
			self.start_turns = 1
			self.turns = self.start_turns
		
		def get_symbol(self):
			return 'z' if self.turns == 0 else ''
		
		def exec_task(self, task, *args):
			cell = sc_map.map[self.y][self.x]
			
			if task == 'build':
				for resource, count in building_cost['%s-%s' % tuple(args)].iteritems():
					if self.player[resource] < count:
						info.set_msg('Not enough resources', resource)
						return
				for resource, count in building_cost['%s-%s' % tuple(args)].iteritems():
					self.player[resource] -= count
				
				if cell.road_level == 0:
					cell.road_level = 1
					self.player.road_cells.append(cell)
				if args[0] != 'road':
					prev_building_level = cell.building_level
					cell.building, cell.building_level = args
					if prev_building_level == 0:
						self.player.building_cells.append(cell)
					
					if cell.building == 'district':
						self.player.add_worker(self.x, self.y)
					elif not self.player.main_storage_cell and cell.building == 'storage':
						self.player.main_storage_cell = cell
				
			elif task == 'unbuild':
				cell.building = None
				cell.building_level = 0
				self.player.building_cells.remove(cell)
				i = 0
				while i != len(self.player.units):
					unit = self.player.units[i]
					if isinstance(unit, Worker) and (unit.x, unit.y) == (self.x, self.y):
						self.player.units.pop(i)
					else:
						if isinstance(unit, Worker) and unit.work_cell is cell:
							unit.work_cell = None
						i += 1
				if cell is self.player.main_storage_cell:
					self.player.main_storage_cell = None
					for building_cell in self.player.building_cells:
						if building_cell.building == 'storage':
							self.player.main_storage_cell = building_cell
							break
			
			elif task == 'train':
				new_builder = self.player.add_builder(self.x, self.y)
				new_builder.turns = 0
			
			else:
				out_msg('Builder.exec_task', 'Expected task <build>, <unbuild> or <train>, got <' + str(task) + '>')
			
			self.turns = 0
			sc_map.update_block(self.x / sc_map.block_size, self.y / sc_map.block_size)
			control.select_cell(self.x, self.y) # update menu
		
		
		def get_menu(self):
			res = []
			
			if self.turns <= 0:
				res.append(MenuItem('No turns'))
				return res
			
			cell = sc_map.map[self.y][self.x]
			calc = self.player.calc_changing_resources
			
			res.append(MenuItem('Move', control.pick, 'm'))
			
			if cell.building == 'district':
				res.append(MenuItem('Train new builder', [Function(self.exec_task, 'train'), calc], 't'))
			
			if cell.road_level == 0:
				res.append(MenuItem('Make road', [Function(self.exec_task, 'build', 'road', 1), calc], 'r'))
			
			if cell.building:
				need_technology = cell.building if cell.building in technology_names else cell.resource
				if self.player.technology_progress[need_technology] > cell.building_level:
					res.append(MenuItem('Improve building', [
						Function(self.exec_task, 'build', cell.building, cell.building_level + 1),
						calc
					], 'i'))
				res.append(MenuItem('Remove building', [Function(self.exec_task, 'unbuild'), calc], 'u'))
			else:
				res.append(MenuItem('Build'))
				for building in control.selected_cell_buildings:
					need_technology = building if building in technology_names else cell.resource
					if self.player.technology_progress[need_technology] > 0:
						res.append(MenuItem(building, [Function(self.exec_task, 'build', building, 1), calc]))
			
			return res
		
		
		def selected_cell(self, cell_x, cell_y):
			if sc_map.has_road(self.x, self.y, cell_x, cell_y): # road between start and end
				spent_turns = False
				can_move = True
			else:
				spent_turns = True
				can_move = False
				
				cell_to = sc_map.map[cell_y][cell_x]
				if cell_to.road_level == 0:
					for dx, dy in ((-1, 0), (+1, 0), (0, -1), (0, +1)):
						nx, ny = cell_x + dx, cell_y + dy
						if nx == self.x and ny == self.y: # end is near to start
							can_move = True
							break
						
						if sc_map.outside(nx, ny): continue
						near = sc_map.map[ny][nx]
						if near.road_level == 0: continue
						
						if sc_map.has_road(self.x, self.y, nx, ny): # end's near have road to start
							can_move = True
							break
			
			if not can_move:
				info.set_msg('Moving is available on roads or to near cells')
				return
			
			if spent_turns:
				self.turns -= 1
			
			prev_x, prev_y = self.x, self.y
			self.x, self.y = cell_x, cell_y
			control.select_cell(prev_x, prev_y) # update menu
			
			self.player.calc_changing_resources()
