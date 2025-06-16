init -9000 python:
	
	class Fade(SimpleObject):
		def __init__(self, out_time, hold_time = 0, in_time = None, color = '#000', old_sprite = None, new_sprite = None):
			SimpleObject.__init__(self)
			
			if in_time is None:
				in_time = out_time
			
			self.start_time = None
			self.out_time   = max(out_time , 0.001)
			self.hold_time  = max(hold_time, 0.001)
			self.in_time    = max(in_time  , 0.001)
			
			self.color = color
			self.after_middle = False
			
			self.old_sprite = old_sprite
			self.new_sprite = new_sprite
			if new_sprite:
				new_sprite.alpha = 0
		
		def copy(self, old_sprite, new_sprite):
			screen = sprites.screen
			if type(screen.effect) is not Fade:
				screen.effect = Fade(self.out_time, self.hold_time, self.in_time, self.color, screen)
				screen.image = im.rect(self.color)
				screen.alpha = 0
			if old_sprite is screen:
				return screen.effect
			
			res = Fade(self.out_time, self.hold_time, self.in_time, self.color, old_sprite, new_sprite)
			return res
		
		def update(self):
			screen = sprites.screen
			
			if self.old_sprite is not screen:
				if self.new_sprite:
					effect = screen.effect
					if effect:
						if effect.after_middle:
							self.new_sprite.alpha = 1
					else:
						self.new_sprite.remove_effect()
				return
			
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			if dtime < self.out_time + self.hold_time:
				screen.alpha = in_bounds(dtime / self.out_time, 0, 1)
			else:
				if not self.after_middle:
					self.after_middle = True
					sprites.remove_hiding()
				
				screen.alpha = 1 - in_bounds((dtime - self.out_time - self.hold_time) / self.in_time, 0, 1)
				if screen.alpha == 0:
					screen.remove_effect()
		
		def remove(self):
			screen = sprites.screen
			if screen.effect:
				screen.effect = None
				screen.image = ''
				screen.alpha = 1
			
			if self.new_sprite:
				self.new_sprite.alpha = 1
	
	
	fade = Fade(0.5)
	fade2 = Fade(1)
	fade3 = Fade(1.5)
	
	flash = Fade(1, color = '#FFF')
	flash2 = Fade(2, 2, 2, color = '#FFF')
	flash_red = Fade(1, color = '#E11')
