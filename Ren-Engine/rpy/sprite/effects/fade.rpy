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
			
			self.removing_sprites = []
		
		def copy(self, old_sprite, new_sprite):
			overlay = sprites.overlay
			if type(overlay.effect) is not Fade:
				overlay.effect = Fade(self.out_time, self.hold_time, self.in_time, self.color, None, overlay)
				overlay.image = im.rect(self.color)
				overlay.alpha = 0
			if old_sprite is overlay:
				return overlay.effect
			
			res = Fade(self.out_time, self.hold_time, self.in_time, self.color, old_sprite, new_sprite)
			return res
		
		def update(self):
			overlay = sprites.overlay
			sprite = self.new_sprite or self.old_sprite
			
			if sprite is not overlay:
				effect = overlay.effect
				if effect:
					if effect.after_middle:
						sprite.remove_effect()
				else:
					sprite.remove_effect()
				return
			
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			if dtime < self.out_time + self.hold_time:
				overlay.alpha = in_bounds(dtime / self.out_time, 0, 1)
			else:
				if not self.after_middle:
					self.after_middle = True
					sprites.remove_effect_sprites(self)
				
				overlay.alpha = 1 - in_bounds((dtime - self.out_time - self.hold_time) / self.in_time, 0, 1)
				if overlay.alpha == 0:
					overlay.remove_effect()
		
		def remove(self):
			overlay = sprites.overlay
			if self.new_sprite is overlay:
				overlay.effect = None
				overlay.image = ''
				overlay.alpha = 1
			
			sprites.remove_effect_sprites(self)
			if self.new_sprite:
				self.new_sprite.alpha = 1
	
	
	fade = Fade(0.5)
	fade2 = Fade(1)
	fade3 = Fade(1.5)
	
	flash = Fade(1, color = '#FFF')
	flash2 = Fade(2, 2, 2, color = '#FFF')
	flash_red = Fade(1, color = '#E11')
