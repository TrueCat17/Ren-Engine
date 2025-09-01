init 2 python:
	tower_defence.tank_types = { # speed in cells/sec., rotation_speed in rotations/sec.
		'usual': { 'speed': 2, 'rotation_speed': 2, 'hp': 30, 'color': '#080', 'cost': 10 },
		'fast':  { 'speed': 6, 'rotation_speed': 4, 'hp': 20, 'color': '#F80', 'cost': 25 },
		'power': { 'speed': 1, 'rotation_speed': 1, 'hp': 70, 'color': '#822', 'cost': 15 },
	}
	
	def tower_defence__init_tanks():
		image = tower_defence.images + 'tank.png'
		for tank_props in tower_defence.tank_types.values():
			r, g, b, a = renpy.easy.color(tank_props['color'])
			tank_props['image'] = im.recolor(image, r, g, b, a)
	tower_defence__init_tanks()
	
	tower_defence.tanks = []
	tower_defence.tank_size = tower_defence.cell_size // 2

init -1 python:
	def tower_defence__update_tanks():
		to_delete = []
		for tank in tower_defence.tanks:
			if tank.hp:
				tank.update(tower_defence.frame_time)
			if tank.hp == 0:
				to_delete.append(tank)
		for tank in to_delete:
			tower_defence.tanks.remove(tank)
	
	def tower_defence__check_tank_adding():
		tank_type, count = tower_defence.levels[tower_defence.level]
		
		if tower_defence.tanks_created == count: return
		if tower_defence.tanks and tower_defence.tanks[-1].x < 0: return
		
		tower_defence.tanks_created += 1
		tank = TowerDefenceTank(tank_type)
		tower_defence.tanks.append(tank)
	
	
	class TowerDefenceTank:
		FROST_K = 0.75
		
		def __init__(self, tank_type):
			self.__dict__.update(tower_defence.tank_types[tank_type])
			
			self.frost = False
			self.hp *= tower_defence.level + 1
			
			k = 1 + tower_defence.level * 0.05
			self.speed *= k
			self.rotation_speed *= k
			
			self.to_index = 0
			
			self.to_x, self.to_y = tower_defence.tank_path[self.to_index]
			self.x, self.y = self.to_x - 1, self.to_y
			self.from_x, self.from_y = self.x, self.y
			
			self.rotation = self.from_rotation = self.to_rotation = tower_defence.rotations[self.to_index]
			
			self.size = tower_defence.tank_size
		
		def update(self, dtime):
			while dtime:
				if self.rotation != self.to_rotation:
					dtime = self.rotate(dtime)
				else:
					dtime = self.move(dtime)
		
		def move(self, dtime):
			i = tower_defence.tanks.index(self)
			prev = i and tower_defence.tanks[i - 1]
			if prev and abs(prev.x - self.x) + abs(prev.y - self.y) < 1:
				return 0
			
			k = TowerDefenceTank.FROST_K if self.frost else 1
			cdx, cdy = k * (self.to_x - self.from_x), k * (self.to_y - self.from_y) # (+/-k, 0) or (0, +/-k)
			dx, dy = self.to_x - self.x, self.to_y - self.y
			d = dx + dy
			
			step = dtime * self.speed * (cdx + cdy)
			if abs(step) < abs(d):
				if cdx:
					self.x += step
				else:
					self.y += step
				return 0
			
			self.to_index += 1
			if self.to_index == len(tower_defence.tank_path):
				tower_defence.hp = max(tower_defence.hp - self.hp, 0)
				self.hp = 0
				return 0
			
			self.x, self.y = self.to_x, self.to_y
			self.from_x, self.from_y = self.x, self.y
			self.to_x, self.to_y = tower_defence.tank_path[self.to_index]
			
			self.from_rotation = self.to_rotation
			self.to_rotation = tower_defence.rotations[self.to_index - 1]
			
			return dtime * (1 - abs(d / step))
		
		def rotate(self, dtime):
			k = TowerDefenceTank.FROST_K if self.frost else 1
			rotation_speed = self.rotation_speed * k
			
			cd = self.to_rotation - self.from_rotation
			d  = self.to_rotation - self.rotation
			
			step = dtime * rotation_speed * cd
			if abs(step) < abs(d):
				self.rotation += step
				return 0
			
			self.rotation = self.to_rotation
			return dtime * (1 - abs(d / step))
