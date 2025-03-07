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
			show_screen('sprites')
			self.effect = effect.copy(self) if effect else None
		
		def remove_effect(self):
			if self.effect:
				self.effect.remove()
				self.effect = None
				
				self.old_data = None
			
			self.data_list = (self.new_data,) if self.new_data else ()
		
		def update(self, parent = None):
			for data in self.data_list:
				data.update()
			
			if self.effect:
				self.effect.update()
			
			self.calculate_props(parent)
		
		def calculate_props(self, parent):
			for data in self.data_list:
				data.calculate_props(parent)
		
		def get_all_data(self):
			res = []
			for data in self.data_list:
				res.extend(data.get_all_data())
			return res
		
		def __str__(self):
			return str(self.call_str)


