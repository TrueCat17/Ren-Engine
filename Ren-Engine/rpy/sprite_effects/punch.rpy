init -9000 python:
	
	class Punch(Object):
		def __init__(self, prop, dist, time_one, time_all):
			Object.__init__(self)
			
			self.prop, self.dist, self.time_one, self.time_all = prop, dist, time_one, time_all
			self.start_time = None
		
		def copy(self, spr):
			sprites.screen.effect = Punch(self.prop, self.dist, self.time_one, self.time_all)
			if spr is sprites.screen:
				return sprites.screen.effect
			
			spr.old_data = None
			spr.data_list = (spr.new_data, )
			return None 
		
		
		def update(self):
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			if dtime >= self.time_all:
				sprites.screen.new_data[self.prop] = 0
				sprites.screen.remove_effect()
			else:
				t = (dtime % self.time_one) / self.time_one # 0.0 -> 1.0
				
				t = 1 if t > 0.5 else -1
				m = 1 if int(dtime / self.time_one) % 2 else -1
				
				sprites.screen.new_data[self.prop] = round(t * m * self.dist)
		
		def remove(self):
			sprites.screen.new_data[self.prop] = 0
		
		def for_not_hiding(self):
			pass
	
	
	hpunch = Punch('xpos', 10, 0.1, 0.5)
	vpunch = Punch('ypos',  7, 0.1, 0.5)
