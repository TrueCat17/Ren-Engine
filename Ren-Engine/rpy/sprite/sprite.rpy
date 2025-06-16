init -9000 python:
	
	next_sprite_tag = 0
	def get_next_sprite_tag():
		global next_sprite_tag
		next_sprite_tag += 1
		return 'tmp_tag_' + str(next_sprite_tag)
	
	
	class Sprite(SimpleObject):
		def __init__(self, tag, call_str, decl_at, at, show_at, old_sprite = None):
			SimpleObject.__init__(self)
			
			self.tag = tag or get_next_sprite_tag()
			self.call_str = call_str
			self.hiding = False
			
			self.xpos    = self.ypos    = 0
			self.xanchor = self.yanchor = 0
			self.xsize   = self.ysize   = None
			self.xzoom   = self.yzoom   = 1.0
			self.xcrop, self.ycrop, self.xsizecrop, self.ysizecrop = 0.0, 0.0, 1.0, 1.0
			self.alpha = 1.0
			self.rotate = 0
			
			self.contains = []
			self.image = None
			self.res_image = None
			
			self.old_sprite = old_sprite
			self.parent = None
			
			self.decl_at = SpriteAnimation(decl_at, self)
			self.at      = SpriteAnimation(at,      self)
			self.show_at = SpriteAnimation(show_at, self)
			
			self.effect = None
		
		def __str__(self):
			return str(self.call_str)
		
		def set_effect(self, effect):
			if not effect:
				return
			self.effect = effect.copy(self, None) if self.hiding else effect.copy(self.old_sprite, self)
			
			if not has_screen('sprites'):
				show_screen('sprites')
		
		def remove_effect(self):
			if self.effect:
				self.effect.remove()
				self.effect = None
				self.old_sprite = None
		
		def update(self):
			self.decl_at.update()
			self.at.update()
			self.show_at.update()
			
			for spr in self.contains:
				spr.update()
			
			if self.effect:
				self.effect.update()
			
			self.calculate_props()
		
		def calculate_props(self):
			parent = self.parent
			if parent is not None:
				p_xzoom,   p_yzoom   = parent.real_xzoom,   parent.real_yzoom
				p_xsize,   p_ysize   = parent.real_xsize,   parent.real_ysize
				p_xpos,    p_ypos    = parent.real_xpos,    parent.real_ypos
				p_xanchor, p_yanchor = parent.real_xanchor, parent.real_yanchor
				p_rotate             = parent.real_rotate
				p_alpha              = parent.real_alpha
			else:
				p_xzoom,   p_yzoom   = 1, 1
				p_xsize,   p_ysize   = config.width or get_stage_width(), config.height or get_stage_height()
				p_xpos,    p_ypos    = 0, 0
				p_xanchor, p_yanchor = 0, 0
				p_rotate             = 0
				p_alpha              = 1.0
			
			
			self.real_rotate = self.rotate + p_rotate
			self.real_alpha  = self.alpha * p_alpha
			self.real_xzoom  = absolute(self.xzoom * p_xzoom)
			self.real_yzoom  = absolute(self.yzoom * p_yzoom)
			
			if self.xsize is None:
				xsize = get_image_width(self.image) if self.image else 0
			else:
				xsize = get_absolute(self.xsize, config.width or get_stage_width())
			
			if self.ysize is None:
				ysize = get_image_height(self.image) if self.image else 0
			else:
				ysize = get_absolute(self.ysize, config.height or get_stage_height())
			
			self.real_xsize = xsize * self.real_xzoom
			self.real_ysize = ysize * self.real_yzoom
			
			self.real_xanchor = get_absolute(self.xanchor, xsize) * self.real_xzoom
			self.real_yanchor = get_absolute(self.yanchor, ysize) * self.real_yzoom
			
			x = get_absolute(self.xpos, p_xsize / p_xzoom) * p_xzoom - p_xanchor
			y = get_absolute(self.ypos, p_ysize / p_yzoom) * p_yzoom - p_yanchor
			sina = _sin(int(p_rotate))
			cosa = _cos(int(p_rotate))
			xrot = x * cosa - y * sina
			yrot = x * sina + y * cosa
			
			self.real_xpos = p_xpos + p_xanchor + xrot - self.real_xanchor
			self.real_ypos = p_ypos + p_yanchor + yrot - self.real_yanchor
			
			for spr in self.contains:
				spr.calculate_props()
		
		def get_all_sprites(self):
			res = [self]
			for spr in self.contains:
				res.extend(spr.get_all_sprites())
			return res
