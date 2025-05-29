init -9000 python:
	
	class ImageDissolve(Object):
		def __init__(self, mask, t = 1.0, ramp = 5, reverse = False, spr = None):
			Object.__init__(self)
			
			self.start_time = None
			self.mask = mask
			self.time = max(t, 0.001)
			self.ramp = max(ramp, 1)
			self.reverse = reverse
			
			self.sprite = spr
		
		def copy(self, spr):
			load_image(self.mask)
			
			res = ImageDissolve(self.mask, self.time, self.ramp, self.reverse, spr)
			return res
		
		def update(self):
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			def upd_spr_data(data, k_time, mask, ramp, reverse, hiding):
				image = data.image
				if not image:
					data.res_image = data.image = None
					return
				
				if hiding:
					reverse = not reverse
					mask = im.matrix_color(mask, im.matrix.invert())
				
				w, h = orig_w, orig_h = get_image_size(image)
				if data.xsize:
					w = min(get_absolute(data.xsize, config.width  or get_stage_width()),  w)
				if data.ysize:
					h = min(get_absolute(data.ysize, config.height or get_stage_height()), h)
				
				if (w, h) != (orig_w, orig_h):
					image = im.renderer_scale(image, w, h)
				mask = im.scale(mask, w, h)
				
				value = in_bounds(int(k_time * 255), 0, 255)
				if reverse:
					value = 255 - value
				if value != 255:
					value = int(value / ramp) * ramp
				data.res_image = im.mask(image, mask, value, 'r', '<=', 'a', 1)
			
			sprite = self.sprite
			is_scene = sprite is sprites.scene
			new_data, old_data = sprite.new_data, sprite.old_data
			
			if old_data and not is_scene:
				for data in old_data.get_all_data():
					upd_spr_data(data, dtime / self.time, self.mask, self.ramp, self.reverse, True)
			
			if new_data:
				for data in new_data.get_all_data():
					upd_spr_data(data, dtime / self.time, self.mask, self.ramp, self.reverse, False)
			
			if dtime >= self.time:
				sprite.remove_effect()
				if is_scene:
					sprites.remove_hiding()
		
		def remove(self):
			self.for_not_hiding()
		
		def for_not_hiding(self):
			new_data = self.sprite.new_data
			if new_data:
				for data in new_data.get_all_data():
					data.res_image = None
