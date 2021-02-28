init -1 python:
	
	def tg_tanks_get_from_level(level, x, y):
		return level[(y % tg_height) * tg_width + (x % tg_width)]
	def tg_tanks_set_to_level(level, x, y, v):
		level[(y % tg_height) * tg_width + (x % tg_width)] = v
	
	
	def tg_tanks_level_to_map(level, to_x = -1, to_y = -1, max_step = -1):
		step = 0
		while step <= max_step if max_step != -1 else tg_tanks_get_from_level(level, to_x, to_y) == 0:
			step += 1
			changed = False
			
			for y in xrange(tg_height):
				for x in xrange(tg_width):
					if tg_tanks_get_from_level(level, x, y) != step:
						continue
					
					nears = (
						(x - 1, y),
						(x + 1, y),
						(x, y - 1),
						(x, y + 1)
					)
					
					for near in nears:
						v = tg_tanks_get_from_level(level, near[0], near[1])
						if v == 0 or v > step:
							tg_tanks_set_to_level(level, near[0], near[1], step + 1)
							changed = True
			if not changed:
				break
		return step - 1
	
	def tg_tanks_get_path(level, x, y):
		res = [(x, y)]
		step = tg_tanks_get_from_level(level, x, y) - 1
		
		while step != 1:
			if tg_tanks_get_from_level(level, x - 1, y) == step:
				x = (x - 1) % tg_width
			elif tg_tanks_get_from_level(level, x + 1, y) == step:
				x = (x + 1) % tg_width
			elif tg_tanks_get_from_level(level, x, y - 1) == step:
				y = (y - 1) % tg_height
			elif tg_tanks_get_from_level(level, x, y + 1) == step:
				y = (y + 1) % tg_height
			else:
				break
			
			res.insert(0, (x, y))
			step -= 1
		
		return res
	
	
	
	class Tank:
		last_shot_time = 0
		last_step_time = 0
		last_action_time = get_game_time()
		
		need_to_delete = False
		need_to_forward = False
		need_to_fire = False
		
		def __init__(self, x, y, team):
			self.color = tg_colors[len(tg_tanks_players)]
			
			self.x, self.y, self.team = x, y, team
			self.to_x, self.to_y = x, y
			self.is_bot = True
			self.start_time = get_game_time()
			
			self.rotation = 'left' if x > tg_width / 2 else 'right'
			self.dx = -1 if self.rotation == 'left' else 1
			self.dy = 0
			
			self.hp = 6
			self.max_hp = self.hp
			self.min_hp_to_war = random.randint(0, self.max_hp - 1)
			
			self.path_to_aim = None
			
			is_friend = tg_tanks_player and tg_tanks_player.team == team
			self.sleeping = 0.2 if is_friend else 0.8
		
		
		def update(self):
			if self.need_to_delete:
				return
			
			if self.is_bot:
				self.update_bot()
			
			if self.hp != self.max_hp and get_game_time() - self.last_action_time > tg_tanks_WAIT_FOR_HP_START:
				self.last_action_time = get_game_time() - tg_tanks_WAIT_FOR_HP_START + tg_tanks_WAIT_FOR_HP_NEXT
				self.hp += 1
			
			if self.need_to_fire:
				self.need_to_fire = False
				self.fire()
			
			if self.need_to_forward:
				self.need_to_forward = False
				self.to_forward()
		
		def update_bot(self):
			if random.random() < self.sleeping:
				return
			
			count_front_enemies = self.count_front_enemies()
			count_side_enemies = self.count_side_enemies()
			
			action = None
			if self.hp < self.min_hp_to_war:
				if count_front_enemies or count_side_enemies:
					self.path_to_aim = None
					self.to_defence()
					action = 'to_defence'
				else:
					self.path_to_aim = None
					action = 'waiting'
			
			if not action:
				near_bullet = self.get_near_bullet()
				if near_bullet:
					self.path_to_aim = None
					self.shoot_to_bullet(near_bullet)
					action = 'shoot_to_bullet'
			
			if not action:
				if count_front_enemies:
					self.path_to_aim = None
					self.shoot_to_front_enemy()
					action = 'fire'
				elif count_side_enemies:
					self.path_to_aim = None
					self.shoot_to_side_enemy()
					action = 'fire'
			
			if not action:
				self.path_to_aim = None
				self.to_attack()
				action = 'to_attack'
			
			if action is None and self.path_to_aim is None:
				self.destroy_walls()
			elif self.path_to_aim is not None:
				to_x, to_y = self.x, self.y
				
				if len(self.path_to_aim) > 0:
					to_x, to_y = self.path_to_aim[0]
					self.path_to_aim.pop()
				if len(self.path_to_aim) == 0:
					self.path_to_aim = None
				
				self.dx, self.dy = to_x - self.x, to_y - self.y
				
				if self.dx or self.dy:
					self.update_rotation()
					self.need_to_forward = True
		
		def update_rotation(self):
			if self.dy == 0:
				self.rotation = 'left' if self.dx == -1 else 'right'
			else:
				self.rotation = 'up' if self.dy == -1 else 'down'
		
		def shoot_to_bullet(self, near_bullet):
			self.dx = sign(near_bullet[0])
			self.dy = sign(near_bullet[1])
			self.update_rotation()
			self.need_to_fire = True
		
		def check_enemy(self, x, y, dx, dy):
			tanks = [tank for tank in tg_tanks_players if (abs(tank.x - self.x) <= 1 or abs(tank.y - self.y) <= 1) and (tank.team != self.team)]
			
			i = 1
			max_dist = tg_width if dx else tg_height
			while i < max_dist:
				i += 1
				
				x = (x + dx) % tg_width
				y = (y + dy) % tg_height
				if tg_tanks_get_from_level(tg_tanks_walls, x, y) > 0:
					return False
				
				for tank in tanks:
					if x == tank.x and y == tank.y:
						return True
			return False
		
		def shoot_to_front_enemy(self):
			if   self.check_enemy(self.x, self.y, -1,  0):
				self.dx, self.dy = -1,  0
			elif self.check_enemy(self.x, self.y, +1,  0):
				self.dx, self.dy = +1,  0
			elif self.check_enemy(self.x, self.y,  0, -1):
				self.dx, self.dy =  0, -1
			elif self.check_enemy(self.x, self.y,  0, +1):
				self.dx, self.dy =  0, +1
			
			self.update_rotation()
			self.need_to_fire = True
		
		def shoot_to_side_enemy(self):
			if   self.check_enemy(self.x - 1, self.y + 1, -1,  0) or self.check_enemy(self.x - 1, self.y - 1, -1,  0):
				self.dx, self.dy = -1, 0
			elif self.check_enemy(self.x - 1, self.y - 1,  0, -1) or self.check_enemy(self.x + 1, self.y - 1,  0, -1):
				self.dx, self.dy = 0, -1
			elif self.check_enemy(self.x + 1, self.y - 1, +1,  0) or self.check_enemy(self.x + 1, self.y + 1, +1,  0):
				self.dx, self.dy = 1, 0
			elif self.check_enemy(self.x + 1, self.y + 1,  0, +1) or self.check_enemy(self.x - 1, self.y + 1,  0, +1):
				self.dx, self.dy = 0, 1
			
			self.update_rotation()
			self.need_to_fire = True
		
		def count_front_enemies(self):
			res = 0
			if self.check_enemy(self.x, self.y, -1,  0):
				res += 1
			if self.check_enemy(self.x, self.y, +1,  0):
				res += 1
			if self.check_enemy(self.x, self.y,  0, -1):
				res += 1
			if self.check_enemy(self.x, self.y,  0, +1):
				res += 1
			return res
		
		def count_side_enemies(self):
			res = 0
			if self.check_enemy(self.x - 1, self.y + 1, -1,  0):
				res += 1
			if self.check_enemy(self.x - 1, self.y - 1, -1,  0):
				res += 1
			if self.check_enemy(self.x - 1, self.y - 1,  0, -1):
				res += 1
			if self.check_enemy(self.x + 1, self.y - 1,  0, -1):
				res += 1
			if self.check_enemy(self.x + 1, self.y - 1, +1,  0):
				res += 1
			if self.check_enemy(self.x + 1, self.y + 1, +1,  0):
				res += 1
			if self.check_enemy(self.x + 1, self.y + 1,  0, +1):
				res += 1
			if self.check_enemy(self.x - 1, self.y + 1,  0, +1):
				res += 1
			return res
		
		def get_near_bullet(self):
			near_dist = 1e6
			near_bullet = None
			
			for bullet in tg_tanks_bullets:
				if bullet.team == self.team:
					continue
				
				dx, dy = self.x - bullet.x, self.y - bullet.y
				if (dx and dy) or (bullet.dx != dx and bullet.dy != dy):
					continue
				
				last_point = self.bullet_last_point(bullet.x, bullet.y, bullet.dx, bullet.dy, bullet.hp)
				if last_point is None:
					continue
				
				dx, dy = last_point
				dist = dx * dx + dy * dy
				if dist < near_dist:
					near_dist = dist
					near_bullet = last_point
			
			return near_bullet
		
		def bullet_last_point(self, x, y, dx, dy, max_dist):
			while tg_tanks_walls[y * tg_width + x] == 0:
				x = (x + dx) % tg_width
				y = (y + dy) % tg_height
				max_dist -= 1
				if max_dist == 0:
					return None
			
			dx, dy = (x - self.x) % tg_width, (y - self.y) % tg_height
			if dx > tg_width / 2:
				dx -= tg_width
			if dy > tg_height / 2:
				dy -= tg_height
			return dx, dy
		
		def fire(self):
			self.last_shot_time = get_game_time()
			self.last_action_time = get_game_time()
			
			bullet = Bullet(self.team, self.x, self.y, self.dx, self.dy)
			tg_tanks_bullets.append(bullet)
		
		def to_forward(self):
			if self.can_move():
				self.last_step_time = get_game_time()
				self.last_action_time = get_game_time()
				
				self.to_x = (self.x + self.dx) % tg_width
				self.to_y = (self.y + self.dy) % tg_height
		
		def can_move(self):
			to_x = (self.x + self.dx) % tg_width
			to_y = (self.y + self.dy) % tg_height
			
			to_index = to_y * tg_width + to_x
			if tg_tanks_walls[to_index] != 0:
				return False
			
			for tank in tg_tanks_players:
				if tank is self:
					continue
				
				dx_range = (self.dx * 3,   ) if self.dx else (-2, -1, 0, 1, 2)
				dy_range = (-2, -1, 0, 1, 2) if self.dx else (self.dy * 3,   )
				
				for dy in dy_range:
					for dx in dx_range:
						x = (self.x + dx) % tg_width
						y = (self.y + dy) % tg_height
						
						if x == tank.x and y == tank.y:
							return False
			return True
		
		def to_attack(self):
			level = [0 if v == 0 else -1 for v in tg_tanks_walls]
			
			for tank in tg_tanks_players:
				if tank.team != self.team:
					tg_tanks_set_to_level(level, tank.x, tank.y, 1)
			
			tg_tanks_level_to_map(level, self.x, self.y)
			
			if tg_tanks_get_from_level(level, self.x, self.y) <= 0:
				self.destroy_walls()
				return
			
			self.path_to_aim = tg_tanks_get_path(level, self.x, self.y)
			self.path_to_aim.pop()
			self.path_to_aim.reverse()
		
		def to_defence(self):
			level = [0 if v == 0 else -1 for v in tg_tanks_walls]
			
			def fill_side(level, x, y, dx, dy):
				i = 0
				m = tg_width if dx else tg_height
				while i < m:
					i += 1
					x += dx
					y += dy
					
					if tg_tanks_get_from_level(level, x, y) != -1:
						tg_tanks_set_to_level(level, x, y, 1)
					else:
						break
			
			for tank in tg_tanks_players:
				if tank.team != self.team:
					fill_side(level, tank.x, tank.y, -1,  0)
					fill_side(level, tank.x, tank.y, +1,  0)
					fill_side(level, tank.x, tank.y,  0, -1)
					fill_side(level, tank.x, tank.y,  0, +1)
			
			max_step = tg_tanks_level_to_map(level, max_step = 25)
			if max_step <= 3:
				self.to_attack()
				return
			
			for y in xrange(tg_height):
				for x in xrange(tg_width):
					index = y * tg_width + x
					if level[index] == max_step:
						to_x, to_y = x, y
			
			level = [0 if v == 0 else -1 for v in tg_tanks_walls]
			tg_tanks_set_to_level(level, self.x, self.y, 1)
			tg_tanks_level_to_map(level, to_x, to_y)
			
			self.path_to_aim = tg_tanks_get_path(level, to_x, to_y)
		
		def destroy_walls(self):
			for v in tg_tanks_walls:
				if v != 0:
					break
			else:
				self.to_attack()
				return
			
			if   self.bullet_last_point(self.x, self.y, -1, 0, (tg_width + 1) / 2):
				self.dx, self.dy, self.need_to_fire = -1, 0, True
			elif self.bullet_last_point(self.x, self.y, +1, 0, (tg_width + 1) / 2):
				self.dx, self.dy, self.need_to_fire = 1, 0, True
			elif self.bullet_last_point(self.x, self.y, 0, -1, (tg_height + 1) / 2):
				self.dx, self.dy, self.need_to_fire = 0, -1, True
			elif self.bullet_last_point(self.x, self.y, 0, +1, (tg_height + 1) / 2):
				self.dx, self.dy, self.need_to_fire = 0, 1, True
			else:
				sides = [(-1, 0), (0, -1), (1, 0), (0, 1)]
				random.shuffle(sides)
				for i in xrange(4):
					self.dx, self.dy = sides[i]
					if self.can_move():
						break
				self.need_to_forward = True
			
			self.update_rotation()

