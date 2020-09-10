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
		
		def update(self):
			for at in (self.decl_at, self.at, self.show_at):
				at.update()
			
			for spr in self.contains:
				spr.update()
		
		
		def get_all_data(self, parent = None):
			if parent is not None:
				p_xzoom,   p_yzoom   = parent.real_xzoom,   parent.real_yzoom
				p_xsize,   p_ysize   = parent.real_xsize,   parent.real_ysize
				p_xanchor, p_yanchor = parent.real_xanchor, parent.real_yanchor
				p_rotate             = parent.real_rotate
			else:
				p_xzoom,   p_yzoom   = 1, 1
				p_xsize,   p_ysize   = get_stage_width(),   get_stage_height()
				p_xanchor, p_yanchor = 0, 0
				p_rotate             = 0
			
			
			x = get_absolute(self.xpos, p_xsize) * p_xzoom - p_xanchor
			y = get_absolute(self.ypos, p_ysize) * p_yzoom - p_yanchor
			
			sina = _sin(p_rotate)
			cosa = _cos(p_rotate)
			
			self.real_xpos    = absolute(x * cosa - y * sina)
			self.real_ypos    = absolute(x * sina + y * cosa)
			
			self.real_xanchor = get_absolute(self.xanchor, self.real_xsize) * self.xzoom
			self.real_yanchor = get_absolute(self.yanchor, self.real_ysize) * self.yzoom
			self.real_alpha   = self.alpha
			self.real_rotate  = self.rotate
			self.real_xzoom   = self.xzoom
			self.real_yzoom   = self.yzoom
			
			res = [self]
			
			for spr in self.contains:
				for spr_data in spr.data_list:
					for data in spr_data.get_all_data(self):
						data.real_xpos   += self.real_xpos
						data.real_ypos   += self.real_ypos
						data.real_alpha  *= self.real_alpha
						data.real_rotate += self.real_rotate
						data.real_xzoom  *= self.xzoom
						data.real_yzoom  *= self.yzoom
						res.append(data)
			
			return res
	
	
