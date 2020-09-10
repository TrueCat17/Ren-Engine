init -9000 python:
	
	class Sprite(Object):
		def __init__(self, decl_at, at, show_at, old_sprite = None):
			Object.__init__(self)
			
			self.hiding = False
			
			self.new_data = SpriteAnimationData(self, decl_at, at, show_at)
			
			if old_sprite:
				self.old_data = old_sprite.new_data or old_sprite.old_data
				self.data_list = (self.old_data, self.new_data)
			else:
				self.old_data = None
				self.data_list = (self.new_data,)
		
		
		def set_effect(self, effect):
			self.effect = effect.copy(self) if effect else None
		
		def remove_effect(self):
			if self.effect:
				self.effect.remove()
				self.effect = None
				
				self.old_data = None
			
			self.data_list = (self.new_data,) if self.new_data else ()
		
		
		def update_data_size(self):
			main_data = self.new_data or self.old_data
			if main_data is None:
				return
			
			for data in self.data_list:
				xsize = ysize = 0
				
				if data.xsize is not None:
					xsize = get_absolute(data.xsize, get_stage_width())
				else:
					if main_data.xsize is None and data.image:
						xsize = get_image_width(data.image)
					else:
						xsize = main_data.xsize or 0
			
				if data.ysize is not None:
					ysize = get_absolute(data.ysize, get_stage_height())
				else:
					if main_data.ysize is None and data.image:
						ysize = get_image_height(data.image)
					else:
						ysize = main_data.ysize or 0
				
				data.real_xsize, data.real_ysize = xsize, ysize
		
		def update(self):
			for data in self.data_list:
				data.update()
			
			self.update_data_size()
			if self.effect:
				self.effect.update()
		
		def __str__(self):
			return str(self.call_str)


