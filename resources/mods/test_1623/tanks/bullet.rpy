init -1 python:
	class Bullet:
		need_to_delete = False
		
		def __init__(self, tank, x, y, dx, dy):
			self.tank, self.team, self.x, self.y, self.dx, self.dy = tank, tank.team, x, y, dx, dy
			self.to_x, self.to_y = x + dx, y + dy
			if test_1623_tanks__simpled:
				self.hp = max(test_1623__width, test_1623__height) * 2  # Аналог бесконечности...
			else:
				self.hp = min(test_1623__width, test_1623__height) # Не выносить из конструктора, иначе она всегда = 1 (возможно, инициализируется сразу после запуска игры)
		
		def update(self):
			if self.need_to_delete:
				return
			
			self.hp -= 1
			if self.hp == 0:
				self.need_to_delete = True
				return
			
			to_x = (self.x + self.dx) % test_1623__width
			to_y = (self.y + self.dy) % test_1623__height
			
			to_index = to_y * test_1623__width + to_x
			
			wall_hp = test_1623_tanks__walls[to_index]
			self.need_to_delete = wall_hp > 0
			if self.need_to_delete:
				test_1623_tanks__walls[to_index] -= 1
				if wall_hp == 1:
					self.tank.add_exp(test_1623_tanks__price_walls[to_index], True)
					test_1623_tanks__update_walls()
				return
			
			for tank in test_1623_tanks__players:
				if tank.team == self.team or time.time() - tank.start_time < 3:
					continue
				
				dx, dy = abs(to_x - tank.x), abs(to_y - tank.y)
				if dx <= 1 and dy <= 1:
					self.need_to_delete = True
					self.tank.exp += 1
					tank.last_action_time = test_1623_tanks__time
					
					if tank.hp > 0:
						tank.hp -= 1
					else:
						self.tank.add_exp(max(tank.level, 3))
						tank.need_to_delete = True
					return
			
			for bullet in test_1623_tanks__bullets:
				if bullet.team == self.team:
					continue
				
				bx, by, bdx, bdy = bullet.x, bullet.y, bullet.dx, bullet.dy
				btx, bty = bx + bdx, by + bdy
				
				sx, sy, sdx, sdy = self.x, self.y, self.dx, self.dy
				
				# d - есть ли столкновение
				d = bx == sx and by == sy # В одной клетке?
				if not d:
					if bdx == -sdx and bdy == -sdy: # Если движутся в противоположные стороны
						d = btx == sx and bty == sy #   Одна из них в след. ходу будет там, где сейчас другая?
					elif bdx == sdx and bdy == sdy: # Если движутся в одну сторону
						d = False				   #   Никогда не столкнутся
					else:						   # Если движутся перпендикулярно друг к другу
						d = False				   #   Если и столкнутся, то на след. ходу они будут в одной клетке и это определится в строке
													#	 d = bx == sx and by == sy
				# Если столкнулись
				if d:
					self.need_to_delete = True
					self.tank.exp += 1
					
					bullet.need_to_delete = True
					bullet.tank.exp += 1
					return
			
			self.to_x, self.to_y = to_x, to_y
