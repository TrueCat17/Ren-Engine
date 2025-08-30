init -1 python:
	
	def tetris__tanks_flood_level(level, to_x = -1, to_y = -1, max_step = -1):
		opened = set()
		for y in range(tetris.height):
			line = level[y]
			for x in range(tetris.width):
				if line[x] == 1:
					opened.add((x, y))
		
		next_opened = set()
		closed = set()
		
		step = 2
		while (step <= max_step) if max_step != -1 else (level[to_y][to_x] == 0):
			for pos in opened:
				closed.add(pos)
				
				x, y = pos
				
				for dx, dy in tetris.sides:
					near = tetris.normal_pos(x + dx, y + dy)
					
					if near in opened or near in next_opened or near in closed:
						continue
					
					near_x, near_y = near
					if level[near_y][near_x] != 0:
						continue
					
					next_opened.add(near)
					level[near_y][near_x] = step
			
			if not next_opened:
				return
			
			opened = next_opened
			next_opened = set()
			
			step += 1
			if step > 10000:
				return
	
	def tetris__tanks_get_path(level, x, y):
		res = [(x, y)]
		step = level[y][x] - 1
		
		while step != 0:
			for dx, dy in tetris.sides:
				nx, ny = tetris.normal_pos(x + dx, y + dy)
				
				if level[ny][nx] == step:
					x, y = nx, ny
					res.insert(0, (x, y))
					step -= 1
					break
			else:
				return None
		
		return res
	
	
	
	class TetrisTank:
		def __init__(self, x, y, team):
			self.last_shot_time = 0
			self.last_step_time = 0
			self.last_action_time = get_game_time()
			
			self.need_to_delete = False
			self.need_to_forward = False
			self.need_to_fire = False
			
			self.color = tetris.colors[len(tetris.tanks_players)]
			
			self.x, self.y, self.team = x, y, team
			self.to_x, self.to_y = x, y
			self.is_bot = True
			self.start_time = get_game_time()
			
			self.rotation = 'left' if x > tetris.width // 2 else 'right'
			self.dx = -1 if self.rotation == 'left' else 1
			self.dy = 0
			
			self.hp = 6
			self.max_hp = self.hp
			self.min_hp_to_war = random.randint(0, self.max_hp - 1)
			
			is_friend = team == 0
			self.sleep_chance = 0.2 if is_friend else 0.7
		
		
		def update(self):
			if self.need_to_delete:
				return
			
			if self.is_bot:
				self.update_bot()
			
			if self.hp != self.max_hp and get_game_time() - self.last_action_time > tetris.tanks_WAIT_FOR_HP_START:
				self.last_action_time = get_game_time() - tetris.tanks_WAIT_FOR_HP_START + tetris.tanks_WAIT_FOR_HP_NEXT
				self.hp += 1
			
			if self.need_to_fire:
				self.need_to_fire = False
				self.fire()
			
			if self.need_to_forward:
				self.need_to_forward = False
				self.to_forward()
		
		def update_bot(self):
			if random.random() < self.sleep_chance:
				return
			
			path = None
			action = None
			
			if self.hp < self.min_hp_to_war:
				if self.danger():
					path = self.to_defence()
					action = 'to_defence'
				else:
					action = 'waiting'
			
			if not action:
				near_bullet = self.get_near_bullet()
				if near_bullet:
					self.shoot_to_bullet(near_bullet)
					action = 'shoot_to_bullet'
			
			if not action:
				if self.front_enemy() or self.side_enemy():
					action = 'front_or_side_enemy'
				else:
					path = self.to_enemy()
					action = 'to_enemy'
			
			if path:
				to_x, to_y = path[0]
				self.dx, self.dy = to_x - self.x, to_y - self.y
				
				if self.dx or self.dy:
					self.update_rotation()
					self.need_to_forward = True
		
		
		def get_simple_level(self):
			level = tetris.tanks_walls_with_borders
			
			res = []
			for y in range(tetris.height):
				line = [-1 if v else 0 for v in level[y]]
				res.append(line)
			
			for tank in tetris.tanks_players:
				if tank is self:
					continue
				if tank.team != self.team:
					continue
				
				for dy in (-1, 0, +1):
					for dx in (-1, 0, +1):
						x, y = tetris.normal_pos(tank.x + dx, tank.y + dy)
						res[y][x] = -1
			
			return res
		
		
		def update_rotation(self):
			if self.dy == 0:
				self.rotation = 'left' if self.dx == -1 else 'right'
			else:
				self.rotation =   'up' if self.dy == -1 else 'down'
		
		def shoot_to_bullet(self, near_bullet):
			self.dx = -near_bullet.dx
			self.dy = -near_bullet.dy
			self.update_rotation()
			self.need_to_fire = True
		
		def check_enemy(self, x, y, dx, dy):
			tanks = [tank for tank in tetris.tanks_players if tank.team != self.team]
			
			i = 1
			max_dist = tetris['width' if dx else 'height']
			while i < max_dist:
				i += 1
				
				x, y = tetris.normal_pos(x + dx, y + dy)
				if tetris.tanks_walls[y][x] != 0:
					return False
				
				for tank in tanks:
					if x == tank.x and y == tank.y:
						return True
			return False
		
		def danger(self):
			params = (
				(-1, -1, 0, -1),
				( 0, -1, 0, -1),
				(+1, -1, 0, -1),
				
				(+1, -1, +1, 0),
				(+1,  0, +1, 0),
				(+1, +1, +1, 0),
				
				(+1, +1, 0, +1),
				( 0, +1, 0, +1),
				(-1, +1, 0, +1),
				
				(-1, -1, -1, 0),
				(-1,  0, -1, 0),
				(-1, +1, -1, 0),
			)
			
			for dx, dy, to_x, to_y in params:
				if self.check_enemy(self.x + dx, self.y + dy, to_x, to_y):
					return True
			
			return False
		
		def front_enemy(self):
			for dx, dy in tetris.sides:
				if self.check_enemy(self.x, self.y, dx, dy):
					self.dx, self.dy = dx, dy
					self.update_rotation()
					self.need_to_fire = True
					return True
			
			return False
		
		def side_enemy(self):
			for dx, dy in tetris.sides:
				self.dx, self.dy = dx, dy
				if not self.can_move():
					continue
				
				if dx:
					for dy in (-1, +1):
						if self.check_enemy(self.x + dx, self.y, 0, dy):
							self.update_rotation()
							self.need_to_forward = True
							return True
				else:
					for dx in (-1, +1):
						if self.check_enemy(self.x, self.y + dy, dx, 0):
							self.update_rotation()
							self.need_to_forward = True
							return True
			
			self.dx = self.dy = 0
			return False
		
		def get_near_bullet(self):
			near_dist2 = 1e6
			near_bullet = None
			
			# center of self, not all 9 cells
			self_pos = ((self.x, self.y), )
			
			for bullet in tetris.tanks_bullets:
				if bullet.team == self.team:
					continue
				
				if self.x != bullet.x and self.y != bullet.y:
					continue
				
				last_point = self.bullet_last_point(bullet.x, bullet.y, bullet.dx, bullet.dy, bullet.hp, self_pos)
				if last_point not in self_pos:
					continue
				
				dx, dy = self.x - bullet.x, self.y - bullet.y
				dist2 = dx * dx + dy * dy
				if dist2 < near_dist2:
					near_dist2 = dist2
					near_bullet = bullet
			
			return near_bullet
		
		def bullet_last_point(self, x, y, dx, dy, max_dist, enemy_cells = ()):
			while tetris.tanks_walls[y][x] == 0:
				x, y = tetris.normal_pos(x + dx, y + dy)
				
				if (x, y) in enemy_cells:
					return (x, y)
				
				max_dist -= 1
				if max_dist == 0:
					return None
			
			return x, y
		
		def fire(self):
			self.last_shot_time = get_game_time()
			self.last_action_time = get_game_time()
			
			bullet = TetrisTankBullet(self.team, self.x, self.y, self.dx, self.dy)
			tetris.tanks_bullets.append(bullet)
		
		def to_forward(self):
			if self.can_move():
				self.last_step_time = get_game_time()
				self.last_action_time = get_game_time()
				
				self.to_x, self.to_y = tetris.normal_pos(self.x + self.dx, self.y + self.dy)
		
		def can_move(self):
			to_x, to_y = tetris.normal_pos(self.x + self.dx, self.y + self.dy)
			
			if tetris.tanks_walls_with_borders[to_y][to_x] != 0:
				return False
			
			dx_range = (self.dx * 3,   ) if self.dx else (-2, -1, 0, 1, 2)
			dy_range = (-2, -1, 0, 1, 2) if self.dx else (self.dy * 3,   )
			
			for tank in tetris.tanks_players:
				if tank is self:
					continue
				
				for dy in dy_range:
					for dx in dx_range:
						if tetris.normal_pos(self.x + dx, self.y + dy) == (tank.x, tank.y):
							return False
			return True
		
		def to_enemy(self):
			enemy_cells = []
			for tank in tetris.tanks_players:
				if tank.team == self.team:
					continue
				
				for dy in (-1, 0, +1):
					for dx in (-1, 0, +1):
						x, y = tetris.normal_pos(tank.x + dx, tank.y + dy)
						enemy_cells.append((x, y))
			
			for dx, dy in tetris.sides:
				m = tetris['width' if dx else 'height']
				last_point = self.bullet_last_point(self.x, self.y, dx, dy, m, enemy_cells)
				if last_point not in enemy_cells:
					continue
				
				self.dx, self.dy = dx, dy
				self.update_rotation()
				self.need_to_fire = True
				return
			
			
			level = self.get_simple_level()
			
			for tank in tetris.tanks_players:
				if tank.team != self.team:
					level[tank.y][tank.x] = 1
			
			tetris.tanks_flood_level(level, self.x, self.y)
			if level[self.y][self.x] <= 0:
				self.destroy_walls()
				return
			
			path = tetris.tanks_get_path(level, self.x, self.y)
			if path:
				path.pop()
				path.reverse()
			return path
		
		def to_defence(self):
			level = self.get_simple_level()
			
			def fill_side(x, y, dx, dy):
				for i in range(tetris['width' if dx else 'height']):
					level[y][x] = 1
					
					x, y = tetris.normal_pos(x + dx, y + dy)
					if tetris.tanks_walls[y][x] != 0:
						return
			
			for tank in tetris.tanks_players:
				if tank.team != self.team:
					fill_side(tank.x, tank.y, -1,  0)
					fill_side(tank.x, tank.y, +1,  0)
					fill_side(tank.x, tank.y,  0, -1)
					fill_side(tank.x, tank.y,  0, +1)
			
			tetris.tanks_flood_level(level, max_step = tetris.width + tetris.height)
			
			drange = (-3, -2, -1, 0, +1, +2, +3)
			near = []
			for dy in drange:
				for dx in drange:
					x, y = tetris.normal_pos(self.x + dx, self.y + dy)
					near.append((x, y))
			
			for y in range(tetris.height):
				line = level[y]
				for x in range(tetris.width):
					if (x, y) in near:
						continue
					if line[x] < 3:
						line[x] = 0
			
			opened = set([(self.x, self.y)])
			closed = set()
			while opened:
				x, y = pos = opened.pop()
				closed.add(pos)
				
				for dx, dy in tetris.sides:
					nx, ny = tetris.normal_pos(x + dx, y + dy)
					pos = (nx, ny)
					
					if pos in opened or pos in closed:
						continue
					
					if level[ny][nx] > 0:
						opened.add(pos)
			
			max_step = 0
			for y in range(tetris.height):
				line = level[y]
				for x in range(tetris.width):
					if (x, y) not in closed:
						line[x] = 0
					else:
						v = line[x]
						if v > max_step:
							max_step = v
							to_x, to_y = (x, y)
			
			if max_step <= 3:
				self.to_enemy()
				return
			
			level = self.get_simple_level()
			level[self.y][self.x] = 1
			tetris.tanks_flood_level(level, to_x, to_y)
			path = tetris.tanks_get_path(level, to_x, to_y)
			if path:
				path.pop(0)
			return path
		
		def destroy_walls(self):
			for line in tetris.tanks_walls:
				no_walls = True
				for v in line:
					if v != 0:
						no_walls = False
						break
				if not no_walls:
					break
			else:
				return
			
			for dx, dy in tetris.sides:
				dist = round(tetris['width' if dx else 'height'] / 2)
				
				if self.bullet_last_point(self.x, self.y, dx, dy, dist):
					self.dx, self.dy = dx, dy
					self.need_to_fire = True
					break
			else:
				for side in random.sample(tetris.sides, 4):
					self.dx, self.dy = side
					if self.can_move():
						self.need_to_forward = True
						break
				else:
					self.dx = self.dy = 0
					return
			
			self.update_rotation()
