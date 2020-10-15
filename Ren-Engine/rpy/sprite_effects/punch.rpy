init -9000 python:
	
	class Punch(Object):
		def __init__(self, prop, dist, time_one, time_all):
			Object.__init__(self)
			
			self.prop, self.dist, self.time_one, self.time_all = prop, dist, time_one, time_all
			self.start_time = time.time()
		
		def copy(self, spr = None):
			screen.effect = Punch(self.prop, self.dist, self.time_one, self.time_all)
			
			if spr is screen:
				return screen.effect
			return None 
		
		
		def update(self):
			now = time.time()
			dtime = now - self.start_time
			
			if dtime > self.time_all:
				screen.new_data[self.prop] = 0
				screen.remove_effect()
			else:
				t = (dtime % self.time_one) / self.time_one # 0.0 -> 1.0
				
				t = 1 if t > 0.5 else -1
				m = 1 if int(dtime / self.time_one) % 2 else -1
				
				screen.new_data[self.prop] = int(round(t * m * self.dist))
		
		def remove(self):
			screen.new_data[self.prop] = 0
		
		def for_not_hiding(self):
			pass
	
	
	hpunch = Punch('xpos', 10, 0.1, 0.5)
	vpunch = Punch('ypos',  7, 0.1, 0.5)
	
