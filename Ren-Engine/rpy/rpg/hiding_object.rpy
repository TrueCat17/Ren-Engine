init python:
	
	def create_hiding_object(origin):
		location = origin.location
		if not location:
			return None
		
		obj = RpgHidingObject(origin)
		location.objects.append(obj)
		return obj
	
	class RpgHidingObject(SimpleObject):
		def __init__(self, origin):
			SimpleObject.__init__(self)
			self.show_time = get_game_time()
			
			self.location = origin.location
			self.x, self.y = origin.x, origin.y
			self.alpha = 1
			
			self.data = origin.get_draw_data()
			if type(self.data) not in (list, tuple):
				self.data = [self.data]
		
		def update(self):
			dtime = get_game_time() - self.show_time
			self.alpha = 1 - dtime / location_fade_time
			if self.alpha < 0:
				self.location.objects.remove(self)
		
		def get_draw_data(self):
			res = []
			for data in self.data:
				data = SimpleObject(data)
				data.alpha = self.alpha
				res.append(data)
			return res
		
		def free(self):
			return None
