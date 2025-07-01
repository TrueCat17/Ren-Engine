init -9000 python:
	
	class Punch(SimpleObject):
		def __init__(self, prop, dist, time_one, time_all):
			SimpleObject.__init__(self)
			
			self.prop = prop
			self.dist = dist
			self.time_one = time_one
			self.time_all = time_all
			
			self.start_time = None
			
			self.removing_sprites = []
		
		def copy(self, old_sprite, new_sprite):
			sprites.overlay.effect = Punch(self.prop, self.dist, self.time_one, self.time_all)
			if new_sprite is sprites.overlay:
				return sprites.overlay.effect
			return None 
		
		
		def update(self):
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
				sprites.remove_effect_sprites(self)
			
			if dtime >= self.time_all:
				sprites.overlay.remove_effect()
			else:
				t = (dtime % self.time_one) / self.time_one # 0.0 -> 1.0
				
				t = 1 if t > 0.5 else -1
				m = 1 if int(dtime / self.time_one) % 2 else -1
				
				sprites[self.prop] = round(t * m * self.dist)
		
		def remove(self):
			sprites[self.prop] = 0
	
	
	hpunch = Punch('xpos', 15, 0.07, 0.275)
	vpunch = Punch('ypos', 10, 0.07, 0.275)
