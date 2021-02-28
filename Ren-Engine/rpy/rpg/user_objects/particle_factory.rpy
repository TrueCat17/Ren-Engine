init -1000 python:
	
	class ParticleFactory(Object):
		def __init__(self, xpos, ypos, xsize, ysize, **kwargs):
			Object.__init__(self)
			
			if kwargs.has_key('image'):
				self.image = kwargs['image']
			else:
				self.image = im.rect('#FFF')
				out_msg('ParticleFactory.__init__', 'param <image> not defined')
			
			self.background = kwargs.get('background', None)
			
			self.type = kwargs.get('name', 'particles')
			self.zorder = kwargs.get('zorder', -10)
			
			self.xpos, self.ypos, self.xsize, self.ysize = xpos, ypos, xsize, ysize
			
			self.min_dx = kwargs.get('min_dx', -0.15)
			self.max_dx = kwargs.get('max_dx', 0.15)
			self.min_dy = kwargs.get('min_dy', 0.15)
			self.max_dy = kwargs.get('max_dy', 0.4)
			self.min_size = kwargs.get('min_size', 0.5)
			self.max_size = kwargs.get('max_size', 1.5)
			
			self.objs = []
			self.set_count(kwargs.get('count', 50))
		
		def __str__(self):
			return '<ParticleFactory ' + self.type + '>'
		
		def set_count(self, count):
			def rand_float(min, max):
				return random.random() * (max - min) + min
			
			old_count = len(self.objs)
			if count <= old_count:
				self.objs = self.objs[0:count]
			else:
				self.objs.extend([None] * (count - old_count))
				
				for i in xrange(count - old_count):
					x, y = rand_float(0, self.xsize), rand_float(0, self.ysize)
					dx, dy = rand_float(self.min_dx, self.max_dx), rand_float(self.min_dy, self.max_dy)
					size = rand_float(self.min_size, self.max_size)
					
					self.objs[old_count + i] = [x, y, dx, dy, size]
		
		def set_direction(self, dx, dy):
			self.dx, self.dy = dx, dy
		
		def update(self):
			dtime = get_last_tick()
			
			w, h = self.xsize, self.ysize
			x, y, dx, dy = 0, 1, 2, 3
			for obj in self.objs:
				obj[x] = (obj[x] + obj[dx] * dtime) % w
				obj[y] = (obj[y] + obj[dy] * dtime) % h
		
		def get_zorder(self):
			return self.zorder
		
		def get_draw_data(self):
			back = 1 if self.background else 0
			res = [None] * (back + len(self.objs))
			
			if back:
				res[0] = {
					'image':   self.background,
					'pos':    (self.xpos, self.ypos),
					'size':   (self.xsize, self.ysize),
					'anchor': (0, 0),
					'crop':   (0, 0, 1.0, 1.0),
					'alpha':   1,
					'zorder':  self.get_zorder(),
				}
			
			for i in xrange(len(self.objs)):
				x, y, dx, dy, size = self.objs[i]
				res[back + i] = {
					'image':   self.image,
					'pos':    (absolute(self.xpos + x), absolute(self.ypos + y)),
					'size':    absolute(size),
					'anchor': (0, 0),
					'crop':   (0, 0, 1.0, 1.0),
					'alpha':   1,
					'zorder':  self.get_zorder(),
				}
			
			return res
		
