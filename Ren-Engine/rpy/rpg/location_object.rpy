init -1001 python:
	
	location_object_ext = 'png'
	
	
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
		if location_objects.has_key(obj_name):
			out_msg('register_location_object', 'Object <' + obj_name + '> already exists')
			return
		
		obj = location_objects[obj_name] = Object()
		obj.name = obj_name
		obj.max_in_inventory_cell = max_in_inventory_cell
		obj.remove_to_location = remove_to_location
		obj.animations = Object()
		obj.on = []
		obj.sit_places = []
		
		register_location_object_animation(obj_name, None, directory, main_image, free_image, 0, 0, 1, 0, 0)
	
	def register_location_object_animation(obj_name, anim_name,
	                                       directory, main_image, free_image,
	                                       xoffset, yoffset,
	                                       count_frames, start_frame, end_frame, time = 1.0):
		if not location_objects.has_key(obj_name):
			out_msg('register_location_object_animation', 'Object <' + str(obj_name) + '> not registered')
			return
		
		if type(xoffset) is not int or type(yoffset) is not int:
			out_msg('register_location_object_animation',
			        'On registration of animation <' + str(anim_name) + '> of object <' + str(obj_name) + '>\n' +
			        'set invalid pos: <' + str(xoffset) + ', ' + str(yoffset) + '>, expected ints')
			return
		
		if count_frames <= 0 or not (0 <= start_frame < count_frames) or not (0 <= end_frame < count_frames):
			out_msg('register_location_object_animation',
			        'On registration of animation <' + str(anim_name) + '> of object <' + str(obj_name) + '>\n' +
			        'set invalid frames:\n' +
			        'count, start, end = ' + str(count_frames) + ', ' + str(start_frame) + ', ' + str(end_frame))
			return
		
		obj = location_objects[obj_name]
		animations = obj.animations
		if animations.has_key(anim_name):
			out_msg('register_location_object_animation', 'Animation <' + str(anim_name) + '> of object <' + str(obj_name) + '> already exists')
			return
		
		animations[anim_name] = Object(
			directory    = directory,
			main_image   = main_image,
			free_image   = free_image,
			
			count_frames = count_frames,
			start_frame  = start_frame,
			end_frame    = end_frame,
			time         = float(time),
		
			xoffset = xoffset,
			yoffset = yoffset,
			xsize = 0,
			ysize = 0,
			loaded = False
		)
	
	def set_sit_place(obj_name, sit_places, over = None):
		if not location_objects.has_key(obj_name):
			out_msg('set_sit_place', 'Object <' + str(obj_name) + '> not registered')
			return
		
		obj = location_objects[obj_name]
		obj.on = [None] * len(sit_places)
		obj.sit_places = sit_places
		if over:
			obj.animations[None].over_image = over
		
		for name, location in rpg_locations.iteritems():
			for obj in location.objects:
				if obj.type != obj_name:
					continue
				
				for character in obj.on:
					if character:
						character.stand_up()
				obj.on = [None] * len(sit_places)
				obj.sit_places = sit_places
				if over:
					obj.animations[None].over_image = over
	
	
	def get_usual_location_object_data(obj):
		x, y = obj.x + obj.xoffset, obj.y + obj.yoffset
		w, h = obj.xsize, obj.ysize
		
		obj_xanchor, obj_yanchor = get_absolute(obj.xanchor, w), get_absolute(obj.yanchor, h)
		
		return {
			'image':   obj.main(),
			'size':   (w, h),
			'pos':    (absolute(x), absolute(y)),
			'anchor': (obj_xanchor, obj_yanchor),
			'crop':    obj.crop,
			'alpha':   obj.alpha,
			'zorder':  obj.get_zorder(),
		}
	
	
	class RpgLocationObject(Object):
		def __init__(self, name, x, y):
			Object.__init__(self, location_objects[name])
			
			self.type = name
			
			self.x, self.y = x, y
			self.xoffset, self.yoffset = 0, 0
			
			self.xanchor, self.yanchor = 0.5, 1.0
			self.xsize, self.ysize = 0, 0
			
			self.alpha = 1
			
			self.remove_animation()
			self.update()
		
		def __str__(self):
			return '<RpgLocationObject ' + str(self.type) + '>'
		
		def get_zorder(self):
			return self.y + self.yoffset
		def get_draw_data(self):
			res = []
			main = get_usual_location_object_data(self)
			x, y = main['pos']
			w, h = main['size']
			ystart = int(y - main['anchor'][1])
			
			characters = [character for character in self.on if character]
			characters.sort(key = lambda character: character.get_zorder())
			
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
					res.append(character.get_draw_data())
				
			over_image = self.over()
			if over_image:
				res.append(dict(main, image=over_image))
			
			return res
		
		def set_frame(self, frame):
			self.crop = (int(frame) * self.xsize, 0, self.xsize, self.ysize)
		
		def set_animation(self, anim_name):
			if not self.animations.has_key(anim_name):
				out_msg('set_animation', 'Animation <' + str(anim_name) + '> not found in object <' + str(self.type) + '>')
				return False
			
			self.anim_name = anim_name
			self.animation = self.animations[anim_name]
			self.animation.first_update = True
			return True
		
		def main(self):
			return get_location_image(self.animation, self.animation.directory, self.animation.main_image, '', location_object_ext, False)
		def over(self):
			if not self.animation.over_image:
				return None
			return get_location_image(self.animation, self.animation.directory, self.animation.over_image, '', location_object_ext, False)
		
		def free(self):
			if self.animation.free_image is None:
				return None
			res = get_location_image(self.animation, self.animation.directory, self.animation.free_image, '', location_object_ext, True, False)
			if self.animation.count_frames != 1:
				res = im.crop(res, self.crop)
			return res
		
		def start_animation(self, anim_name, repeat = 0):
			if not self.set_animation(anim_name):
				return
			
			self.animation_start_time = get_game_time()
			self.repeat = int(repeat)
		
		def remove_animation(self):
			self.start_animation(None)
		
		def update(self):
			animation = self.animation
			
			dtime = get_game_time() - self.animation_start_time
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
					animation.xsize = int(math.ceil(animation.xsize / animation.count_frames))
				
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
				self.user_function(self)
	
	
	def add_location_object(location_name, place, obj_name, **kwargs):
		if not rpg_locations.has_key(location_name):
			out_msg('add_location_object', 'Location <' + location_name + '> not registered')
			return
		location = rpg_locations[location_name]
		
		if place is None:
			out_msg('add_location_object', 'Unexpected place <None>')
			return
		
		if type(place) is str:
			tmp_place = location.get_place(place)
			if not tmp_place:
				out_msg('add_location_object', 'Place <' + place + '> not found in location <' + location_name + '>')
				return
			place = tmp_place
			pw, ph = place.xsize, place.ysize
			px, py = place.x, place.y
		else:
			px, py = place['x'], place['y'] - 1
			if isinstance(place, (dict, RpgPlace)):
				pw = place['xsize'] if place.has_key('xsize') else 0
				ph = place['ysize'] if place.has_key('ysize') else 0
			else:
				pw = ph = 0
		
		if location_objects.has_key(obj_name):
			instance = RpgLocationObject(obj_name, px + pw / 2, py + ph / 2)
		elif str(type(obj_name)) == "<type 'classobj'>":
			instance = obj_name(px, py, pw, ph, **kwargs)
		else:
			out_msg('', 'Object <' + obj_name + '> not registered')
			return
		
		instance.location = location
		location.objects.append(instance)
		
		free = instance.free
		if callable(free) and free():
			location.path_need_update = True
		return instance
	
	
	def get_location_objects(location_name, place, obj_type, count = -1):
		if not rpg_locations.has_key(location_name):
			out_msg('get_location_objects', 'Location <' + location_name + '> not registered')
			return
		location = rpg_locations[location_name]
		
		if type(place) is str:
			tmp_place = location.get_place(place)
			if not tmp_place:
				out_msg('get_location_objects', 'Place <' + place + '> not found in location <' + location_name + '>')
				return
			place = tmp_place
			px, py = place.x + place.xsize / 2, place.y + place.ysize / 2
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
			
			dx, dy = obj.x - px, obj.y - py
			dist = dx * dx + dy * dy
			res.append((dist, obj))
		
		res.sort(key = lambda (dist, obj): dist)
		if count >= 0:
			res = res[:count]
		return [obj for dist, obj in res]
	
	def get_near_location_object_for_inventory(character = None):
		character = character or me
		x, y = character['x'], character['y']
		
		min_dist = character_radius * 3
		res = None
		
		for i in character.location.objects:
			if not isinstance(i, RpgLocationObject):
				continue
			
			obj = location_objects[i.type]
			if obj['max_in_inventory_cell'] <= 0:
				continue
			
			dist = get_dist(i.x, i.y, x, y)
			if dist < min_dist:
				min_dist = dist
				res = i
		return res
	
	def get_near_sit_objects(character = None, max_dist = None):
		character = character or me
		max_dist = max_dist or character_radius * 4
		
		sides_dpos = {
			to_left:    (-1, 0),
			to_right:   (+1, 0),
			to_back:    (0, -1),
			to_forward: (0, +1),
		}
		
		res = []
		for obj in character.location.objects:
			if not obj.sit_places:
				continue
			
			min_dist = max_dist
			near_point = None
			
			for i in xrange(len(obj.sit_places)):
				if obj.on[i]:
					continue
				px, py, to_side = obj.sit_places[i]
				dx, dy = sides_dpos[to_side]
				px = obj.x + px + dx * character_radius
				py = obj.y - obj.ysize + py + dy * character_radius
				
				dist = get_dist(px, py, character.x, character.y)
				if dist < min_dist:
					min_dist = dist
					near_point = px, py
			
			if near_point:
				res.append((min_dist, obj, near_point))
		
		res.sort()
		return [(obj, near_point) for min_dist, obj, near_point in res]
	
	
	def remove_location_object(location_name, place, obj_name, count = 1):
		if not rpg_locations.has_key(location_name):
			out_msg('remove_location_object', 'Location <' + location_name + '> not registered')
			return
		location = rpg_locations[location_name]
		
		if place is None:
			px = py = 0
		else:
			if type(place) is str:
				tmp_place = location.get_place(place)
				if tmp_place is None:
					out_msg('remove_location_object', 'Place <' + place + '> in location <' + location_name + '> not found')
					return
				place = tmp_place
			px, py = place['x'], place['y']
		
		to_remove = []
		for i in location.objects:
			if i.type == obj_name:
				to_remove.append(i)
		
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
	
