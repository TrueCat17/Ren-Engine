init -9000 python:
	
	class Fade(Object):
		def __init__(self, out_time, hold_time = 0, in_time = None, color = '000', spr = None):
			Object.__init__(self)
			
			if in_time is None:
				in_time = out_time
			
			self.start_time = get_game_time()
			self.out_time   = max(out_time , 0.001)
			self.hold_time  = max(hold_time, 0.001)
			self.in_time    = max(in_time  , 0.001)
			
			self.color = color
			self.inited = False
			self.after_middle = False
			
			self.sprite = spr
		
		def copy(self, spr = None):
			screen.effect = Fade(self.out_time, self.hold_time, self.in_time, self.color, screen)
			if spr is screen:
				return screen.effect
			
			if spr:
				spr.data_list = (spr.old_data,) if spr.old_data else ()
				res = Fade(self.out_time, self.hold_time, self.in_time, self.color, spr)
				return res
			
			return None
		
		def update(self):
			if self.sprite is not screen:
				if screen.effect is None:
					self.sprite.remove_effect()
				return
			
			
			if not self.inited:
				self.inited = True
				
				screen.new_data.alpha = 0
				screen.new_data.image = im.rect(self.color, 1, 1)
			
			dtime = get_game_time() - self.start_time
			
			if dtime < self.out_time + self.hold_time:
				screen.new_data.alpha = in_bounds(dtime / self.out_time, 0.0, 1.0)
			else:
				if not self.after_middle:
					self.after_middle = True
					remove_hiding_sprites()
				
				screen.new_data.alpha = 1 - in_bounds((dtime - self.out_time - self.hold_time) / self.out_time, 0.0, 1.0)
				if screen.new_data.alpha == 0:
					screen.remove_effect()
		
		def remove(self):
			screen.new_data.alpha = 0
			screen.effect = None
		
		def for_not_hiding(self):
			self.sprite.old_data = None
			self.sprite.data_list = (self.sprite.new_data,)
	
	
	fade = Fade(0.5)
	fade2 = Fade(1)
	fade3 = Fade(1.5)
	
	flash = Fade(1, color="#FFF")
	flash2 = Fade(2, 2, 2, color="#FFF")
	flash_red = Fade(1, color="#E11")

