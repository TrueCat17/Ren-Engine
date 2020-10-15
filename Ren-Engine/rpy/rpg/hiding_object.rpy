init python:
	
	def create_hiding_object(origin):
		location = origin.location
		if location:
			obj = HidingObject(origin)
			location.objects.append(obj)
	
	class HidingObject(Object):
		def __init__(self, origin):
			Object.__init__(self)
			self.show_time = time.time()
			
			self.x, self.y = origin.x, origin.y
			self.xoffset, self.yoffset = origin.xoffset, origin.yoffset
			
			self.xanchor, self.yanchor = origin.xanchor, origin.yanchor
			self.xsize, self.ysize = origin.xsize, origin.ysize
			
			self.crop = origin.crop
			self.alpha = 1
			
			self.location = origin.location
			self.image = origin.main()
		
		def get_zorder(self):
			return self.y + self.yoffset
		def get_draw_data(self):
			return get_usual_location_object_data(self)
		
		def main(self):
			return self.image
		def free(self):
			return None
		
		def update(self):
			dtime = time.time() - self.show_time
			self.alpha = 1 - dtime / location_fade_time
			if self.alpha < 0:
				self.location.objects.remove(self)

