init -1 python:
	class Bullet:
		need_to_delete = False
		
		def __init__(self, team, x, y, dx, dy):
			self.team, self.x, self.y, self.dx, self.dy = team, x, y, dx, dy
			self.to_x, self.to_y = x + dx, y + dy
			self.hp = tg_width if dx else tg_height
		
		def update(self):
			if self.need_to_delete:
				return
			
			self.hp -= 1
			if self.hp == 0:
				self.need_to_delete = True
				return
			
			to_x = (self.x + self.dx) % tg_width
			to_y = (self.y + self.dy) % tg_height
			
			to_index = to_y * tg_width + to_x
			
			wall_hp = tg_tanks_walls[to_index]
			self.need_to_delete = wall_hp > 0
			if self.need_to_delete:
				tg_tanks_walls[to_index] -= 1
				if wall_hp == 1:
					tg_tanks_update_walls()
				return
			
			for tank in tg_tanks_players:
				if tank.team == self.team or time.time() - tank.start_time < tg_tanks_NO_DAMAGE:
					continue
				
				dx, dy = abs(to_x - tank.x), abs(to_y - tank.y)
				if dx <= 1 and dy <= 1:
					self.need_to_delete = True
					tank.last_action_time = time.time()
					
					if tank.hp > 0:
						tank.hp -= 1
					else:
						tank.need_to_delete = True
					return
			
			for bullet in tg_tanks_bullets:
				if bullet.team == self.team:
					continue
				
				bx, by, bdx, bdy = bullet.x, bullet.y, bullet.dx, bullet.dy
				sx, sy, sdx, sdy = self.x, self.y, self.dx, self.dy
				
				
				in_1_cell_now = bx == sx and by == sy
				
				opposite_directions = bdx == -sdx and bdy == -sdy
				in_1_cell_next = bx == to_x and by == to_y
				
				collision = in_1_cell_now or (opposite_directions and in_1_cell_next)
				if collision:
					self.need_to_delete = True
					bullet.need_to_delete = True
					return
			
			self.to_x, self.to_y = to_x, to_y
