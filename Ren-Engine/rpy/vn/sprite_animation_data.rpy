init -9000 python:
	
	class SpriteAnimationData(Object):
		def __init__(self, sprite, decl_at, at, show_at):
			Object.__init__(self)
			
			self.state_num = 0
			self.except_state_props = set()
			
			self.xpos = self.ypos = 0
			self.xanchor = self.yanchor = 0
			self.xsize = self.ysize = None
			self.xzoom = self.yzoom = 1.0
			self.xcrop, self.ycrop, self.xsizecrop, self.ysizecrop = 0, 0, 1.0, 1.0
			self.alpha = 1.0
			self.rotate = 0
			
			self.contains = []
			self.image = None
			
			self.sprite  = sprite
			
			self.decl_at = SpriteAnimation(decl_at, self)
			self.at      = SpriteAnimation(at,      self)
			self.show_at = SpriteAnimation(show_at, self)
			
			self.update()
			self.calculate_props()
		
		def update(self):
			self.decl_at.update()
			self.at.update()
			self.show_at.update()
			
			for spr in self.contains:
				spr.update()
		
		def calculate_props(self, parent = None):
			if parent is not None:
				p_xzoom,   p_yzoom   = parent.real_xzoom,   parent.real_yzoom
				p_xsize,   p_ysize   = parent.real_xsize,   parent.real_ysize
				p_xpos,    p_ypos    = parent.real_xpos,    parent.real_ypos
				p_xanchor, p_yanchor = parent.real_xanchor, parent.real_yanchor
				p_rotate             = parent.real_rotate
				p_alpha              = parent.real_alpha
			else:
				p_xzoom,   p_yzoom   = 1, 1
				p_xsize,   p_ysize   = get_stage_width(),   get_stage_height()
				p_xpos,    p_ypos    = 0, 0
				p_xanchor, p_yanchor = 0, 0
				p_rotate             = 0
				p_alpha              = 1.0
			
			
			self.real_rotate = self.rotate + p_rotate
			self.real_alpha  = self.alpha * p_alpha
			self.real_xzoom  = absolute(self.xzoom * p_xzoom)
			self.real_yzoom  = absolute(self.yzoom * p_yzoom)
			
			
			main_data = self.sprite.new_data or self.sprite.old_data
			
			if self.xsize is None:
				xsize = (main_data and main_data.xsize) or (get_image_width(self.image) if self.image else 0)
			else:
				xsize = get_absolute(self.xsize, config.width or get_stage_width())
			
			if self.ysize is None:
				ysize = (main_data and main_data.ysize) or (get_image_height(self.image) if self.image else 0)
			else:
				ysize = get_absolute(self.ysize, config.height or get_stage_height())
			
			self.real_xsize = xsize * self.real_xzoom
			self.real_ysize = ysize * self.real_yzoom
			
			
			self.real_xanchor = get_absolute(self.xanchor, xsize) * self.real_xzoom
			self.real_yanchor = get_absolute(self.yanchor, ysize) * self.real_yzoom
			
			x = get_absolute(self.xpos, p_xsize) * p_xzoom - p_xanchor
			y = get_absolute(self.ypos, p_ysize) * p_yzoom - p_yanchor
			sina = _sin(p_rotate)
			cosa = _cos(p_rotate)
			xrot = absolute(x * cosa - y * sina)
			yrot = absolute(x * sina + y * cosa)
			
			self.real_xpos = p_xpos + p_xanchor + xrot - self.real_xanchor
			self.real_ypos = p_ypos + p_yanchor + yrot - self.real_yanchor
			
			
			for spr in self.contains:
				spr.calculate_props(self)
		
		
		def get_all_data(self):
			res = [self]
			for spr in self.contains:
				for spr_data in spr.data_list:
					res.extend(spr_data.get_all_data())
			
			return res
	
	
