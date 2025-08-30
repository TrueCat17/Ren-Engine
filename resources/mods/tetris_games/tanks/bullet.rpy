init python:
	class TetrisTankBullet:
		def __init__(self, team, x, y, dx, dy):
			self.need_to_delete = False
			self.team, self.x, self.y, self.dx, self.dy = team, x, y, dx, dy
			self.to_x, self.to_y = tetris.normal_pos(x + dx, y + dy)
			self.hp = tetris['width' if dx else 'height']
		
		def update(self):
			if self.need_to_delete:
				return
			
			self.hp -= 1
			if self.hp == 0:
				self.need_to_delete = True
				return
			
			to_x, to_y = tetris.normal_pos(self.x + self.dx, self.y + self.dy)
			
			wall_hp = tetris.tanks_walls[to_y][to_x]
			self.need_to_delete = wall_hp > 0
			if self.need_to_delete:
				tetris.tanks_walls[to_y][to_x] -= 1
				if wall_hp == 1:
					tetris.tanks_update_walls_with_borders()
				return
			
			for tank in tetris.tanks_players:
				if tank.team == self.team or get_game_time() - tank.start_time < tetris.tanks_NO_DAMAGE:
					continue
				
				tank_x, tank_y = tank.x, tank.y
				
				for dy in (-1, 0, +1):
					for dx in (-1, 0, +1):
						x, y = tetris.normal_pos(tank_x + dx, tank_y + dy)
						
						if x == to_x and y == to_y:
							self.need_to_delete = True
							tank.last_action_time = get_game_time()
							
							if tank.hp > 0:
								tank.hp -= 1
							else:
								tank.need_to_delete = True
							return
			
			x, y, dx, dy = self.x, self.y, self.dx, self.dy
			for bullet in tetris.tanks_bullets:
				if bullet.team == self.team:
					continue
				
				bx, by, bdx, bdy = bullet.x, bullet.y, bullet.dx, bullet.dy
				
				in_1_cell_now = bx == x and by == y
				
				opposite_directions = bdx == -dx and bdy == -dy
				in_1_cell_next = bx == to_x and by == to_y
				
				collision = in_1_cell_now or (opposite_directions and in_1_cell_next)
				if collision:
					self.need_to_delete = True
					bullet.need_to_delete = True
					return
			
			self.to_x, self.to_y = to_x, to_y
