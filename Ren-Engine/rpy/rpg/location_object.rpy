init -1001 python:
	
	location_object_ext = 'png'
	
	std_sit_dist = 50
	
	location_objects = dict()
	
	
	def location_objects_animations_ended():
		if cur_location:
			for obj in cur_location.objects:
				if not isinstance(obj, RpgLocationObject):
					continue
				
				animation = obj.animation
				if animation.start_frame != animation.end_frame and obj.repeat >= 0:
					if animation.time > 0 and get_game_time() - obj.animation_start_time < animation.time:
						return False
		return True
	can_exec_next_check_funcs.append(location_objects_animations_ended)
	
	def location_objects_animations_to_end():
		if cur_location:
			for obj in cur_location.objects:
				if not isinstance(obj, RpgLocationObject):
					continue
				
				animation = obj.animation
				if animation.start_frame != animation.end_frame and obj.repeat >= 0 and animation.time > 0:
					obj.animation_start_time = get_game_time() - obj.animation.time
					obj.repeat = 0
	can_exec_next_skip_funcs.append(location_objects_animations_to_end)
	
	
	def register_location_object(obj_name, directory, main_image, free_image,
	                             max_in_inventory_cell = 0, remove_to_location = True):
		if obj_name in location_objects:
			out_msg('register_location_object', 'Object <%s> already exists' % (obj_name, ))
			return
		
		obj = location_objects[obj_name] = SimpleObject()
		obj.name = obj_name
		obj.max_in_inventory_cell = max_in_inventory_cell
		obj.remove_to_location    = remove_to_location
		obj.animations = {}
		obj.on = []
		obj.sit_places = []
		obj.is_vertical_sit_place = False
		
		obj.xoffset = 0
		obj.yoffset = 0
		obj.xanchor = 0.5
		obj.yanchor = 1.0
		obj.xsize = 0
		obj.ysize = 0
		obj.alpha = 1
		obj.inventory = []
		
		obj.location = None
		obj.user_function = None
		
		register_location_object_animation(obj_name, None, directory, main_image, free_image, 0, 0, 1, 0, 0)
	
	def register_location_object_animation(obj_name, anim_name,
	                                       directory, main_image, free_image,
	                                       xoffset, yoffset,
	                                       count_frames, start_frame, end_frame, time = 1.0):
		if obj_name not in location_objects:
			out_msg('register_location_object_animation', 'Object <%s> was not registered' % (obj_name, ))
			return
		
		if (type(xoffset), type(yoffset)) != (int, int):
			msg = (
				'On registration of animation <%s> of object <%s>\n'
				'set invalid pos: <%s, %s>, expected ints'
			)
			params = (anim_name, obj_name, xoffset, yoffset)
			out_msg('register_location_object_animation', msg % params)
			return
		
		if (type(count_frames), type(start_frame), type(end_frame)) != (int, int, int):
			msg = (
				'On registration of animation <%s> of object <%s>\n'
				'params count_frame, start_frame and end_frame must be ints\n'
				'(got %s, %s, %s)'
			)
			params = (anim_name, obj_name, count_frames, start_frame, end_frame)
			out_msg('register_location_object_animation', msg % params)
			return
		if count_frames <= 0 or not (0 <= start_frame < count_frames) or not (0 <= end_frame < count_frames):
			msg = (
				'On registration of animation <%s> of object <%s>\n'
				'set invalid frames:\n'
				'count, start, end = %s, %s, %s'
			)
			params = (anim_name, obj_name, count_frames, start_frame, end_frame)
			out_msg('register_location_object_animation', msg % params)
			return
		
		obj = location_objects[obj_name]
		animations = obj.animations
		if anim_name in animations:
			out_msg('register_location_object_animation', 'Animation <%s> of object <%s> already exists' % (anim_name, obj_name))
			return
		
		obj = animations[anim_name] = SimpleObject()
		obj.name = anim_name
			
		obj.directory    = directory
		obj.main_image   = main_image
		obj.free_image   = free_image
		obj.over_image   = None
			
		obj.count_frames = count_frames
		obj.start_frame  = start_frame
		obj.end_frame    = end_frame
		obj.time         = float(time)
		
		obj.xoffset = xoffset
		obj.yoffset = yoffset
		obj.xsize = 0
		obj.ysize = 0
		obj.loaded = False
	
	def set_sit_place(obj_name, sit_places, over = None):
		if obj_name not in location_objects:
			out_msg('set_sit_place', 'Object <%s> was not registered' % (obj_name, ))
			return
		
		for i in range(len(sit_places)):
			if len(sit_places[i]) == 3:
				sit_places[i] += (True, )
		
		obj = location_objects[obj_name]
		obj.on = [None] * len(sit_places)
		obj.sit_places = sit_places
		if over:
			obj.animations[None].over_image = over
		
		is_vertical_sit_place = False
		if sit_places:
			left = right = sit_places[0][0]
			top = bottom = sit_places[0][1]
			for x, y, _, _ in sit_places[1:]:
				left = min(left, x)
				right = max(right, x)
				top = min(top, y)
				bottom = max(bottom, y)
			if bottom - top > right - left:
				is_vertical_sit_place = True
		obj.is_vertical_sit_place = is_vertical_sit_place
		
		for name, location in rpg_locations.items():
			for obj in location.objects:
				if obj.type != obj_name:
					continue
				
				for character in obj.on:
					if character:
						character.stand_up()
				obj.on = [None] * len(sit_places)
				obj.sit_places = sit_places
				obj.is_vertical_sit_place = is_vertical_sit_place
				if over:
					obj.animations[None].over_image = over
	
	
	def get_usual_location_object_data(obj):
		x, y = obj.x + obj.xoffset, obj.y + obj.yoffset
		w, h = obj.xsize, obj.ysize
		
		return {
			'image':   obj.main(),
			'size':   (w, h),
			'pos':    (absolute(x), absolute(y)),
			'anchor': (get_absolute(obj.xanchor, w), get_absolute(obj.yanchor, h)),
			'crop':    obj.crop,
			'alpha':   obj.alpha,
			'zorder':  obj.get_zorder(),
		}
	
	
	class RpgLocationObject(SimpleObject):
		def __init__(self, name, x, y):
			SimpleObject.__init__(self, location_objects[name])
			
			self.type = name
			self.x = x
			self.y = y
			
			self.remove_animation()
			self.update()
		
		def __str__(self):
			return '<RpgLocationObject %s>' % (self.type, )
		
		def get_zorder(self):
			return self.y + self.yoffset
		def get_draw_data(self):
			res = []
			main = get_usual_location_object_data(self)
				
			characters = [character for character in self.on if character]
			characters.sort(key = lambda character: character.get_zorder())
			
			if not self.is_vertical_sit_place:
				res.append(main)
				
				for character in characters:
					main['zorder'] = min(main['zorder'], character.get_zorder())
					
					data = character.get_draw_data()
					if type(data) in (list, tuple):
						res.extend(data)
					else:
						res.append(data)
			
			else:
				x, y = main['pos']
				w, h = main['size']
				ystart = int(y - main['anchor'][1])
				
				top = 0
				for character in characters + [None]:
					bottom = absolute((character or self).get_zorder() - ystart)
					if bottom <= top:
						break
					
					crop = (
						main['crop'][0],
						main['crop'][1] + top,
						main['crop'][2],
						bottom - top
					)
					
					part = dict(main, crop=crop)
					part['size'] = w, bottom - top
					part['anchor'] = main['anchor'][0], 0
					part['pos'] = x, ystart + top
					part['zorder'] = ystart + bottom
					res.append(part)
					
					top = bottom
					
					if character:
						data = character.get_draw_data()
						if type(data) in (list, tuple):
							res.extend(data)
						else:
							res.append(data)
				
			over_image = self.over()
			if over_image:
				over = dict(main, image=over_image)
				w, h = get_image_size(over_image)
				over['size'] = (w, h)
				over['anchor'] = get_absolute(self.xanchor, w), get_absolute(self.yanchor, h)
				del over['crop']
				res.append(over)
			
			return res
		
		def set_frame(self, frame):
			self.crop = (int(frame) * self.xsize, 0, self.xsize, self.ysize)
		
		def update_location_paths(self):
			self.update()
			if self.location:
				self.location.path_need_update = True
		
		def set_animation(self, anim_name):
			if anim_name not in self.animations:
				out_msg('set_animation', 'Animation <%s> not found in object <%s>' % (anim_name, self.type))
				return False
			
			self.anim_name = anim_name
			self.animation = self.animations[anim_name]
			self.animation.first_update = True
			
			self.animation_start_time = get_game_time()
			self.update_location_paths()
			return True
		
		def main(self):
			animation = self.animation
			return get_location_image(animation.directory, animation.main_image, '', location_object_ext, False)
		def over(self):
			animation = self.animation
			if not animation.over_image:
				return None
			return get_location_image(animation.directory, animation.over_image, '', location_object_ext, False)
		
		def free(self):
			animation = self.animation
			free = animation.free_image
			if free is None:
				return None
			res = get_location_image(animation.directory, free, '', location_object_ext, True, False)
			if animation.count_frames != 1:
				res = im.crop(res, self.crop)
			return res
		
		def get_free_rect(self):
			free = self.free()
			if not free:
				return 0, 0, self.xsize, self.ysize
			
			cache = RpgLocationObject.get_free_rect.__dict__
			if free not in cache:
				w, h = get_image_size(free)
				black_color = 0x000000FF # 0xRRGGBBAA
				
				for left in range(w):
					empty = True
					for i in range(h):
						if get_image_pixel(free, left, i) == black_color:
							empty = False
							break
					if not empty:
						break
				
				if empty:
					left, top = 0, 0
					right, bottom = w, h
				else:
					for right in range(w - 1, -1, -1):
						empty = True
						for i in range(h):
							if get_image_pixel(free, right, i) == black_color:
								empty = False
								break
						if not empty:
							break
					
					for top in range(h):
						empty = True
						for i in range(left, right + 1):
							if get_image_pixel(free, i, top) == black_color:
								empty = False
								break
						if not empty:
							break
					for bottom in range(h - 1, -1, -1):
						empty = True
						for i in range(left, right + 1):
							if get_image_pixel(free, i, bottom) == black_color:
								empty = False
								break
						if not empty:
							break
				
				cache[free] = left, top, right, bottom # (x, y, x + w, y + h)
			return cache[free]
		
		def dist_to(self, x, y):
			x -= self.x - get_absolute(self.xanchor, self.xsize)
			y -= self.y - get_absolute(self.yanchor, self.ysize)
			left, top, right, bottom = self.get_free_rect()
			
			if left <= x <= right:
				if y <= top:
					return top - y
				if y >= bottom:
					return y - bottom
			elif top <= y <= bottom:
				if x <= left:
					return left - x
				if x >= right:
					return x - right
			
			dx = min(abs(x - left), abs(x - right))
			dy = min(abs(y - top),  abs(y - bottom))
			return math.sqrt(dx * dx + dy * dy)
		
		def start_animation(self, anim_name, repeat = 0):
			if self.set_animation(anim_name):
				self.repeat = int(repeat)
		
		def remove_animation(self):
			self.start_animation(None)
		
		def update(self):
			animation = self.animation
			dtime = get_game_time() - self.animation_start_time
			
			if has_screen('pause') and not animation.first_update:
				self.animation_start_time += get_last_tick()
				return
			
			time_k = 1
			if animation.time > 0:
				if dtime > animation.time:
					if self.repeat:
						self.animation_start_time = get_game_time()
						time_k = 0
					if self.repeat > 0:
						self.repeat -= 1
				else:
					time_k = dtime / animation.time
			
			if animation.first_update:
				animation.first_update = False
				
				if not animation.loaded:
					animation.loaded = True
					animation.xsize, animation.ysize = get_image_size(self.main())
					if animation.xsize % animation.count_frames:
						msg = 'Animation <%s> of object <%s> has xsize (%s) that is not divisible by the count of frames (%s)'
						params = (animation.name, self.type, animation.xsize, animation.count_frames)
						out_msg('RpgLocationObject.update', msg % params)
					animation.xsize = math.ceil(animation.xsize / animation.count_frames)
				
				self.xsize, self.ysize = animation.xsize, animation.ysize
				self.xoffset, self.yoffset = animation.xoffset, animation.yoffset
			
			start_frame = animation.start_frame
			end_frame = animation.end_frame
			if end_frame < start_frame:
				frame = start_frame - int((start_frame - end_frame + 1) * time_k)
				frame = in_bounds(frame, end_frame, start_frame)
			else:
				frame = start_frame + int((end_frame - start_frame + 1) * time_k)
				frame = in_bounds(frame, start_frame, end_frame)
			
			self.set_frame(frame)
			
			if self.user_function:
				funcs = self.user_function
				if type(funcs) not in (list, tuple):
					funcs = [funcs]
				for func in funcs:
					func(self)
	
	
	def add_location_object(location_name, place, obj_name, **kwargs):
		if location_name not in rpg_locations:
			out_msg('add_location_object', 'Location <%s> was not registered' % (location_name, ))
			return
		location = rpg_locations[location_name]
		
		if place is None:
			out_msg('add_location_object', 'Unexpected place <None>')
			return
		
		if type(place) is str:
			tmp_place = location.get_place(place)
			if not tmp_place:
				out_msg('add_location_object', 'Place <%s> not found in location <%s>' % (place, location_name))
				return
			place = tmp_place
			pw, ph = place.xsize, place.ysize
			px, py = place.x, place.y
		else:
			px, py = place['x'], place['y']
			if isinstance(place, (dict, RpgPlace)):
				pw = place.get('xsize', 0)
				ph = place.get('ysize', 0)
			else:
				pw = ph = 0
		
		if obj_name in location_objects:
			instance = RpgLocationObject(obj_name, px + pw // 2, py + ph // 2)
		elif callable(obj_name):
			instance = obj_name(px, py, pw, ph, **kwargs)
			if not instance.type:
				instance.type = obj_name
		else:
			out_msg('add_location_object', 'Object <%s> was not registered' % (obj_name, ))
			return
		
		instance.location = location
		location.objects.append(instance)
		
		free = instance.free
		if callable(free) and free():
			location.path_need_update = True
		return instance
	
	
	def get_location_objects(location_name, place, obj_type, count = -1):
		if location_name not in rpg_locations:
			out_msg('get_location_objects', 'Location <%s> was not registered' % (location_name, ))
			return
		location = rpg_locations[location_name]
		
		if type(place) is str:
			tmp_place = location.get_place(place)
			if not tmp_place:
				out_msg('get_location_objects', 'Place <%s> not found in location <%s>' % (place, location_name))
				return
			place = tmp_place
			px, py = place.x + place.xsize // 2, place.y + place.ysize // 2
		elif place is not None:
			px, py = place['x'], place['y']
		else:
			px = py = 0
		
		res = []
		for obj in location.objects:
			if not isinstance(obj, RpgLocationObject):
				continue
			if obj_type is not None and obj_type != obj.type:
				continue
			
			dist = obj.dist_to(px, py)
			res.append((dist, obj))
		
		res.sort(key = lambda dist_and_obj: dist_and_obj[0])
		if count >= 0:
			res = res[:count]
		return [obj for dist, obj in res]
	
	def get_near_location_object_for_inventory(character = None):
		character = character or me
		x, y = character['x'], character['y']
		
		min_dist = character.radius * 3
		res = None
		
		for i in character.location.objects:
			if not isinstance(i, RpgLocationObject):
				continue
			
			obj = location_objects[i.type]
			if obj['max_in_inventory_cell'] <= 0:
				continue
			
			dist = i.dist_to(x, y)
			if dist < min_dist:
				min_dist = dist
				res = i
		return res
	
	def get_near_location_object_with_inventory(character = None):
		character = character or me
		x, y = character['x'], character['y']
		
		min_dist = character.radius * 3
		res = None
		
		for i in character.location.objects + list(character.location.places.values()):
			if isinstance(i, RpgLocationObject):
				dist = i.dist_to(x, y)
			elif isinstance(i, RpgPlace):
				ix, iy = get_place_center(i)
				dist = get_dist(ix, iy, x, y)
			else:
				continue
			if not i.inventory:
				continue
			if i.openable is not None:
				openable = i.openable
				if callable(openable):
					openable = openable()
				if not openable:
					continue
			
			if dist < min_dist:
				min_dist = dist
				res = i
		return res
	
	def get_near_sit_objects(character = None, max_dist = None):
		character = character or me
		max_dist = max_dist or std_sit_dist
		
		character_radius = character.radius
		character_x, character_y = character.x, character.y
		
		sides_dpos = {
			to_left:    (-1, 0),
			to_right:   (+1, 0),
			to_back:    (0, +1),
			to_forward: (0, -1),
		}
		
		res = []
		for obj in character.location.objects:
			if isinstance(obj, Character):
				continue
			sit_places = obj.get('sit_places')
			if not sit_places:
				continue
			
			min_dist = max_dist
			near_point = None
			
			for i in range(len(sit_places)):
				if obj.on[i]:
					continue
				px, py, to_side, can_use = sit_places[i]
				if not can_use:
					continue
				
				dx, dy = sides_dpos[to_side]
				px = obj.x + px + dx * character_radius
				py = obj.y - obj.ysize + py + dy * character_radius
				
				dist = get_dist(px, py, character_x, character_y)
				if dist < min_dist:
					min_dist = dist
					near_point = px, py
			
			if near_point:
				res.append((min_dist, obj, near_point))
		
		res.sort(key = lambda t: t[0])
		return [(obj, near_point) for min_dist, obj, near_point in res]
	
	
	def remove_location_object(location_name, place, obj_name, count = 1):
		if location_name not in rpg_locations:
			out_msg('remove_location_object', 'Location <%s> was not registered' % (location_name, ))
			return
		location = rpg_locations[location_name]
		
		if place is None:
			px = py = 0
		else:
			if type(place) is str:
				tmp_place = location.get_place(place)
				if tmp_place is None:
					out_msg('remove_location_object', 'Place <%s> in location <%s> not found' % (place, location_name))
					return
				place = tmp_place
			px, py = place['x'], place['y']
		
		if count == 0:
			return
		
		to_remove = []
		for i in location.objects:
			if i.type == obj_name:
				to_remove.append(i)
		
		if count > 0:
			def dist_sqr(obj):
				dx, dy = (obj.x or 0) - px, (obj.y or 0) - py
				return dx*dx + dy*dy
			
			to_remove.sort(key = dist_sqr)
			to_remove = to_remove[0:count]
		
		for i in to_remove:
			i.location = None
			
			if i in location.objects:
				location.objects.remove(i)
				
				if callable(i.free) and i.free():
					location.path_need_update = True
	
