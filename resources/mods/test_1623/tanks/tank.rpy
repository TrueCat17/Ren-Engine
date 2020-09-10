# test_1623_tanks__

init -1 python:
	
	min_gens = -1 * 7
	max_gens =  1 * 7
	
	super_gens = (
		(-1, -0.25, -0.25, -0.5, -0.75, -1, 1),
		(-1, -0.25, 0.25, -0.5, 0.25, -0.5, 0.5),
		(-1, 0, 0.75, -0.25, 1, 0, 0.75),
		(-1, 0.25, 0.25, -0.25, 1, -0.25, -0.5),
		(-1, 0.25, 1, -0.75, -0.75, 1, 1),
		(-1, 0.5, -0.75, -1, 0.75, -0.5, 0.25),
		(-0.75, 0.75, -0.75, -0.75, 0.5, -1, 1),
		(-0.75, 0.75, 0, -0.75, 0, 0, 0.25),
		(-0.75, 0.75, 0.75, -0.75, -0.5, -0.25, 0.75),
		(-0.5, -1, -0.5, -0.75, 0.5, 0.5, -1),
		(-0.5, -0.75, 0.75, -0.5, -0.25, -1, 0.75),
		(-0.5, 0, -1, -0.75, 0, -0.5, 0.5),
		(-0.5, 0, -0.75, -1, -0.5, 0.25, 0.75),
		(-0.5, 0.25, 0, -0.25, 0, -0.25, 0.25),
		(-0.25, -1, 0.25, -0.75, -0.25, -1, 1),
		(-0.25, 0, -0.75, -0.5, 0, 0, -0.5),
		(-0.25, 0.75, -0.5, -1, -0.75, 1, 1),
		(-0.25, 0.75, 0.75, -0.75, -0.5, 0.75, 1),
		(-0.25, 1, 0.25, -0.75, -0.5, 0, 0),
		(-0.25, 1, 0.75, -0.25, -0.25, -0.25, 0.25),
		(0, -0.75, -0.25, -0.75, 0.25, -1, 0.25),
		(0, -0.25, -0.5, -0.25, -0.5, -1, -0.5),
		(0, -0.25, -0.5, -0.25, -0.25, 0.25, -0.75),
		(0, -0.25, 0, -0.5, -0.5, 0, 0),
		(0, 0, -0.75, -0.25, -0.25, 0.75, -0.75),
		(0, 0, 0.75, -1, -0.5, 0, -0.75),
		(0, 0.5, -1, -0.5, -0.5, 0.25, 0.5),
		(0, 0.75, -0.25, -0.5, 0, 0, -1),
		(0.25, 0, -1, -0.75, 0.25, 0.75, -0.25),
		(0.25, 0.25, -0.75, -0.75, 0, -1, 0.5),
		(0.25, 0.25, -0.25, -0.75, 0, 0, 0),
		(0.25, 0.25, 0, -0.25, 0, 0.75, 0.25),
		(0.25, 1, -1, -1, -1, -0.75, 0.5),
		(0.5, -1, -1, -1, 0, 0.25, 0.75),
		(0.5, -0.5, 0, -0.5, -0.25, -0.75, -1),
		(0.5, 0, -0.75, -0.25, 0, -0.5, 0),
		(0.5, 0.5, 1, -0.75, 0.25, 0, -0.25),
		(0.75, -1, -0.5, -0.75, -0.75, 0.25, -0.25),
		(0.75, -1, 0, -0.5, -0.75, 0, -1),
		(0.75, -0.75, 0.75, -0.5, 0, 0, 0),
		(0.75, -0.5, -0.5, -0.75, -1, 0, -1),
		(0.75, -0.25, -1, -0.5, -0.75, 0.75, -0.5),
		(0.75, -0.25, -0.5, -0.5, 0, 0.25, -0.25),
		(0.75, -0.25, 0.5, -0.5, 0, -0.25, -0.5),
		(0.75, 0, 0.5, -1, 0.5, 0.75, 0.5),
		(1, -1, 0, -0.5, 0.5, 0.75, -0.25),
		(1, -0.25, -0.75, -0.5, -0.5, 0.75, 0.25),
		(1, 0.5, 0, -1, 0.5, 1, 0.75),
		(1, 0.75, 0.25, -1, 1, -1, 0.5)
	)
	
	
	class Tank:
		# Используется только в этом классе, поэтому заменять self.sign на test_1623_utils__sign (и проч.) не так уж много смысла
		def sign(self, n):
			if n > 0:
				return 1
			elif n < 0:
				return -1
			return 0
		
		def get_from_level(self, level, x, y):
			return level[(y % test_1623__height) * test_1623__width + (x % test_1623__width)]
		def set_to_level(self, level, x, y, v):
			level[(y % test_1623__height) * test_1623__width + (x % test_1623__width)] = v
		
		team = 0
		exp = 0
		level = 1
		price_destroyed_walls = 0
		
		count_shots = 0
		count_steps = 0
		last_shot_time = 0
		last_step_time = 0
		last_shots_time = time.time() - 2
		last_steps_time = time.time() - 2
		last_action_time = time.time()
		
		bullets_spent = 0
		bullets_to_shorts = 5
		
		need_to_delete = False
		need_to_forward = False
		need_to_fair = False
		
		def __init__(self, is_bot, x, y):
			self.color = test_1623_tanks__colors[len(test_1623_tanks__players)]
			
			self.x, self.y = x, y
			self.to_x, self.to_y = x, y
			self.is_bot = is_bot
			self.start_time = 0
			
			self.rotation = 'left' if x < test_1623__width / 2 else 'right'
			self.dx = -1 if self.rotation == 'left' else 1
			self.dy = 0
			
			if test_1623_tanks__simpled:
				self.hp = 0 # У всех одна (последняя) жизнь
			else:
				self.hp = 6
			
			self.path_to_aim = None
			self.motivation = renpy.random.randint(40, 100)
			self.sleeping = renpy.random.randint(25, 50)
			
			self.gens = super_gens[renpy.random.randint(0, len(super_gens) - 1)]
		
		
		def update(self):
			if self.need_to_delete:
				return
			
			if self.is_bot:
				self.update_bot() # Во имя честности определяет лишь поворот и _необходимость_ ехать и стрелять, но не двигает и не стреляет сама
			
			# Для восстановления здоровья простоять неподвижно, не стреляя и не получая урона 3 секунды
			if test_1623_tanks__time - self.last_action_time > 3 and self.hp != 6 and not test_1623_tanks__simpled:
				self.hp += 1
				self.last_action_time = test_1623_tanks__time - 2 # после первого восстановления они будут происходить каждые 2 секунды
			
			if self.need_to_fair:
				self.need_to_fair = False
				self.fair()
			elif test_1623_tanks__time - self.last_shot_time > 1:
				self.count_shots = 0
			
			if self.need_to_forward:
				self.need_to_forward = False
				self.to_forward()
			elif test_1623_tanks__time - self.last_step_time > 1:
				self.count_steps = 0
			
		def update_bot(self):
			if renpy.random.randint(0, 100) < self.sleeping:
				return
			
			front_enemy_count = self.front_enemy_count()
			side_enemy_count = self.side_enemy_count()
		
			a = (self.hp						) * self.gens[0]
			b = (self.level - self.count_steps  ) * self.gens[1]
			c = (self.level - self.count_shots  ) * self.gens[2]
			d = (self.bullets_to_shorts		 ) * self.gens[3]
			e = (self.near_bullet_count()	   ) * self.gens[4]
			f = (front_enemy_count			  ) * self.gens[5]
			g = (side_enemy_count			   ) * self.gens[6]
			s = sum((a, b, c, d, e, f, g))
			s -= min_gens		# s >= 0
			s *= 5.0 / max_gens  # 0 <= s <= 5
			
			if front_enemy_count > 0 or side_enemy_count > 0:   # если есть враг, то есть только 2 пути:
				if s < 2.5:
					s = 1.5 if front_enemy_count > 0 else 2.5   # стрелять
				else:										   #  или
					s = 4.5									 # или уйти в оборону (убежать)
			
			action = None
			if s <= 0:
				if self.hp != 6 and not test_1623_tanks__simpled:
					action = 'waiting'
			elif s < 1:
				near_bullet = self.get_near_bullet()
				if near_bullet is not None:
					self.shot_to_bullet(near_bullet)
					action = 'shot_to_bullet'
			elif s < 2:
				if front_enemy_count > 0:
					self.rotate_to_front_enemy()
					self.need_to_fair = True
					action = 'fair'
			elif s < 3:
				if self.side_enemy_count() > 0:
					self.rotate_to_side_enemy()
					self.need_to_fair = True
					action = 'fair'
			elif s < 4:
				self.to_attack()
				action = 'to_attack'
			else:
				self.to_defence()
				action = 'to_defence'
			
			no_move = self.count_steps >= self.level and not (self.count_steps == 0 and test_1623_tanks__time - self.last_step_time < 1)
			
			if action is None and self.path_to_aim is None:
				self.to_training()
			elif self.path_to_aim is not None and (test_1623_tanks__simpled or not no_move):
				to_x, to_y = self.x, self.y
				
				if len(self.path_to_aim) > 0:
					to_x, to_y = self.path_to_aim[0]
					self.path_to_aim = self.path_to_aim[1:]
				if len(self.path_to_aim) == 0:
					self.path_to_aim = None
				
				self.dx, self.dy = to_x - self.x, to_y - self.y
				
				if self.dx != 0 or self.dy != 0:
					self.update_rotation()
					self.need_to_forward = True
		
		def update_rotation(self):
			if self.dy == 0:
				self.rotation = 'left' if self.dx == -1 else 'right'
			else:
				self.rotation = 'up' if self.dy == -1 else 'down'
		
		def shot_to_bullet(self, near_bullet):
			self.dx = self.sign(near_bullet[0] - self.x)
			self.dy = self.sign(near_bullet[1] - self.y)
			self.need_to_fair = True
		
		def check_enemy(self, x, y, dx, dy):
			tanks = [tank for tank in test_1623_tanks__players if (abs(tank.x - self.x) <= 1 or abs(tank.y - self.y) <= 1) and (tank.team != self.team)]
			
			i = 1
			while i < 15:
				i += 1
				
				x = (x + dx) % test_1623__width
				y = (y + dy) % test_1623__height
				if self.get_from_level(test_1623_tanks__walls, x, y) > 0:
					return False
				
				for tank in tanks:
					if x == tank.x and y == tank.y:
						return True
			return False
		
		def rotate_to_front_enemy(self):
			if self.check_enemy(self.x, self.y, -1, 0):
				self.dx, self.dy = -1, 0
			elif self.check_enemy(self.x, self.y, 1, 0):
				self.dx, self.dy = 1, 0
			elif self.check_enemy(self.x, self.y, 0, -1):
				self.dx, self.dy = 0, -1
			elif self.check_enemy(self.x, self.y, 0, 1):
				self.dx, self.dy = 0, 1
			
			self.update_rotation()
		
		def rotate_to_side_enemy(self):
			if self.check_enemy(self.x - 1, self.y - 1, -1, 0) or self.check_enemy(self.x - 1, self.y + 1, -1, 0):
				self.dx, self.dy = -1, 0
			elif self.check_enemy(self.x - 1, self.y - 1, 0, -1) or self.check_enemy(self.x + 1, self.y - 1, 0, -1):
				self.dx, self.dy = 0, -1
			elif self.check_enemy(self.x + 1, self.y - 1, 1, 0) or self.check_enemy(self.x + 1, self.y + 1, 1, 0):
				self.dx, self.dy = 1, 0
			elif self.check_enemy(self.x - 1, self.y + 1, 0, 1) or self.check_enemy(self.x + 1, self.y + 1, 0, 1):
				self.dx, self.dy = 0, 1
			
			self.update_rotation()
		
		def front_enemy_count(self):
			res = 0
			
			if self.check_enemy(self.x, self.y, -1, 0):
				res += 1
			elif self.check_enemy(self.x, self.y, 1, 0):
				res += 1
			elif self.check_enemy(self.x, self.y, 0, -1):
				res += 1
			elif self.check_enemy(self.x, self.y, 0, 1):
				res += 1
			
			return res
		
		def side_enemy_count(self):
			res = 0
			if self.check_enemy(self.x - 1, self.y - 1, -1, 0):
				res += 1
			if self.check_enemy(self.x - 1, self.y + 1, -1, 0):
				res += 1
			if self.check_enemy(self.x - 1, self.y - 1, 0, -1):
				res += 1
			if self.check_enemy(self.x + 1, self.y - 1, 0, -1):
				res += 1
			if self.check_enemy(self.x + 1, self.y - 1, 1, 0):
				res += 1
			if self.check_enemy(self.x + 1, self.y + 1, 1, 0):
				res += 1
			if self.check_enemy(self.x - 1, self.y + 1, 0, 1):
				res += 1
			if self.check_enemy(self.x + 1, self.y + 1, 0, 1):
				res += 1
			return res
		
		def get_near_bullet(self):
			near_dist = 1e3
			near_bullet = None
			
			for bullet in test_1623_tanks__bullets:
				if bullet.team == self.team:
					continue
				
				dx, dy = bullet.x - self.x, bullet.y - self.y
				if dx != 0 and dy != 0:
					continue
				
				prev_last_point = self.check_walls(bullet.x, bullet.y, bullet.dx, bullet.dy, bullet.hp)
				if prev_last_point is None:
					continue
				
				dist = dx + dy
				if dist < near_dist:
					near_dist = dist
					near_bullet = prev_last_point
			
			return near_bullet
		
		def near_bullet_count(self):
			res = 0
			
			for bullet in test_1623_tanks__bullets:
				if bullet.team == self.team:
					continue
				
				dx, dy = bullet.x - self.x, bullet.y - self.y
				if dx != 0 and dy != 0:
					continue
				
				prev_last_point = self.check_walls(bullet.x, bullet.y, bullet.dx, bullet.dy, bullet.hp)
				if prev_last_point is None:
					continue
				
				res += 1
			
			return res
		
		def check_walls(self, x, y, dx, dy, max_dist):
			while test_1623_tanks__walls[y * test_1623__width + x] <= 0 and max_dist > 0:
				x = (x + dx) % test_1623__width
				y = (y + dy) % test_1623__height
				max_dist -= 1
			
			if max_dist == 0:
				return None
			return x - self.x, y - self.y
		
		def fair(self):
			if (self.bullets_to_shorts <= 0 and not test_1623_tanks__simpled and test_1623_tanks__limit_bullets) or (time.time() - test_1623_tanks__start_level_time < 3):
				return
			
			if (test_1623_tanks__limit_bullets or self.team != test_1623_tanks__PLAYER) and not test_1623_tanks__simpled:
				if self.count_shots >= self.level:
					self.count_shots = 0
					self.last_shots_time = test_1623_tanks__time
					return
				if self.count_shots == 0 and test_1623_tanks__time - self.last_shots_time < 1:
					return
			
			self.last_shot_time = test_1623_tanks__time
			self.last_action_time = test_1623_tanks__time
			
			self.count_shots += 1
			self.bullets_spent += 1
			self.bullets_to_shorts = 10 * self.level + 2 * self.price_destroyed_walls - self.bullets_spent
			
			for bullet in test_1623_tanks__bullets:
				if bullet.x == self.x and bullet.y == self.y:
					return
			
			bullet = Bullet(self, self.x, self.y, self.dx, self.dy)
			test_1623_tanks__bullets.append(bullet)
		
		def to_forward(self):
			if not self.can_move():
				return
			
			if (test_1623_tanks__limit_moves or self.team != test_1623_tanks__PLAYER) and not test_1623_tanks__simpled:
				if self.count_steps >= self.level:
					self.count_steps = 0
					self.last_steps_time = test_1623_tanks__time
					return
				if self.count_steps == 0 and test_1623_tanks__time - self.last_steps_time < 1:
					return
			
			self.last_step_time = test_1623_tanks__time
			self.last_action_time = test_1623_tanks__time
			
			self.count_steps += 1
			self.to_x = (self.x + self.dx) % test_1623__width
			self.to_y = (self.y + self.dy) % test_1623__height
		
		def can_move(self):
			to_x = (self.x + self.dx) % test_1623__width
			to_y = (self.y + self.dy) % test_1623__height
			
			to_index = to_y * test_1623__width + to_x
			if test_1623_tanks__walls[to_index] != 0:
				return False
			
			for tank in test_1623_tanks__players:
				if tank is self:
					continue
				
				dx_range = (self.dx * 3,   ) if self.dx != 0 else (-2, -1, 0, 1, 2)
				dy_range = (-2, -1, 0, 1, 2) if self.dx != 0 else (self.dy * 3,   )
				
				for dy in dy_range:
					for dx in dx_range:
						x = (self.x + dx) % test_1623__width
						y = (self.y + dy) % test_1623__height
						
						if x == tank.x and y == tank.y:
							return False
			
			return True
		
		def add_exp(self, exp, is_wall = False):
			self.exp += exp
			if is_wall:
				self.price_destroyed_walls += exp
			
			self.level = 1
			while 2 ** self.level <= self.exp:
				self.level += 1
			
			self.bullets_to_shorts = 10 * self.level + 2 * self.price_destroyed_walls - self.bullets_spent
		
		def level_to_map(self, level, to_x = -1, to_y = -1, max_step = -1):
			step = 0
			changed = True
			while changed and (step <= max_step if max_step != -1 else self.get_from_level(level, to_x, to_y) == 0):
				step += 1
				changed = False
				
				for y in xrange(test_1623__height):
					for x in xrange(test_1623__width):
						if self.get_from_level(level, x, y) != step:
							continue
						
						nears = (
							(x - 1, y),
							(x + 1, y),
							(x, y - 1),
							(x, y + 1)
						)
						
						for near in nears:
							v = self.get_from_level(level, near[0], near[1])
							if v == 0 or v > step:
								self.set_to_level(level, near[0], near[1], step + 1)
								changed = True
			
			return step - 1 if max_step != -1 else step
		
		def get_path(self, level, x, y):
			res = [(x, y)]
			step = self.get_from_level(level, x, y) - 1
			
			while step != 1:
				if self.get_from_level(level, x - 1, y) == step:
					x = (x - 1) % test_1623__width
				elif self.get_from_level(level, x + 1, y) == step:
					x = (x + 1) % test_1623__width
				elif self.get_from_level(level, x, y - 1) == step:
					y = (y - 1) % test_1623__height
				elif self.get_from_level(level, x, y + 1) == step:
					y = (y + 1) % test_1623__height
				else:
					return []
				
				t = (x, y)
				res.insert(0, t)
				
				step -= 1
			
			return res
		
		def to_attack(self):
			level = [0 if v == 0 else -1 for v in test_1623_tanks__walls]
			
			for tank in test_1623_tanks__players:
				if tank.team != self.team:
					self.set_to_level(level, tank.x, tank.y, 1)
			
			self.level_to_map(level, self.x, self.y)
			
			self.path_to_aim = self.get_path(level, self.x, self.y)
			self.path_to_aim.reverse()
			self.path_to_aim = self.path_to_aim[1:]
		
		def to_defence(self):
			level = [0 if v == 0 else -1 for v in test_1623_tanks__walls]
			
			def fill_side(level, x, y, dx, dy):
				i = 0
				m = max(test_1623__width, test_1623__height)
				while i < m:
					i += 1
					x += dx
					y += dy
					
					if self.get_from_level(level, x, y) != -1:
						_dx = self.x - x
						_dy = self.y - y
						self.set_to_level(level, x, y, 1)
					else:
						break
			
			for tank in test_1623_tanks__players:
				if tank.team != self.team:
					fill_side(level, tank.x, tank.y, -1, 0)
					fill_side(level, tank.x, tank.y, 1, 0)
					fill_side(level, tank.x, tank.y, 0, -1)
					fill_side(level, tank.x, tank.y, 0, 1)
			
			max_step = self.level_to_map(level, max_step = 25)
			
			if max_step <= 3:
				self.to_attack()
				return
			
			for y in xrange(test_1623__height):
				for x in xrange(test_1623__width):
					index = y * test_1623__width + x
					if level[index] == max_step:
						to_x, to_y = x, y
			
			
			level = [0 if v == 0 else -1 for v in test_1623_tanks__walls]
			self.set_to_level(level, self.x, self.y, 1)
			self.level_to_map(level, to_x, to_y)
			
			self.path_to_aim = self.get_path(level, to_x, to_y)
		
		def to_training(self):
			for v in test_1623_tanks__walls:
				if v != 0:
					break
			else:
				self.to_attack()
				return
			
			if self.check_walls(self.x, self.y, -1, 0, 15) is not None:
				self.dx, self.dy, self.need_to_fair = -1, 0, True
			elif self.check_walls(self.x, self.y, 1, 0, 15) is not None:
				self.dx, self.dy, self.need_to_fair = 1, 0, True
			elif self.check_walls(self.x, self.y, 0, -1, 15) is not None:
				self.dx, self.dy, self.need_to_fair = 0, -1, True
			elif self.check_walls(self.x, self.y, 0, 1, 15) is not None:
				self.dx, self.dy, self.need_to_fair = 0, 1, True
			else:
				sides = [(-1, 0), (0, -1), (1, 0), (0, 1)]
				renpy.random.shuffle(sides)
				for i in xrange(4):
					self.dx, self.dy = sides[i]
					if self.can_move():
						break
				self.need_to_forward = True
			
			self.update_rotation()

