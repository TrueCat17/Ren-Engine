init -9000 python:
	
	class ImageDissolve(SimpleObject):
		def __init__(self, mask, t = 1.0, ramp = 5, reverse = False, old_sprite = None, new_sprite = None):
			SimpleObject.__init__(self)
			
			self.mask = mask if not reverse else im.matrix_color(mask, im.matrix.invert())
			self.time = max(t, 0.001)
			self.ramp = max(ramp, 1)
			self.reverse = reverse
			
			self.start_time = None
			
			self.old_sprite = old_sprite
			self.new_sprite = new_sprite
		
		def copy(self, old_sprite, new_sprite):
			load_image(self.mask)
			
			res = ImageDissolve(self.mask, self.time, self.ramp, self.reverse, old_sprite, new_sprite)
			return res
		
		def update(self):
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			value = in_bounds(int(dtime / self.time * 255), 0, 255)
			if value != 255:
				value = value // self.ramp * self.ramp
			if not self.reverse:
				value = 255 - value
			
			def upd_spr(spr, mask, value, cmp_func):
				image = spr.image
				if not image:
					spr.res_image = spr.image = None
					return
				
				(w, h) = (orig_w, orig_h) = get_image_size(image)
				if spr.xsize:
					w = min(w, get_absolute(spr.xsize, config.width  or get_stage_width()))
				if spr.ysize:
					h = min(h, get_absolute(spr.ysize, config.height or get_stage_height()))
				
				if (w, h) != (orig_w, orig_h):
					image = im.renderer_scale(image, w, h)
				mask = im.scale(mask, w, h)
				
				spr.res_image = im.mask(image, mask, value, 'r', cmp_func, 'a', 1)
			
			old_sprite, new_sprite = self.old_sprite, self.new_sprite
			is_scene = new_sprite is sprites.scene
			
			if old_sprite and not is_scene:
				for spr in old_sprite.get_all_sprites():
					upd_spr(spr, self.mask, value, '>' if self.reverse else '<')
			
			if new_sprite:
				for spr in new_sprite.get_all_sprites():
					upd_spr(spr, self.mask, value, '<=' if self.reverse else '>=')
			
			if dtime >= self.time:
				(new_sprite or old_sprite).remove_effect()
				if is_scene:
					sprites.remove_hiding()
		
		def remove(self):
			sprites.remove_hiding()
			
			if self.new_sprite:
				for spr in self.new_sprite.get_all_sprites():
					spr.res_image = None
