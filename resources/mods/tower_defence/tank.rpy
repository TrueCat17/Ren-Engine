init -1 python:
	td_tank_types = { # speed in cells/sec., speed_rotations in rotations/sec.
		'usual': { 'speed':2, 'speed_rotation':2, 'hp':30, 'color':'#080', 'cost':10 },
		'fast':  { 'speed':6, 'speed_rotation':4, 'hp':20, 'color':'#F80', 'cost':25 },
		'power': { 'speed':1, 'speed_rotation':1, 'hp':70, 'color':'#822', 'cost':15 }
	}
	
	td_tank_size = td_cell_size // 2
	
	td_tank_image = 'mods/tower_defence/images/tank.png'
	
	def init_tanks():
		for tank_type in td_tank_types:
			d = td_tank_types[tank_type]
			r, g, b, a = renpy.easy.color(d['color'])
			d['image'] = im.ReColor(td_tank_image, r, g, b, a)
	init_tanks()

init python:
	td_tanks = []
	
	def td_update_tanks():
		to_delete = []
		for tank in td_tanks:
			if tank.hp:
				tank.update(td_frame_time)
			if tank.hp == 0:
				to_delete.append(tank)
		for tank in to_delete:
			td_tanks.remove(tank)
	
	def td_check_tank_add():
		global td_tanks_created
		
		tank_type, count = td_levels[td_level]
		
		if td_tanks_created == count: return
		if td_tanks and td_tanks[-1].x < 0: return
		
		td_tanks_created += 1
		tank = TD_Tank(tank_type)
		td_tanks.append(tank)
	
	
	class TD_Tank(Object):
		FROST_K = 0.75
		
		def __init__(self, tank_type):
			Object.__init__(self)
			
			self.frost = False
			
			orig = td_tank_types[tank_type]
			for prop in orig:
				self[prop] = orig[prop]
			self.hp *= (td_level + 1) # x level
			self.speed *= 1 + td_level / 20.0 # 5% / level
			self.speed_rotation *= 1 + td_level / 20.0 # 5% / level
			
			self.to_index = 0
			
			self.to_x, self.to_y = td_path[self.to_index]
			self.x, self.y = self.to_x - 1, self.to_y
			self.from_x, self.from_y = self.x, self.y
			
			self.rotation = self.from_rotation = self.to_rotation = td_rotations[self.to_index]
			
			self.size = td_tank_size
		
		def update(self, dtime):
			while dtime:
				if self.rotation != self.to_rotation:
					dtime = self.rotate(dtime)
				else:
					dtime = self.move(dtime)
		
		def move(self, dtime):
			i = td_tanks.index(self)
			prev = i and td_tanks[i - 1]
			if prev and abs(prev.x - self.x) + abs(prev.y - self.y) < 1:
				return 0
			
			k = TD_Tank.FROST_K if self.frost else 1
			cdx, cdy = k * (self.to_x - self.from_x), k * (self.to_y - self.from_y) # (+/-k, 0) or (0, +/-k)
			dx, dy = self.to_x - self.x, self.to_y - self.y
			d = abs(dx + dy)
			
			step = dtime * self.speed * (cdx + cdy)
			if abs(step) < abs(d):
				if cdx:
					self.x += step
				else:
					self.y += step
				return 0
			
			self.to_index += 1
			if self.to_index == len(td_path):
				global td_hp
				td_hp = max(td_hp - self.hp, 0)
				self.hp = 0
				return 0
			
			self.x, self.y = self.to_x, self.to_y
			self.from_x, self.from_y = self.x, self.y
			self.to_x, self.to_y = td_path[self.to_index]
			
			self.from_rotation = self.to_rotation
			self.to_rotation = td_rotations[self.to_index - 1]
			
			return dtime * (1 - d / step)
		
		def rotate(self, dtime):
			k = TD_Tank.FROST_K if self.frost else 1
			speed_rotation = self.speed_rotation * k
			
			cd = (self.to_rotation - self.from_rotation) % 360
			d = (self.to_rotation - self.rotation) % 360
			if cd == 270:
				cd = -90
				d = 360 - d
			
			step = dtime * speed_rotation * cd
			if abs(step) < abs(d):
				self.rotation += step
				return 0
			
			self.rotation = self.to_rotation
			return dtime * (1 - d / step)

