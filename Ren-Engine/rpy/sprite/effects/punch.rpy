init -9000 python:
	
	class Punch(SimpleObject):
		def __init__(self, prop, dist, time_one, time_all):
			SimpleObject.__init__(self)
			
			self.prop = prop
			self.dist = dist
			self.time_one = time_one
			self.time_all = time_all
			
			self.start_time = None
		
		def copy(self, old_sprite, new_sprite):
			sprites.remove_hiding()
			sprites.screen.effect = Punch(self.prop, self.dist, self.time_one, self.time_all)
			if new_sprite is sprites.screen:
				return sprites.screen.effect
			return None 
		
		
		def update(self):
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			if dtime >= self.time_all:
				sprites.screen.remove_effect()
			else:
				t = (dtime % self.time_one) / self.time_one # 0.0 -> 1.0
				
				t = 1 if t > 0.5 else -1
				m = 1 if int(dtime / self.time_one) % 2 else -1
				
				sprites.screen[self.prop] = round(t * m * self.dist)
		
		def remove(self):
			sprites.screen[self.prop] = 0
	
	
	hpunch = Punch('xpos', 15, 0.07, 0.275)
	vpunch = Punch('ypos', 10, 0.07, 0.275)
