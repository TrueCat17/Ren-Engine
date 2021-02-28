init -1 python:
	td_tower_types = {
		'usual': { 'speed':2, 'color':'#FF0', 'cost':50 },
		'fast':  { 'speed':3, 'color':'#F00', 'cost':75 },
		'frost': { 'speed':1, 'color':'#00F', 'cost':75 },
	}
	
	def init_towers():
		image = 'mods/tower_defence/images/tower.png'
		for tower_type in td_tower_types:
			d = td_tower_types[tower_type]
			r, g, b, a = renpy.easy.color(d['color'])
			d['image'] = im.ReColor(image, r, g, b, a)
	init_towers()

init python:
	td_tower_size = td_cell_size / 2
	td_half_tower_size = td_tower_size / 2
	
	td_towers = []
	
	def td_update_towers():
		for tower in td_towers:
			tower.update()
	
	def make_tower(x, y):
		global td_moneys
		td_moneys -= td_tower_types[selected_tower_type]['cost']
		
		tower = TD_Tower(selected_tower_type, x, y)
		td_towers.append(tower)
	
	
	class TD_Tower(Object):
		MAX_DIST2 = 25 # 2 - power, **2, ^2
		
		def __init__(self, tower_type, x, y):
			Object.__init__(self)
			
			self.tower_type = tower_type
			orig = td_tower_types[tower_type]
			for prop in orig:
				self[prop] = orig[prop]
			
			self.x, self.y = x, y
			self.size = td_tower_size
			
			self.rotation = 0
			
			self.shut_time = 1.0 / self.speed
			self.last_shut = -self.shut_time
		
		def update(self):
			tank = self.get_near_tank()
			if tank is None:
				return
			
			dx = tank.x - self.x
			dy = tank.y - self.y
			dist = math.sqrt(dx*dx + dy*dy)
			
			self.rotation = math.atan2(dy, dx) * 180 / math.pi + 90
			
			if get_game_time() - self.last_shut < self.shut_time:
				return
			
			self.last_shut = get_game_time()
			
			bullet = Bullet(self.x, self.y, dx / dist, dy / dist, self.tower_type)
			td_bullets.append(bullet)
		
		def get_near_tank(self):
			x, y = self.x, self.y
			for tank in td_tanks:
				dx, dy = tank.x - x, tank.y - y
				if dx*dx + dy*dy < TD_Tower.MAX_DIST2:
					return tank
			return None

