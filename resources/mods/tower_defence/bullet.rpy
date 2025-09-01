init 3 python:
	tower_defence.bullet_types = { # speed in cells/sec
		'usual': { 'speed': 10, 'damage': 5, 'frost': False },
		'fast':  { 'speed': 20, 'damage': 4, 'frost': False },
		'frost': { 'speed': 8,  'damage': 4, 'frost': True  },
	}
	
	def tower_defence__init_bullets():
		image = tower_defence.images + 'bullet.png'
		for bullet_type, bullet_props in tower_defence.bullet_types.items():
			tower_props = tower_defence.tower_types[bullet_type]
			r, g, b, a = renpy.easy.color(tower_props['color'])
			bullet_props['image'] = im.recolor(image, r, g, b, a)
		tower_defence.bullet_image_bung = im.recolor(image, 255, 128, 0)
	tower_defence__init_bullets()
	
	tower_defence.bullets = []
	
	tower_defence.bullet_size = tower_defence.cell_size // 8
	tower_defence.bullet_test_size = (tower_defence.tank_size + tower_defence.bullet_size) / tower_defence.cell_size

init -1 python:
	def tower_defence__update_bullets():
		to_delete = []
		for bullet in tower_defence.bullets:
			need_del = bullet.image == tower_defence.bullet_image_bung or bullet.update()
			if need_del:
				to_delete.append(bullet)
		for bullet in to_delete:
			tower_defence.bullets.remove(bullet)
	
	
	class TowerDefenceBullet:
		def __init__(self, x, y, dx, dy, bullet_type):
			self.__dict__.update(tower_defence.bullet_types[bullet_type])
			
			self.x, self.y = absolute(x), absolute(y)
			self.dx, self.dy = dx, dy
			
			self.size = tower_defence.bullet_size
		
		def check_tanks(self):
			x, y = self.x, self.y
			
			size = tower_defence.bullet_test_size
			half_size = size / 2
			
			for tank in tower_defence.tanks:
				tx = tank.x - half_size
				ty = tank.y - half_size
				
				if x >= tx and x < tx + size and y >= ty and y < ty + size:
					tank.hp -= self.damage
					if self.frost:
						tank.frost = True
					if tank.hp <= 0:
						tower_defence.moneys += tank.cost
						tower_defence.tanks.remove(tank)
					self.image = tower_defence.bullet_image_bung
					self.size = int(self.size * 1.2)
					break
		
		def check_outside(self):
			return self.x < 0 or self.y < 0 or self.x > tower_defence.map_w or self.y > tower_defence.map_h
		
		def update(self):
			self.x += self.dx * self.speed * tower_defence.frame_time
			self.y += self.dy * self.speed * tower_defence.frame_time
			
			self.check_tanks()
			
			return self.check_outside()
