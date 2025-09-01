init 2 python:
	tower_defence.tower_types = { # speed in bullets/sec
		'usual': { 'speed': 2, 'color': '#FF0', 'cost': 50 },
		'fast':  { 'speed': 3, 'color': '#F00', 'cost': 75 },
		'frost': { 'speed': 1, 'color': '#00F', 'cost': 75 },
	}
	
	def tower_defence__init_towers():
		image = tower_defence.images + 'tower.png'
		for tower_props in tower_defence.tower_types.values():
			r, g, b, a = renpy.easy.color(tower_props['color'])
			tower_props['image'] = im.recolor(image, r, g, b, a)
	tower_defence__init_towers()
	
	tower_defence.towers = []
	tower_defence.tower_size = tower_defence.cell_size // 2

init -1 python:
	def tower_defence__update_towers():
		for tower in tower_defence.towers:
			tower.update()
	
	def tower_defence__make_tower(x, y):
		tower_type = tower_defence.selected_tower_type
		tower_defence.moneys -= tower_defence.tower_types[tower_type]['cost']
		
		tower = TowerDefenceTower(tower_type, x, y)
		tower_defence.towers.append(tower)
	
	
	class TowerDefenceTower:
		MAX_DIST2 = 25 # 2 - power, **2, ^2
		
		def __init__(self, tower_type, x, y):
			self.__dict__.update(tower_defence.tower_types[tower_type])
			self.tower_type = tower_type
			
			self.x, self.y = absolute(x + 0.5), absolute(y + 0.5)
			self.size = tower_defence.tower_size
			
			self.rotation = 0
			
			self.shut_time = 1 / self.speed
			self.last_shut = -self.shut_time
		
		def update(self):
			tank = self.get_target_tank()
			if not tank:
				return
			
			dx = tank.x - self.x
			dy = tank.y - self.y
			
			self.rotation = math.atan2(dy, dx) * 180 / math.pi + 90
			
			if get_game_time() - self.last_shut < self.shut_time:
				return
			
			self.last_shut = get_game_time()
			
			dist = math.sqrt(dx * dx + dy * dy)
			bullet = TowerDefenceBullet(self.x, self.y, dx / dist, dy / dist, self.tower_type)
			tower_defence.bullets.append(bullet)
		
		def get_target_tank(self):
			x, y = self.x, self.y
			MAX_DIST2 = TowerDefenceTower.MAX_DIST2
			
			for tank in tower_defence.tanks:
				dx, dy = tank.x - x, tank.y - y
				if dx * dx + dy * dy < MAX_DIST2:
					return tank
			return None
