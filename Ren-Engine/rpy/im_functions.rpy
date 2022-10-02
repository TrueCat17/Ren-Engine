init -1001 python:
	
	# All im-funcs works (takes and returns) with str
	# Engine use inside image-cache for execution im-func, including in-between images
	
	
	def ConditionSwitch(*args):
		if len(args) % 2:
			out_msg('ConditionSwitch', 'Expected even count of arguments')
			return ''
		
		for i in xrange(0, len(args), 2):
			cond = args[i]
			if type(cond) == bool and cond == True:
				return args[i + 1]
			
			cond_res = eval(cond)
			if cond_res:
				return args[i + 1]
		
		out_msg('ConditionSwitch', 'All conditions are failed')
		return args[1]
	
	def get_back_with_color(image, color = '#000', alpha = 0.05):
		w, h = get_image_size(image)
		return im.composite((w, h),
		                    (0, 0), im.alpha(im.rect(color, w, h), alpha),
		                    (0, 0), image)
	
	
	class ImMatrix(list):
		def __new__(cls, *args):
			size = len(args)
			
			if size == 1:
				args = args[0]
			else:
				if size != 0 and size != 20 and size != 25:
					out_msg('ImMatrix', 'Expected 20 or 25 values, got ' + str(len(args)))
				args = list(args) + [0] * (25 - size)
			
			return list.__new__(cls, args)
		
		def __init__(self, *args):
			size = len(args)
			
			if size == 1:
				args = args[0]
			else:
				if size != 0 and size != 20 and size != 25:
					out_msg('ImMatrix', 'Expected 20 or 25 values, got ' + str(len(args)))
				args = list(args) + [0] * (25 - size)
			
			list.__init__(self, args)
		
		def __str__(self):
			return ' '.join(map(str, self))
		
		def __mul__(self, other):
			s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20, s21, s22, s23, s24 = self
			o0, o1, o2, o3, o4, o5, o6, o7, o8, o9, o10, o11, o12, o13, o14, o15, o16, o17, o18, o19, o20, o21, o22, o23, o24 = other
			return ImMatrix(
				s0 * o0 + s5 * o1 + s10 * o2 + s15 * o3 + s20 * o4,
				s1 * o0 + s6 * o1 + s11 * o2 + s16 * o3 + s21 * o4,
				s2 * o0 + s7 * o1 + s12 * o2 + s17 * o3 + s22 * o4,
				s3 * o0 + s8 * o1 + s13 * o2 + s18 * o3 + s23 * o4,
				s4 * o0 + s9 * o1 + s14 * o2 + s19 * o3 + s24 * o4,
				
				s0 * o5 + s5 * o6 + s10 * o7 + s15 * o8 + s20 * o9,
				s1 * o5 + s6 * o6 + s11 * o7 + s16 * o8 + s21 * o9,
				s2 * o5 + s7 * o6 + s12 * o7 + s17 * o8 + s22 * o9,
				s3 * o5 + s8 * o6 + s13 * o7 + s18 * o8 + s23 * o9,
				s4 * o5 + s9 * o6 + s14 * o7 + s19 * o8 + s24 * o9,
				
				s0 * o10 + s5 * o11 + s10 * o12 + s15 * o13 + s20 * o14,
				s1 * o10 + s6 * o11 + s11 * o12 + s16 * o13 + s21 * o14,
				s2 * o10 + s7 * o11 + s12 * o12 + s17 * o13 + s22 * o14,
				s3 * o10 + s8 * o11 + s13 * o12 + s18 * o13 + s23 * o14,
				s4 * o10 + s9 * o11 + s14 * o12 + s19 * o13 + s24 * o14,
				
				s0 * o15 + s5 * o16 + s10 * o17 + s15 * o18 + s20 * o19,
				s1 * o15 + s6 * o16 + s11 * o17 + s16 * o18 + s21 * o19,
				s2 * o15 + s7 * o16 + s12 * o17 + s17 * o18 + s22 * o19,
				s3 * o15 + s8 * o16 + s13 * o17 + s18 * o18 + s23 * o19,
				s4 * o15 + s9 * o16 + s14 * o17 + s19 * o18 + s24 * o19,
				
				s0 * o20 + s5 * o21 + s10 * o22 + s15 * o23 + s20 * o24,
				s1 * o20 + s6 * o21 + s11 * o22 + s16 * o23 + s21 * o24,
				s2 * o20 + s7 * o21 + s12 * o22 + s17 * o23 + s22 * o24,
				s3 * o20 + s8 * o21 + s13 * o22 + s18 * o23 + s23 * o24,
				s4 * o20 + s9 * o21 + s14 * o22 + s19 * o23 + s24 * o24
			)
			# slow, but simple:
#			res = Matrix()
#			for y in xrange(5):
#				for x in xrange(5):
#					for i in xrange(5):
#						res[y * 5 + x] += self[i * 5 + x] * other[y * 5 + i]
#			return res
		
		
		@staticmethod
		def identity():
			res = ImMatrix(
				1, 0, 0, 0, 0,
				0, 1, 0, 0, 0,
				0, 0, 1, 0, 0,
				0, 0, 0, 1, 0,
				0, 0, 0, 0, 1
			)
			return res
		
		@staticmethod
		def tint(r, g, b, a = 1):
			res = ImMatrix(
				r, 0, 0, 0, 0,
				0, g, 0, 0, 0,
				0, 0, b, 0, 0,
				0, 0, 0, a, 0,
				0, 0, 0, 0, 1
			)
			return res
		
		@staticmethod
		def saturation(level, desat=(0.2126, 0.7152, 0.0722)):
			r, g, b = desat
			def I(a, b):
				return a + (b - a) * level
			
			res = ImMatrix(
				I(r, 1), 	I(g, 0), 	I(b, 0), 	0, 	0,
				I(r, 0), 	I(g, 1), 	I(b, 0), 	0, 	0,
				I(r, 0), 	I(g, 0), 	I(b, 1), 	0, 	0,
				0, 			0, 			0, 			1, 	0,
				0, 			0, 			0, 			0, 	1
			)
			return res
		
		@staticmethod
		def invert():
			res = ImMatrix(
				-1, 0, 0, 0, 1,
				0, -1, 0, 0, 1,
				0, 0, -1, 0, 1,
				0, 0, 0,  1, 0,
				0, 0, 0,  0, 1
			)
			return res
		
		@staticmethod
		def brightness(b):
			res = ImMatrix(
				1, 0, 0, 0, b,
				0, 1, 0, 0, b,
				0, 0, 1, 0, b,
				0, 0, 0, 1, 0,
				0, 0, 0, 0, 1
			)
			return res
		
		@staticmethod
		def contrast(c):
			return ImMatrix.brightness(-0.5) * ImMatrix.tint(c, c, c) * ImMatrix.brightness(0.5)
		
		@staticmethod
		def opacity(o):
			res = ImMatrix(
				1, 0, 0, 0, 0,
				0, 1, 0, 0, 0,
				0, 0, 1, 0, 0,
				0, 0, 0, o, 0,
				0, 0, 0, 0, 1
			)
			return res
		
		@staticmethod
		def colorize(black_color, white_color):
			r0, g0, b0, _a0 = renpy.easy.color(black_color)
			r1, g1, b1, _a1 = renpy.easy.color(white_color)
			
			r0 /= 255.0
			g0 /= 255.0
			b0 /= 255.0
			r1 /= 255.0
			g1 /= 255.0
			b1 /= 255.0
			
			res = ImMatrix(
				(r1-r0), 0, 0, 0, r0,
				0, (g1-g0), 0, 0, g0,
				0, 0, (b1-b0), 0, b0,
				0, 0, 0, 1, 0,
				0, 0, 0, 0, 1
			)
			return res
	
	
	
	im__matrix = ImMatrix
	
	
	def im__scale(image, w, h):
		return 'Scale|(' + image + ')|' + str(int(w)) + '|' + str(int(h))
	
	def im__factor_scale(image, k):
		return 'FactorScale|(' + image + ')|' + str(k)
	
	def im__renderer_scale(image, w, h):
		return 'RendererScale|(' + image + ')|' + str(int(w)) + '|' + str(int(h))
	
	def im__crop(image, rect):
		rect = map(lambda f: str(int(f)), rect)
		rect = ' '.join(rect)
		return 'Crop|(' + image + ')|(' + rect + ')'
	
	
	def im__composite(*args):
		if (len(args) % 2) == 0:
			out_msg('im.composite', 'Expected odd count of arguments')
			return ''
		
		size = str(int(args[0][0])) + ' ' + str(int(args[0][1]))
		res = 'Composite|(' + size + ')'
		
		for i in xrange(1, len(args) - 1, 2):
			pos = str(int(args[i][0])) + ' ' + str(int(args[i][1]))
			img = str(args[i + 1])
			
			res += '|(' + pos + ')|(' + img + ')'
		return res
	
	
	def im__flip(image, horizontal = False, vertical = False):
		return 'Flip|(' + image + ')|' + str(bool(horizontal)) + '|' + str(bool(vertical))
	
	
	def im__matrix_color(image, matrix):
		return 'MatrixColor|(' + image + ')|(' + str(matrix) + ')'
	
	def im__grayscale(image, desat=(0.2126, 0.7152, 0.0722)):
		return im.matrix_color(image, im.matrix.saturation(0.0, desat))
	
	def im__sepia(image, tint=(1.0, 0.94, 0.76), desat=(0.2126, 0.7152, 0.0722)):
		return im.matrix_color(image, im.matrix.saturation(0.0, desat) * im.matrix.tint(tint[0], tint[1], tint[2]))
	
	
	def im__recolor(image, r, g, b, a = 255):
		colors = map(str, (r + 1, g + 1, b + 1, a + 1))
		colors = ' '.join(colors)
		return 'ReColor|(' + image + ')|(' + colors + ')'
	
	def im__color(image, color):
		r, g, b, a = renpy.easy.color(color)
		return im.recolor(image, r, g, b, a)
	
	def im__alpha(image, alpha):
		return im.recolor(image, 255, 255, 255, int(alpha * 255))
	
	
	def im__rotozoom(image, angle, zoom = 1.0):
		return 'Rotozoom|(' + image + ')|(' + str(int(angle) % 360) + ')|(' + str(zoom) + ')'
	
	
	def im__mask(image, mask, value, channel = 'r', cmp_func_name = 'le', alpha_channel = 'a', alpha_image = 1):
		return 'Mask|(' + image + ')|(' + mask + ')|(' + channel + ')|(' + str(int(value)) + ')|(' + cmp_func_name + ')|(' + alpha_channel + ')|(' + str(alpha_image) + ')'
	def im__alpha_mask(image, mask):
		return im.mask(image, mask, 0, 'r', 'g', 'r', 2)
	
	
	def im__blur_h(image, dist = 5):
		return 'BlurH|(' + image + ')|' + str(dist)
	def im__blur_v(image, dist = 5):
		return 'BlurV|(' + image + ')|' + str(dist)
	def im__blur(image, dist_h = 5, dist_v = 5):
		return im.blur_h(im.blur_v(image, dist_v), dist_h)
	
	def im__motion_blur(image, cx = 0.5, cy = 0.5, dist = 5):
		return 'MotionBlur|(' + image + ')|' + str(cx) + '|' + str(cy) + '|' + str(int(dist))
	
	
	def im__rect(color, width = 1, height = 1):
		res = 'images/bg/black.jpg'
		r, g, b, a = renpy.easy.color(color)
		if (r, g, b, a) != (0, 0, 0, 255):
			m = im.matrix.invert() * im.matrix.tint(r / 255.0, g / 255.0, b / 255.0, a / 255.0)
			res = im.matrix_color(res, m)
		if get_image_size(res) != (width, height):
			res = im.scale(res, width, height)
		return res
	def im__circle(color, width = 64, height = None):
		height = height or width
		res = 'images/bg/black_circle.png'
		r, g, b, a = renpy.easy.color(color)
		if (r, g, b, a) != (0, 0, 0, 255):
			m = im.matrix.invert() * im.matrix.tint(r / 255.0, g / 255.0, b / 255.0, a / 255.0)
			res = im.matrix_color(res, m)
		if get_image_size(res) != (width, height):
			res = im.scale(res, width, height)
		return res
	
	def im__bar(progress_end, progress_start = 0, vertical = False, ground = None, hover = None):
		bar = ('v' if vertical else '') + 'bar'
		if ground is None:
			ground = gui[bar + '_ground']
		if hover is None:
			hover  = gui[bar + '_hover']
		tw, th = get_image_size(ground)
		
		if vertical:
			x, y = 0, progress_start * th
			w, h = tw, (progress_end - progress_start) * th
		else:
			x, y = progress_start * tw, 0
			w, h = (progress_end - progress_start) * tw, th
		
		x, y = in_bounds(int(x), 0, tw), in_bounds(int(y), 0, th)
		w, h = in_bounds(int(w), 0, tw), in_bounds(int(h), 0, th)
		
		if w <= 0 or h <= 0:
			return ground
		if (x, y, w, h) == (0, 0, tw, th):
			return hover
		
		return im.composite((tw, th),
			                (0, 0), ground,
			                (x, y), im.crop(hover, (x, y, w, h)))
	
	def im__save(image, path, width = None, height = None):
		save_image(image, path, str(width and int(width)), str(height and int(height)))
	
	
	
	build_object('im')
	
	im.Scale = im.scale
	im.FactorScale = im.factor_scale
	im.RendererScale = im.renderer_scale
	im.Crop = im.crop
	
	im.Composite = im.composite
	
	im.Flip = im.flip
	
	im.MatrixColor = im.matrix_color
	im.Grayscale = im.grayscale
	im.Sepia = im.sepia
	
	im.ReColor = im.Recolor = im.recolor
	im.Color = im.color
	im.Alpha = im.alpha
	
	im.Rotozoom = im.RotoZoom = im.rotozoom
	
	im.Mask = im.mask
	im.AlphaMask = im.alpha_mask
	
	im.BlurH = im.blur_h
	im.BlurV = im.blur_v
	im.Blur = im.blur
	im.MotionBlur = im.motion_blur
	
	im.Rect = im.rect
	im.Circle = im.circle
	im.Bar = im.bar
	
	im.Save = im.save

