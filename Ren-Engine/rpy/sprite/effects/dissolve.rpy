init -9000 python:
	
	class Dissolve(SimpleObject):
		def __init__(self, t, old_sprite = None, new_sprite = None):
			SimpleObject.__init__(self)
			
			self.start_time = None
			self.time = max(t, 0.001)
			
			self.common_sprite = None
			self.old_sprite = old_sprite
			self.new_sprite = new_sprite
			
			if old_sprite:
				old_sprite.update()
			if new_sprite:
				new_sprite.update()
			
			self.set_common_sprite()
		
		def copy(self, old_sprite, new_sprite):
			res = Dissolve(self.time, old_sprite, new_sprite)
			return res
		
		def set_common_sprite(self):
			new_sprite, old_sprite = self.new_sprite, self.old_sprite
			
			if new_sprite is None or old_sprite is None:
				return
			if new_sprite is sprites.scene:
				return
			
			def rotate_point(x, y, xcenter, ycenter, sina, cosa):
				tx, ty = x - xcenter, y - ycenter
				rx, ry = tx * cosa - ty * sina, tx * sina + ty * cosa
				return round(rx + xcenter), round(ry + ycenter)
			def rotate_rect(xmin, ymin, xmax, ymax, xcenter, ycenter, angle):
				sina, cosa = _sin(angle), _cos(angle)
				points = (
					rotate_point(xmin, ymin, xcenter, ycenter, sina, cosa),
					rotate_point(xmin, ymax, xcenter, ycenter, sina, cosa),
					rotate_point(xmax, ymin, xcenter, ycenter, sina, cosa),
					rotate_point(xmax, ymax, xcenter, ycenter, sina, cosa),
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
			def get_rect(spr):
				if spr in cache:
					return cache[spr]
				
				xanchor, yanchor = spr.real_xanchor, spr.real_yanchor
				x, y             = spr.real_xpos,    spr.real_ypos
				xsize, ysize     = spr.real_xsize,   spr.real_ysize
				
				xmin, ymin = x, y
				xmax, ymax = x + xsize, y + ysize
				
				rotate = int(spr.real_rotate) % 360
				if rotate:
					res = rotate_rect(xmin, ymin, xmax, ymax, x + xanchor, y + yanchor, rotate)
				else:
					res = xmin, ymin, xmax, ymax
				
				cache[spr] = res
				return res
			
			def intersection_rects(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2):
				return xmax1 > xmin2 and xmax2 > xmin1 and ymax1 > ymin2 and ymax2 > ymin1
			
			
			new_sprites = new_sprite.get_all_sprites()
			old_sprites = old_sprite.get_all_sprites()
			
			make_common = False
			for new_spr in new_sprites:
				if not new_spr.image:
					continue
				
				xmin1, ymin1, xmax1, ymax1 = get_rect(new_spr)
				
				for old_spr in old_sprites:
					if not old_spr.image:
						continue
					
					xmin2, ymin2, xmax2, ymax2 = get_rect(old_spr)
					
					if intersection_rects(xmin1, ymin1, xmax1, ymax1, xmin2, ymin2, xmax2, ymax2):
						make_common = True
						break
				if make_common:
					break
			
			if not make_common:
				return
			
			all_sprites = [spr for spr in new_sprites + old_sprites if spr.image]
			all_rects = [get_rect(spr) for spr in all_sprites]
			
			xmin, ymin, xmax, ymax = all_rects[0]
			for rect in all_rects[1:]:
				xmin = min(xmin, rect[0])
				ymin = min(ymin, rect[1])
				xmax = max(xmax, rect[2])
				ymax = max(ymax, rect[3])
			
			width, height = xmax - xmin, ymax - ymin
			if width <= 0 or height <= 0 or (xmin, ymin, xmax, ymax) == (0, 0, get_stage_width(), get_stage_height()):
				return
			
			new_args = [(width, height)]
			old_args = [(width, height)]
			
			for args, new_or_old_sprites in ((new_args, new_sprites), (old_args, old_sprites)):
				for spr in new_or_old_sprites:
					image = spr.image
					if not image or spr.real_alpha <= 0:
						continue
					
					image_xsize, image_ysize = get_image_size(image)
					res_xsize, res_ysize = spr.real_xsize, spr.real_ysize
					
					crop = [spr.xcrop, spr.ycrop, spr.xsizecrop, spr.ysizecrop]
					if crop != [0, 0, 1, 1] and crop != [0, 0, image_xsize, image_ysize]:
						for i in range(4):
							crop[i] = get_absolute(crop[i], image_ysize if i % 2 else image_xsize)
						image = im.crop(image, crop)
						image_xsize, image_ysize = crop[2] - crop[0], crop[3] - crop[1]
					
					if (res_xsize, res_ysize) != (image_xsize, image_ysize):
						image = im.renderer_scale(image, res_xsize, res_ysize)
					
					if spr.real_alpha < 1:
						image = im.alpha(image, spr.real_alpha)
					
					rotate = int(spr.real_rotate) % 360
					if rotate:
						image = im.rotozoom(image, -rotate, 1)
					
					_xmin, _ymin, _xmax, _ymax = get_rect(spr)
					args.append((_xmin - xmin, _ymin - ymin))
					args.append(image)
			
			new_image = im.composite(*new_args)
			old_image = im.composite(*old_args)
			
			common_sprite = Sprite(None, '<common for %s and %s>' % (new_sprite, old_sprite), (), (), ())
			common_sprite.hiding = True
			common_sprite.image = im.mask(new_image, old_image, 1, 'a', '>=', 'a', 1)
			load_image(common_sprite.image)
			
			index = sprites.list.index(old_sprite)
			sprites.list.insert(index, common_sprite)
			
			common_sprite.xanchor, common_sprite.yanchor = new_sprite.xanchor, new_sprite.yanchor
			common_sprite.xpos,    common_sprite.ypos    = xmin + get_absolute(new_sprite.xanchor, width), ymin + get_absolute(new_sprite.yanchor, height)
			common_sprite.xsize,   common_sprite.ysize   = width, height
			common_sprite.calculate_props()
		
		
		def update(self):
			if self.start_time is not None:
				dtime = get_game_time() - self.start_time
			else:
				dtime = 0
				signals.add('enter_frame', SetDictFuncRes(self, 'start_time', get_game_time), times = 1)
			
			new_sprite, old_sprite = self.new_sprite, self.old_sprite
			
			alpha = in_bounds(dtime / self.time, 0, 1)
			anti_alpha = 1 - alpha
			
			if new_sprite:
				new_sprite.alpha = alpha
			if old_sprite:
				old_sprite.alpha = anti_alpha
			
			if alpha == 1:
				(new_sprite or old_sprite).remove_effect()
		
		def remove(self):
			sprites.remove_hiding()
			if self.new_sprite:
				self.new_sprite.alpha = 1.0
	
	
	dspr = Dissolve(0.2)
	dissolve = Dissolve(0.5)
	dissolve1 = Dissolve(1)
	dissolve2 = Dissolve(2)
