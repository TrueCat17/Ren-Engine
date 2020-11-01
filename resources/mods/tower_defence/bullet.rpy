init python:
	td_bullet_types = {
		'usual': { 'color':td_tower_types['usual']['color'], 'speed':10, 'damage':5, 'frost':False },
		'fast':  { 'color':td_tower_types['fast']['color'],  'speed':20, 'damage':5, 'frost':False },
		'frost': { 'color':td_tower_types['frost']['color'], 'speed':8,  'damage':4, 'frost':True  }
	}
	
	td_bullet_image = 'mods/tower_defence/images/bullet.png'
	td_bullet_image_bung = im.ReColor(td_bullet_image, 255, 128, 0)
	
	td_bullet_size = td_cell_size / 8
	td_bullet_test_size = float(td_tank_size + td_bullet_size) / td_cell_size
	td_bullet_test_half_size = td_bullet_test_size / 2
	
	
	td_bullets = []
	
	def td_update_bullets():
		to_delete = []
		for bullet in td_bullets:
			need_del = bullet.image == td_bullet_image_bung or bullet.update()
			if need_del:
				to_delete.append(bullet)
		for bullet in to_delete:
			td_bullets.remove(bullet)
	
	
	class Bullet(Object):
		def __init__(self, x, y, dx, dy, bullet_type):
			Object.__init__(self)
			
			self.x, self.y = x, y
			self.dx, self.dy = dx, dy
			
			orig = td_bullet_types[bullet_type]
			for prop in orig:
				self[prop] = orig[prop]
			
			r, g, b, a = renpy.easy.color(self.color)
			self.image = im.ReColor(td_bullet_image, r, g, b, a)
			
			self.size = td_bullet_size
		
		def check_tanks(self):
			global td_moneys
			x, y = self.x, self.y
			
			for tank in td_tanks:
				ctx, cty = tank.x, tank.y
				tx, ty = ctx - td_bullet_test_half_size, cty - td_bullet_test_half_size
				if x >= tx and x < tx + td_bullet_test_size and y >= ty and y < ty + td_bullet_test_size:
					tank.hp -= self.damage
					if self.frost:
						tank.frost = True
					if tank.hp <= 0:
						td_moneys += tank.cost
						td_tanks.remove(tank)
					self.image = td_bullet_image_bung
					self.size = int(self.size * 1.2)
					break
		
		def check_out(self):
			return self.x < -0.5 or self.y < -0.5 or self.x > td_map_w + 0.5 or self.y > td_map_h + 0.5
		
		def update(self):
			self.x += self.dx * self.speed * td_frame_time
			self.y += self.dy * self.speed * td_frame_time
			
			self.check_tanks()
			
			return self.check_out()

