init -9000 python:
	
	class Dissolve(Object):
		def __init__(self, t, spr = None):
			Object.__init__(self)
			
			self.start_time = time.time()
			self.time = max(t, 0.001)
			
			self.sprite = spr
			if spr:
				if spr.old_data:
					spr.old_data.except_state_props.add('alpha')
				if spr.new_data:
					spr.new_data.except_state_props.add('alpha')
				
				spr.update()
				self.set_data_list()
		
		def copy(self, spr):
			res = Dissolve(self.time, spr)
			return res
		
		def set_data_list(self):
			def rotate_point(x, y, xcenter, ycenter, angle):
				sina, cosa = _sin(angle), _cos(angle)
				tx, ty = x - xcenter, y - ycenter
				rx, ry = tx * cosa - ty * sina, tx * sina + ty * cosa
				return int(round(rx + xcenter)), int(round(ry + ycenter))
			def rotate_rect(xmin, ymin, xmax, ymax, xcenter, ycenter, angle):
				points = (rotate_point(xmin, ymin, xcenter, ycenter, angle),
				          rotate_point(xmin, ymax, xcenter, ycenter, angle),
				          rotate_point(xmax, ymin, xcenter, ycenter, angle),
				          rotate_point(xmax, ymax, xcenter, ycenter, angle)
				)
				xmin = xmax = points[0][0]
				ymin = ymax = points[0][1]
				for point in points[1:]:
					xmin = min(xmin, point[0])
					ymin = min(ymin, point[1])
					xmax = max(xmax, point[0])
					ymax = max(ymax, point[1])
				return xmin, ymin, xmax, ymax
			
			cache = {}
			def get_rect(data):
				if cache.has_key(data):
					return cache[data]
				
				xanchor, yanchor = data.real_xanchor, data.real_yanchor
				x, y             = data.real_xpos - xanchor, data.real_ypos - yanchor
				xsize, ysize     = data.real_xsize, data.real_ysize
				
				xmin, ymin = x, y
				xmax, ymax = x + xsize, y + ysize
				
				rotate = int(data.real_rotate) % 360
				if rotate:
					res = rotate_rect(xmin, ymin, xmax, ymax, x + xanchor, y + yanchor, rotate)
				else:
					res = xmin, ymin, xmax, ymax
				
				cache[data] = res
				return res
			
			def intersection_rects(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2):
				return xmax1 > xmin2 and xmax2 > xmin1 and ymax1 > ymin2 and ymax2 > ymin1
			
			new_datas = self.sprite.new_data.get_all_data() if self.sprite.new_data else ()
			old_datas = self.sprite.old_data.get_all_data() if self.sprite.old_data else ()
			
			make_common = False
			if self.sprite is not scene:
				for new_data in new_datas:
					if not new_data.image:
						continue
					
					xmin1, ymin1, xmax1, ymax1 = get_rect(new_data)
					
					for old_data in old_datas:
						if not old_data.image:
							continue
						
						xmin2, ymin2, xmax2, ymax2 = get_rect(old_data)
						
						if intersection_rects(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2):
							make_common = True
							break
					if make_common:
						break
			
			if make_common:
				all_datas = [data for data in new_datas + old_datas if data.image is not None]
				all_rects = [get_rect(data) for data in all_datas]
				
				xmin, ymin, xmax, ymax = all_rects[0]
				for rect in all_rects[1:]:
					xmin = min(xmin, rect[0])
					ymin = min(ymin, rect[1])
					xmax = max(xmax, rect[2])
					ymax = max(ymax, rect[3])
				
				width, height = xmax - xmin, ymax - ymin
				if width <= 0 or height <= 0:
					self.sprite.data_list = (self.sprite.old_data, self.sprite.new_data)
				else:
					new_args = [(width, height)]
					old_args = [(width, height)]
					
					for args, datas in ((new_args, new_datas), (old_args, old_datas)):
						for data in datas:
							image = data.image
							if not image or data.real_alpha <= 0:
								continue
							
							image_xsize, image_ysize = get_image_size(data.image)
							res_xsize, res_ysize = data.real_xsize, data.real_ysize
							
							crop = [data.xcrop, data.ycrop, data.xsizecrop, data.ysizecrop]
							if crop != [0, 0, 1, 1] and crop != [0, 0, image_xsize, image_ysize]:
								for i in xrange(4):
									prop = crop[i]
									if (prop > 0 and prop < 1) or (prop == 1.0 and type(prop) is float):
										crop[i] = get_absolute(prop, image_ysize if i % 2 else image_xsize)
								image = im.Crop(image, crop)
								image_xsize, image_ysize = crop[2] - crop[0], crop[3] - crop[1]
							
							if (res_xsize, res_ysize) != (image_xsize, image_ysize):
								image = im.RendererScale(image, res_xsize, res_ysize)
							
							if data.real_alpha < 1:
								image = im.Alpha(image, data.real_alpha)
							
							rotate = int(data.real_rotate) % 360
							if rotate:
								image = im.Rotozoom(image, -rotate, 1)
							
							_xmin, _ymin, _xmax, _ymax = get_rect(data)
							args.append((_xmin - xmin, _ymin - ymin))
							args.append(image)
					
					new_image = im.Composite(*new_args)
					old_image = im.Composite(*old_args)
					
					common_data = SpriteAnimationData(self.sprite, [], [], [])
					common_data.image = im.Mask(new_image, old_image, 1, 'a', 'ge', 'a', 1)
					common_data.xpos, common_data.ypos = xmin, ymin
					common_data.xsize, common_data.ysize = width, height
					
					self.sprite.data_list = (common_data, self.sprite.old_data, self.sprite.new_data)
					
					self.state_num = sum(data.state_num for data in all_datas)
		
		
		def update(self):
			new_data, old_data = self.sprite.new_data, self.sprite.old_data
			
			if len(self.sprite.data_list) == 3:
				new_datas = new_data.get_all_data()
				old_datas = old_data.get_all_data()
				
				state_num = sum(data.state_num for data in new_datas + old_datas if data.image)
				if self.state_num != state_num:
					self.set_data_list()
					self.sprite.update_data_size()
			
			now = time.time()
			dtime = now - self.start_time
			
			alpha = in_bounds(dtime / self.time, 0.0, 1.0)
			anti_alpha = 1 - alpha
			
			if new_data:
				new_data.alpha = alpha
			if old_data:
				old_data.alpha = anti_alpha
			
			if alpha == 1:
				self.sprite.remove_effect()
		
		def remove(self):
			remove_hiding_sprites()
		
		def for_not_hiding(self):
			if self.sprite.new_data:
				self.sprite.new_data.alpha = 1
	
	
	dspr = Dissolve(0.2)
	dissolve = Dissolve(0.5)
	dissolve2 = Dissolve(1)

