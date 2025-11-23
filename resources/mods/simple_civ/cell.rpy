init python:
	
	class Cell:
		__slots__ = ('x', 'y', 'resource', 'resource_count', 'building', 'building_level', 'disabled', 'player', 'next_player', 'player_forces', 'nears_for_force')
		
		def __init__(self, x, y, resource, count):
			self.x = x
			self.y = y
			self.resource = resource
			self.resource_count = count
			
			self.building = None
			self.building_level = 0
			self.disabled = False
			
			self.player = None
			self.next_player = None
			self.player_forces = []
			self.nears_for_force = []
		
		def __hash__(self):
			return self.x * 1000 + self.y
		
		
		def update_nears_for_force(self):
			nears_for_force = self.nears_for_force
			nears_for_force.clear()
			
			if self.building is None:
				return
			
			level = self.building_level
			
			abs = __builtins__.abs
			max = __builtins__.max
			
			map = sc_map.map
			map_width = sc_map.xsize
			map_height = sc_map.ysize
			
			self_x = self.x
			self_y = self.y
			
			if self.building != 'barracks':
				dist = 4
				nears = []
				nears_for_force.append((level, nears))
				
				start_x = max(0,         self_x - dist)
				end_x   = min(map_width, self_x + dist + 1)
				
				start_y = max(0,          self_y - dist)
				end_y   = min(map_height, self_y + dist + 1)
				for y in range(start_y, end_y):
					line = map[y]
					nears.extend(line[start_x : end_x])
				return
			
			max_force = level + 3
			
			start_x = max(0,         self_x - max_force)
			end_x   = min(map_width, self_x + max_force + 1)
			range_x = range(start_x, end_x)
			
			start_y = max(0,          self_y - max_force)
			end_y   = min(map_height, self_y + max_force + 1)
			for y in range(start_y, end_y):
				dy = abs(y - self_y)
				line = map[y]
				
				for x in range_x:
					near = line[x]
					
					dist = max(abs(x - self_x), dy)
					force = (max_force - dist) * 2
					
					for existing_force, cells in nears_for_force:
						if existing_force == force:
							cells.append(near)
							break
					else:
						nears_for_force.append((force, [near]))
		
		
		def update_force(self):
			player = self.player
			if not player or self.disabled:
				return
			
			player_index = player.index
			for force, cells in self.nears_for_force:
				for cell in cells:
					cell.player_forces[player_index] += force
		
		
		def __getstate__(self):
			return tuple(getattr(self, prop) for prop in self.__slots__)
		def __setstate__(self, state):
			for i in range(len(self.__slots__)):
				setattr(self, self.__slots__[i], state[i])
