init -900 python:
	
	class Player(Object):
		
		def __init__(self, is_ai = False):
			Object.__init__(self)
			
			self.is_ai = is_ai
			
			self.max_storage_dist = 4
			
			self.units = []
			self.main_storage_cell = None
			self.road_cells = []
			self.building_cells = []
			
			self.science = 0
			for resource in info.resources:
				self[resource] = 0
			
			self.food = 100
			self.wood = 100
			self.stone = 150
			
			self.calc_changing_resources()
			
			self.technology_progress = {}
			for resource in info.simple_resources:
				self.technology_progress[resource] = 0
			for building in common_buildings:
				self.technology_progress[building] = 0
			for name in ('food', 'wood', 'stone', 'storage', 'district'):
				self.technology_progress[name] = 1
		
		def explore(self, building):
			self.science -= technology_costs[self.technology_progress[building] + 1]
			self.technology_progress[building] += 1
			control.select_cell(control.selected_cell.x, control.selected_cell.y) # update menu
		
		def add_builder(self, x, y):
			builder = Builder(self, x, y)
			self.units.append(builder)
			return builder
		def add_worker(self, x, y):
			worker = Worker(self, x, y)
			self.units.append(worker)
			cell = sc_map.map[y][x]
			cell.workers.append(worker)
			return worker
		
		def calc_changing_resources(self):
			# changes = 0
			for resource in info.resources:
				self['change_' + resource] = 0
			
			# cost of support and base science
			for cell in self.road_cells:
				for resource, count in support_cost['road'].iteritems():
					self['change_' + resource] -= count
			for cell in self.building_cells:
				if cell.building in ('storage', 'district'):
					for resource, count in support_cost[cell.building].iteritems():
						self['change_' + resource] -= count * cell.building_level
			for unit in self.units:
				if isinstance(unit, Worker):
					self.change_science += 1
			
			if not self.main_storage_cell:
				return
			
			# set enabled_level for max count of workers
			for cell in self.building_cells:
				cell.enabled_level = 0
			storage_cells = [cell for cell in self.building_cells if cell.building == 'storage']
			for cell in storage_cells:
				has_road = sc_map.has_road(self.main_storage_cell.x, self.main_storage_cell.y, cell.x, cell.y)
				cell.enabled_level = cell.building_level if has_road else 0 
			for cell in self.building_cells:
				if cell.building in ('storage', 'district'): continue
				near_storage_level = 0
				for storage_cell in storage_cells:
					if near_storage_level >= storage_cell.building_level: continue
					if abs(storage_cell.x - cell.x) + abs(storage_cell.y - cell.y) > self.max_storage_dist: continue
					if not sc_map.has_road(storage_cell.x, storage_cell.y, cell.x, cell.y): continue
					near_storage_level = storage_cell.enabled_level
				cell.enabled_level = min(near_storage_level, cell.building_level)
			
			# production
			for cell in self.building_cells:
				workers = min(len(cell.workers), cell.enabled_level)
				power = building_powers[workers]
				
				if cell.building in simple_production:
					self['change_' + cell.resource] += cell.resource_count * power
				elif cell.building in building_production:
					production = building_production[cell.building]
					for resource, count in production['from'].iteritems():
						self['change_' + resource] -= count * power
					for resource, count in production['to'].iteritems():
						self['change_' + resource] += count * power
				else:
					workers = 0
					power = 0
				
				self.change_science += power
				for resource, count in support_cost['worker'].iteritems():
					self['change_' + resource] -= count * workers
				for resource, count in support_cost[cell.building].iteritems():
					self['change_' + resource] -= count * workers
		
		
		def update(self):
			if self.is_ai:
				self.make_turn_ai()
			
			self.calc_changing_resources()
			for resource in info.resources:
				self[resource] += self['change_' + resource]
			
			for unit in self.units:
				if isinstance(unit, Builder):
					unit.turns = unit.start_turns
		
		
		def make_turn_ai(self):
			pass
