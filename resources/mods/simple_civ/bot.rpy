init python:
	resource_to_building = {
		'food': 'farm',
		'wood': 'sawmill',
		'stone': 'stone career',
		'coal':   'coal career',
		'metal': 'metal career',
		'cement': 'cement factory',
		'steel':  'metal factory',
		'science': 'college',
	}
	
	def get_building_short_name(building_full_name):
		if 'career' in building_full_name:
			return 'career'
		return building_full_name
	def get_building_full_name(resource, building):
		if building == 'career':
			return resource + ' ' + building
		return building
	
	
	class SC_Bot(SimpleObject):
		
		def __init__(self, player):
			SimpleObject.__init__(self)
			
			self.player = player
			self.executed = True
			self.attacking = False
			
			self.have_buildings = set()
			
			self.cement_factories = []
			self.metal_factories = []
			
			self.my_cells = []
			self.my_borders = []
			self.my_borders_for_common_buildings = []
			self.near_enemy_buildings = []
			
			self.resource_max = {
				'food':     500,
				'wood':    1000,
				'stone':   1000,
				'coal':     800,
				'metal':    700,
				'cement':   500,
				'steel':    500,
				'science': 1400,
			}
			
			k = (48 * 24) / (sc_map.xsize * sc_map.ysize)
			for resource in self.resource_max:
				if resource != 'science':
					self.resource_max[resource] /= k
			self.resource_max['science'] *= k
			
			self.resource_k = {
				'food':  2000,
				'wood':  1000,
				'stone': 1000,
				'coal':   600,
				'metal':  300,
				'cement': 200,
				'steel':  200,
				'science': 300,
			}
			
			self.looking_future_steps = 3
			
			self.random = random.Random(sc_map.random.randint(0, 999))
			self.chance_to_defence = 0.25
			
			self.to_unbuild = []
		
		
		def build(self, cell, building, level):
			if building == 'cement factory':
				self.cement_factories.append(cell)
			elif building == 'metal factory':
				self.metal_factories.append(cell)
			
			self.player.build(cell.x, cell.y, building, level)
			self.have_buildings.add(get_building_full_name(cell.resource, building))
		
		
		def choose_technology(self):
			player = self.player
			technological_progress = player.technological_progress
			
			order = (
				'food',
				'stone',
				'barracks',
				'college',
				'coal',
				'cement factory',
				'metal',
				'metal factory',
				'wood',
			)
			if self.resource_max['science'] != 0:
				base_only = ('college', 'cement factory', 'metal factory')
			else:
				base_only = ()
			
			min_level = min(technological_progress.values())
			if min_level > 0:
				min_level = min(value for name, value in technological_progress.items() if name not in base_only)
			
			next_level = min_level + 1
			if next_level >= len(sc_technologies.price):
				return None
			
			if player.science < sc_technologies.price[next_level]:
				return ''
			
			for name in order:
				if min_level > 0 and name in base_only: continue
				
				if technological_progress[name] == min_level:
					return name
			
			return ''
		
		
		def get_resource_priority(self):
			player = self.player
			props = player.__dict__
			
			looking_future_steps = self.looking_future_steps
			resource_max = self.resource_max
			have_buildings = self.have_buildings
			
			res = []
			for resource, k in self.resource_k.items():
				cur = props[resource] if building != 'college' else 0
				change = props['change_' + resource]
				max_value = resource_max[resource]
				
				if cur + change >= max_value and change > 0: continue
				
				count = cur + change * looking_future_steps
				score = (max_value - count) * k
				
				building_full_name = resource_to_building[resource]
				if building_full_name not in have_buildings:
					score += 1e9
				
				res.append((score, resource))
			
			res.sort(key = lambda t: t[0], reverse = True)
			return res
		
		
		def get_cell_info(self):
			player = self.player
			index = player.index
			abs = __builtins__.abs
			max = __builtins__.max
			
			near_enemy_buildings = self.near_enemy_buildings
			enemy_indices = [i for i in range(sc_map.count_of_players) if i != index]
			
			cache = self.get_cell_info.__dict__
			if cache.get('last_step') != sc_map.step:
				cache.clear()
				cache['last_step'] = sc_map.step
			
			cells = []
			for cell in player.building_cells:
				min_dist = cache.get(cell)
				if min_dist is None:
					min_dist = 10
					cell_x, cell_y = cell.x, cell.y
					
					for enemy_building in near_enemy_buildings:
						dx = abs(cell_x - enemy_building.x)
						if dx >= min_dist: continue
						
						dy = abs(cell_y - enemy_building.y)
						if dy >= min_dist: continue
						
						min_dist = dx if dx > dy else dy
					
					cache[cell] = min_dist
				
				if min_dist == 10: continue
				
				cell_player_forces = cell.player_forces
				max_enemy_force = max(cell_player_forces[i] for i in enemy_indices)
				
				need_force = max_enemy_force
				if   min_dist < 3: need_force += 9
				elif min_dist < 5: need_force += 7
				elif min_dist < 7: need_force += 5
				else:              need_force += 3
				
				if cell_player_forces[index] < need_force:
					cells.append((need_force, cell))
			
			cells.sort(key = lambda t: t[0], reverse = True)
			return cells
		
		
		def get_dist_to_border(self, cell, max_dist = 1000000):
			cache = self.get_dist_to_border.__dict__
			if cache.get('last_step') != sc_map.step:
				cache.clear()
				cache['last_step'] = sc_map.step
			
			key = (self.player.index, cell, max_dist)
			res = cache.get(key)
			if res is not None:
				return res
			
			abs = __builtins__.abs
			cell_x, cell_y = cell.x, cell.y
			
			res = max_dist
			for border_cell in self.my_borders:
				dx = abs(cell_x - border_cell.x)
				if dx >= res: continue
				
				dy = abs(cell_y - border_cell.y)
				if dy >= res: continue
				
				res = dx if dx > dy else dy
				if res == 0:
					break
			
			cache[key] = res
			return res
		
		
		def get_near_for_barrack(self, nears, level):
			player = self.player
			
			need_cell = None
			min_dist = 4
			
			for near in nears:
				if near.player is not player: continue
				if near.resource != 'wood': continue
				
				building = near.building
				if building is not None and building != 'barracks': continue
				if near.building_level != level: continue
				
				dist = self.get_dist_to_border(near, max_dist = min_dist)
				if dist == 0:
					return near
				
				if dist < min_dist:
					min_dist = dist
					need_cell = near
			
			return need_cell
		
		
		def check_defence(self):
			player = self.player
			if not self.my_borders or player.food + player.change_food <= 0:
				return False
			
			n_level = player.technological_progress['barracks']
			while n_level > 0 and not player.have_resources('barracks', n_level, n = 5):
				n_level -= 1
			if n_level <= 0:
				return False
			
			cell_info = self.get_cell_info()
			for need_force, cell in cell_info:
				nears = []
				for force, cells in cell.nears_for_force:
					nears.extend(cells)
				self.random.shuffle(nears)
				
				for level in range(n_level):
					near = self.get_near_for_barrack(nears, level)
					if near:
						self.build(near, 'barracks', level + 1)
						return True
			
			return False
		
		
		def check_attack(self):
			max_dist_to_border = 4
			
			player = self.player
			n_level = player.technological_progress['barracks']
			
			for cells in (self.my_borders, self.my_cells):
				only_borders = cells is self.my_borders
				
				need_cell = None
				min_level = n_level
				
				for cell in cells:
					if cell.resource != 'wood': continue
					
					building = cell.building
					if building is not None and building != 'barracks': continue
					if cell.building_level >= min_level: continue
					
					if not only_borders:
						if self.get_dist_to_border(cell, max_dist = max_dist_to_border) >= max_dist_to_border:
							continue
					
					min_level = cell.building_level
					need_cell = cell
					if min_level == 0:
						break
				
				if need_cell:
					if not player.have_resources('barracks', min_level + 1):
						return False
					
					self.build(need_cell, 'barracks', min_level + 1)
					return True
			
			return False
		
		
		def prepare(self):
			player = self.player
			if not player.building_cells:
				return
			
			self.executed = False
			self.attacking = False
			
			while True:
				tech = self.choose_technology()
				
				if tech is None:
					self.resource_max['science'] = 0
					for cell in player.building_cells:
						if cell.building == 'college':
							self.to_unbuild.append(cell)
				
				if tech:
					player.technological_progress[tech] += 1
					level = player.technological_progress[tech]
					player.science -= sc_technologies.price[level]
				else:
					break
			
			
			for cell in player.building_cells:
				cell.disabled = False
			
			have_buildings = self.have_buildings = set()
			for cell in player.building_cells:
				building_full_name = get_building_full_name(cell.resource, cell.building)
				have_buildings.add(building_full_name)
			
			for cells in (self.cement_factories, self.metal_factories):
				i = 0
				while i < len(cells):
					if cells[i].player is not player:
						cells.pop(i)
					else:
						i += 1
			
			
			map = sc_map.map
			map_width = sc_map.xsize
			map_height = sc_map.ysize
			
			my_cells = self.my_cells = []
			for line in map:
				for cell in line:
					if cell.player is player:
						my_cells.append(cell)
			
			my_borders = set()
			for cell in my_cells:
				cell_x, cell_y = cell.x, cell.y
				
				for dx, dy in ((-1, 0), (-1, -1), (0, -1), (+1, -1), (+1, 0), (+1, +1), (0, +1), (-1, +1)):
					x = cell_x + dx
					if x < 0 or x >= map_width: continue
					
					y = cell_y + dy
					if y < 0 or y >= map_height: continue
					
					near = map[y][x]
					if near.player is not player:
						my_borders.add(cell)
						break
			self.my_borders = list(my_borders)
			
			near_enemy_buildings = set()
			near_enemy_range = range(-9, 10)
			for cell in my_borders:
				cell_x, cell_y = cell.x, cell.y
				
				for dy in near_enemy_range:
					y = cell_y + dy
					if y < 0 or y >= map_height: continue
					
					line = map[y]
					
					for dx in near_enemy_range:
						x = cell_x + dx
						if x < 0 or x >= map_width: continue
						
						near = line[x]
						near_player = near.player
						if near_player is None: continue
						if near_player is player: continue
						if near.building is None: continue
						
						near_enemy_buildings.add(near)
			self.near_enemy_buildings = list(near_enemy_buildings)
			
			
			resource_cells = self.resource_cells = {}
			for resource, cells in sc_map.resource_cells.items():
				array = resource_cells[resource] = []
				
				for cell in cells:
					if cell.player is not player: continue
					
					dist = self.get_dist_to_border(cell, max_dist = 7)
					
					score = cell.resource_count - dist * 2 - cell.building_level * 10
					array.append((score, cell))
				
				array.sort(key = lambda t: t[0], reverse = True)
				array[:] = [cell for score, cell in array]
			
			
			max_dist_for_barracks = 6
			for cell in player.building_cells:
				if cell.building != 'barracks': continue
				
				min_dist = self.get_dist_to_border(cell, max_dist = max_dist_for_barracks)
				if min_dist >= max_dist_for_barracks:
					self.to_unbuild.append(cell)
			
			
			# cement factory, metal factory, college
			my_borders_for_common_buildings = self.my_borders_for_common_buildings = []
			best_wood_cells = resource_cells['wood']
			for cell in my_borders:
				building = cell.building
				if building is not None and building != 'wood': continue
				
				if cell not in best_wood_cells:
					my_borders_for_common_buildings.append(cell)
			
			self.random.shuffle(self.my_cells)
			self.random.shuffle(self.my_borders)
			self.random.shuffle(self.my_borders_for_common_buildings)
		
		
		def execute_one_building(self):
			player = self.player
			priority = self.get_resource_priority()
			
			for _score, resource in priority:
				building_full_name = resource_to_building[resource]
				building = get_building_short_name(building_full_name)
				
				deficit_resource = player.deficit_resource()
				if deficit_resource and deficit_resource != resource and building_full_name in self.have_buildings: continue
				
				if building in sc_buildings.common:
					if player.technological_progress[building] == 0: continue
					if not player.have_resources(building, 1): continue
					
					takes = sc_buildings.takes.get(building)
					if takes:
						ok = True
						for resource, count in takes:
							if player[resource] + (player['change_' + resource] + count) * self.looking_future_steps < 0:
								ok = False
								break
						if not ok:
							continue
					
					for cells in (self.my_borders_for_common_buildings, self.my_cells):
						for cell in cells:
							if cell.building is not None: continue
							if cell.resource != 'wood': continue
							
							self.build(cell, building, 1)
							return True
				
				else:
					n_level = player.technological_progress[resource]
					while n_level > 0 and not player.have_resources(building, n_level):
						n_level -= 1
					if n_level <= 0:
						continue
					
					need_cell = None
					max_score = None
					
					for cell in self.resource_cells[resource]:
						cell_building = cell.building
						if cell_building is not None and cell_building != building: continue
						if cell.building_level >= n_level: continue
						
						resource_count = cell.resource_count
						if max_score is not None and resource_count < max_score: continue
						
						dist = self.get_dist_to_border(cell, max_dist = 5)
						score = resource_count - dist * 2
						
						if max_score is None or score > max_score:
							max_score = score
							need_cell = cell
					
					if need_cell:
						self.build(need_cell, building, need_cell.building_level + 1)
						return True
			return False
		
		
		def execute(self):
			if self.executed:
				return
			
			player = self.player
			
			if self.to_unbuild:
				for cell in self.to_unbuild[-3:]:
					player.unbuild(cell.x, cell.y)
				self.to_unbuild[-3:] = ()
				return
			
			if self.random.random() < self.chance_to_defence:
				self.check_defence()
				return
			
			if self.execute_one_building():
				return
			
			
			def have_max():
				return player.wood > self.resource_max['wood'] and player.stone > self.resource_max['stone']
			def have_half():
				return player.wood > self.resource_max['wood'] / 2 and player.stone > self.resource_max['stone'] / 2
			
			if have_max():
				self.attacking = True
			if not have_half():
				self.attacking = False
			
			if self.attacking:
				if self.check_attack():
					return
			
			if have_half():
				if self.check_defence():
					return
			
			self.fix_resources()
			self.executed = True
		
		
		def fix_factories(self):
			player = self.player
			player.calc_changing_resources()
			
			resource_max = self.resource_max
			
			for building, cells in (('cement factory', self.cement_factories), ('metal factory', self.metal_factories)):
				
				resource, _count = sc_buildings.makes[building]
				if player[resource] >= resource_max[resource]:
					for cell in cells:
						cell.disabled = True
					player.calc_changing_resources()
					sc_map.update_forces()
					continue
				
				takes = sc_buildings.takes[building]
				for cell in cells:
					disable = False
					for resource, _count in takes:
						if player[resource] + player['change_' + resource] < 0:
							disable = True
							break
					
					if disable:
						cell.disabled = True
						player.calc_changing_resources()
						sc_map.update_forces()
					else:
						break
		
		
		def fix_resources(self):
			self.fix_factories()
			player = self.player
			
			if not player.deficit_resource():
				return
			
			def get_common_score():
				res = 0
				priority = self.get_resource_priority()
				for score, resource in priority:
					if score > 0 and resource != 'science':
						res += score
				return res
			
			simple_buildings = sc_buildings.simple
			
			for soft_actions in (True, False):
				seen = set()
				array_for = defaultdict(list)
				for cell in player.building_cells:
					if cell.disabled: continue
					
					building = cell.building
					building_full_name = get_building_full_name(cell.resource, building)
					
					if building == 'farm': continue
					if soft_actions:
						if building in ('college', 'barracks'): continue
					
					if building_full_name not in seen:
						seen.add(building_full_name)
						if building not in ('college', 'barracks'): continue
					
					score = sc_buildings.power[cell.building_level]
					if building in simple_buildings:
						score *= cell.resource_count
					
					dist = self.get_dist_to_border(cell, max_dist = 5)
					score -= dist * 2
					
					array_for[building_full_name].append((score, cell))
				
				for cells in array_for.values():
					cells.sort(key = lambda t: t[0])
					cells[:] = [cell for _score, cell in cells]
				
				while player.deficit_resource() and any(array_for.values()):
					have_food = player.food + player.change_food >= 0
					
					prev_score = get_common_score()
					min_building_full_name = None
					min_score = None
					
					for building_full_name, cells in array_for.items():
						if not cells: continue
						
						cell = cells[0]
						
						cell.disabled = True
						player.calc_changing_resources()
						score = get_common_score()
						cell.disabled = False
						
						if min_score is None or score < min_score:
							min_building_full_name = building_full_name
							min_score = score
					
					if have_food and min_score >= prev_score:
						break
					
					cells = array_for[min_building_full_name]
					cell = cells.pop(0)
					cell.disabled = True
					player.calc_changing_resources()
					sc_map.update_forces()
				
				self.fix_factories()
