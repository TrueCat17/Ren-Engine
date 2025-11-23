init python:
	
	class SC_Player(SimpleObject):
		
		def __init__(self, index):
			SimpleObject.__init__(self)
			
			self.index = index
			self.bot = None
			
			self.last_building = None
			self.building_cells = []
			
			for resource in sc_info.resources:
				self[resource] = 0
			
			self.food = 100
			self.wood = 100
			self.stone = 150
			
			self.calc_changing_resources()
			
			self.technological_progress = {}
			for resource in sc_info.simple_resources:
				self.technological_progress[resource] = 0
			for building in sc_buildings.common:
				self.technological_progress[building] = 0
			for name in ('food', 'wood', 'stone', 'barracks'):
				self.technological_progress[name] = 1
			
			color = sc_map.player_colors[index]
			self.border_h = im.color(sc_map.image_dir + 'border.webp', color)
			self.border_v = im.rotozoom(self.border_h, 90, 1)
			
			r, g, b, a = [c * 6 // 7 for c in renpy.easy.color(color)]
			size = 10
			border = 1
			self.image = im.composite(
				(size, size),
				(0, 0),           im.rect('#000', size, size),
				(border, border), im.rect((r, g, b), size - border * 2, size - border * 2),
			)
		
		
		def explore(self, building):
			self.technological_progress[building] += 1
			self.science -= sc_technologies.price[self.technological_progress[building]]
			if not self.bot:
				sc_ach.check()
				cell = sc_control.selected_cell
				if cell:
					sc_control.select_cell(cell.x, cell.y)
		
		
		def have_resources(self, building, level, n = 1):
			for resource, count in sc_buildings.price['%s-%s' % (building, level)]:
				if self[resource] < count * n:
					return False
			return True
		
		def deficit_resource(self):
			props = self.__dict__
			for resource in sc_info.simple_resources:
				k = 5 if resource == 'food' else 1
				if props[resource] + props['change_' + resource] * k < 0:
					return resource
			return None
		
		
		def build(self, x, y, building, level, free = False):
			cell = sc_map.map[y][x]
			
			if not free:
				need_resources = sc_buildings.price['%s-%s' % (building, level)]
				
				for resource, count in need_resources:
					if self[resource] < count:
						sc_info.set_msg('%s (%s)' % (_('Not enough resources'), _(resource)))
						return
				
				for resource, count in need_resources:
					self[resource] -= count
			
			if cell.building_level == 0:
				self.building_cells.append(cell)
			
			cell.building = building
			cell.building_level = level
			
			sc_map.update_block(cell.x // sc_map.block_size, cell.y // sc_map.block_size)
			if not self.bot and not free:
				sc_control.select_cell(cell.x, cell.y)
				self.last_building = building
			self.calc_changing_resources()
			
			cell.update_nears_for_force()
			sc_map.update_forces()
		
		
		def unbuild(self, x, y):
			cell = sc_map.map[y][x]
			
			cell.building = None
			cell.building_level = 0
			self.building_cells.remove(cell)
			
			sc_map.update_block(cell.x // sc_map.block_size, cell.y // sc_map.block_size)
			if not self.bot:
				sc_control.select_cell(cell.x, cell.y)
			self.calc_changing_resources()
			
			cell.update_nears_for_force()
			sc_map.update_forces()
		
		
		def calc_changing_resources(self):
			power = sc_buildings.power
			support = sc_buildings.support
			simple = sc_buildings.simple
			takes_get = sc_buildings.takes.get
			makes_get = sc_buildings.makes.get
			
			props = self.__dict__
			
			start_value = sc_map.bonus_for_bots if self.bot else 0
			for resource in sc_info.resources:
				props['change_' + resource] = start_value
			
			for cell in self.building_cells:
				building = cell.building
				level = cell.building_level
				cell_power = power[level]
				
				for resource, count in support[building]:
					props['change_' + resource] -= count * level
				
				if cell.disabled:
					continue
				
				props['change_science'] += cell_power
				props['change_food'] -= level
				
				if building in simple:
					props['change_' + cell.resource] += cell.resource_count * cell_power
				else:
					takes = takes_get(building)
					if takes is not None:
						for resource, count in takes:
							props['change_' + resource] -= count * cell_power
					
					makes = makes_get(building)
					if makes is not None:
						resource, count = makes
						props['change_' + resource] += count * cell_power
		
		
		def update(self):
			if not self.building_cells:
				return
			
			self.calc_changing_resources()
			for resource in sc_info.resources:
				self[resource] += self['change_' + resource]
