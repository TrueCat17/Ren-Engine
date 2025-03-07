init -9000 python:
	
	class Fade(Object):
		def __init__(self, out_time, hold_time = 0, in_time = None, color = '#000', spr = None):
			Object.__init__(self)
			
			if in_time is None:
				in_time = out_time
			
			self.start_time = None
			self.out_time   = max(out_time , 0.001)
			self.hold_time  = max(hold_time, 0.001)
			self.in_time    = max(in_time  , 0.001)
			
			self.color = color
			self.after_middle = False
			
			self.sprite = spr
		
		def copy(self, spr):
			sprites.screen.effect = Fade(self.out_time, self.hold_time, self.in_time, self.color, sprites.screen)
			sprites.screen.new_data.alpha = 0
			sprites.screen.new_data.image = im.rect(self.color)
			if spr is sprites.screen:
				return sprites.screen.effect
			
			spr.data_list = (spr.old_data,) if spr.old_data else ()
			res = Fade(self.out_time, self.hold_time, self.in_time, self.color, spr)
			return res
		
		def update(self):
			if self.sprite is not sprites.screen:
				if sprites.screen.effect is None:
					self.sprite.remove_effect()
				return
			
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times=1)
			
			if dtime < self.out_time + self.hold_time:
				sprites.screen.new_data.alpha = in_bounds(dtime / self.out_time, 0.0, 1.0)
			else:
				if not self.after_middle:
					self.after_middle = True
					sprites.remove_hiding()
				
				sprites.screen.new_data.alpha = 1 - in_bounds((dtime - self.out_time - self.hold_time) / self.in_time, 0.0, 1.0)
				if sprites.screen.new_data.alpha == 0:
					sprites.screen.remove_effect()
		
		def remove(self):
			sprites.screen.new_data.alpha = 0.0
			sprites.screen.new_data.image = ''
			sprites.screen.effect = None
		
		def for_not_hiding(self):
			self.sprite.old_data = None
			self.sprite.data_list = (self.sprite.new_data,)
	
	
	fade = Fade(0.5)
	fade2 = Fade(1)
	fade3 = Fade(1.5)
	
	flash = Fade(1, color="#FFF")
	flash2 = Fade(2, 2, 2, color="#FFF")
	flash_red = Fade(1, color="#E11")

