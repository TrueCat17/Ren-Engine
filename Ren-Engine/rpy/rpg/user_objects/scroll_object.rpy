init -1000 python:
	
	class ScrollObject(Object):
		def __init__(self, xpos, ypos, xsize, ysize, **kwargs):
			Object.__init__(self)
			
			if 'image' in kwargs:
				self.image = kwargs['image']
			else:
				self.image = im.rect('#8888')
				out_msg('ScrollObject.__init__', 'param <image> not defined')
			
			self.type = kwargs.get('name', 'scroll')
			self.zorder = kwargs.get('zorder', 1e7)
			
			self.xpos, self.ypos, self.xsize, self.ysize = xpos, ypos, xsize, ysize
			self.alpha = kwargs.get('alpha', 1.0)
			
			self.cx, self.cy = 0, 0
			self.dx = kwargs.get('dx', 0.01)
			self.dy = kwargs.get('dy', 0.01)
		
		def __str__(self):
			return '<ScrollObject ' + self.type + '>'
		
		def set_direction(self, dx, dy):
			self.dx, self.dy = dx, dy
		
		def update(self):
			dtime = get_last_tick()
			
			self.cx += self.dx * dtime
			self.cy += self.dy * dtime
		
		def get_draw_data(self):
			xsize = get_absolute(self.xsize, self.location.xsize)
			ysize = get_absolute(self.ysize, self.location.ysize)
			
			res = []
			
			ypos = int(((self.cy % 1) - 1) * ysize) + get_absolute(self.ypos, self.location.ysize)
			for y in (0, 1):
				
				xpos = int(((self.cx % 1) - 1) * xsize) + get_absolute(self.xpos, self.location.ysize)
				for x in (0, 1):
					res.append({
						'image':   self.image,
						'size':   (xsize, ysize),
						'pos':    (xpos, ypos),
						'anchor': (0, 0),
						'crop':   (0, 0, 1.0, 1.0),
						'alpha':   self.alpha,
						'zorder':  self.zorder,
					})
					
					xpos += xsize
				ypos += ysize
			return res
		
