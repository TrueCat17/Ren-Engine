init -1002 python:
	
	location_ext = 'png'
	
	
	# params for location effect <fade>
	# this does not apply if next_location is cur_location
	
	location_start_time = -100
	location_fade_time = 0.5
	location_time_without_control = 0.33
	
	location_was_show = True
	def location_showed():
		return location_was_show
	can_exec_next_check_funcs.append(location_showed)
	
	def location_show():
		global location_start_time
		location_start_time = get_game_time() - location_fade_time * 2
	can_exec_next_skip_funcs.append(location_show)
	
	
	# last_set_location_time is ALWAYS updated when <set_location> is called (with correct params)
	prev_set_location_time = last_set_location_time = -1
	
	
	cur_location = None
	cur_location_name = ''
	cur_exit = None
	cur_place = None
	cur_place_name = prev_place_name = ''
	
	
	location_zoom = 1.0
	
	
	rpg_locations = {}
	
	def register_location(name, path_to_images, is_room, xsize, ysize):
		if name in rpg_locations:
			out_msg('register_location', 'Location <%s> is already registered' % (name, ))
		else:
			rpg_locations[name] = RpgLocation(name, path_to_images, is_room, xsize, ysize)
	
	def register_place(location_name, place_name, x, y, xsize, ysize, to = None):
		location = rpg_locations.get(location_name)
		if not location:
			out_msg('register_place', 'Location <%s> was not registered' % (location_name, ))
			return
		
		if location.get_place(place_name):
			out_msg('register_place', 'Place <%s> in location <%s> is already exists' % (place_name, location_name))
			return
		
		
		if to is not None:
			if type(to) not in (list, tuple):
				out_msg('register_place', '<to> must be None, list or tuple')
				to = None
			else:
				if len(to) == 3:
					to += (None, )
				if len(to) != 4:
					out_msg('register_place', 'Expects <to> = None, (exit_side, to_location_name, to_place_name) or (exit_side, to_location_name, to_place_name, to_side)')
					to = None
		if not to:
			to = None, None, None, None
		
		place = RpgPlace(place_name, x, y, xsize, ysize, to)
		place.location = location
		location.add_place(place, place_name)
		location.path_need_update = True
	
	
	def set_location(location_name, place, character = None):
		character = character or me
		
		location = rpg_locations.get(location_name)
		if not location:
			out_msg('set_location', 'Location <%s> was not registered' % (location_name, ))
			return
		
		if type(place) is str:
			place_name = place
			place = location.get_place(place)
			if not place:
				out_msg('set_location', 'Place <%s> in location <%s> not found' % (place_name, location_name))
				return
		
		if not has_screen('location'):
			show_screen('location')
		
		global last_set_location_time
		last_set_location_time = get_game_time()
		
		global cur_location, cur_location_name, cur_place, cur_place_name
		prev_location = cur_location
		cur_location = location
		cur_location_name = location_name
		cur_place = place
		cur_place_name = place.get('name', '')
		
		if prev_location != cur_location:
			signals.send('rpg-location')
		
		main = cur_location.main()
		real_width, real_height = get_image_size(main)
		reg_width, reg_height = cur_location.xsize, cur_location.ysize
		if reg_width != real_width or reg_height != real_height:
			msg = (
				'Location sizes on registration (%sx%s) not equal to real sizes (%sx%s)\n'
				'Location: <%s>\n'
				'Main image: <%s>'
			)
			params = (reg_width, reg_height, real_width, real_height, location_name, main)
			
			out_msg('set_location', msg % params)
		
		end_location_ambience(cur_location)
		ignore_prev_rpg_control()
		
		time_changing = times['next_name'] is not None
		
		global draw_location, location_start_time, location_was_show
		if (draw_location is None) or (draw_location is cur_location and not time_changing):
			location_start_time = -100
			location_was_show = True
			draw_location = cur_location
			cam_object = prev_location.cam_object if prev_location else character
			start_location_ambience()
		else:
			location_start_time = get_game_time()
			location_was_show = False
			cam_object = character
			cur_location.last_coords = []
		
		if not (time_changing and place is character):
			show_character(character, place, auto_change_location = False)
			if prev_location is None:
				character.show_time = -100
		
		# after show_character, because cur_loc.cam_object mb changed (if draw_loc is cur_loc)
		cur_location.cam_object = cam_object
		cur_location.cam_object_old = None
		
		if character is me:
			me_x, me_y = me.x, me.y
			for place in cur_location.places.values():
				if place.inside(me_x, me_y):
					break
	
	def hide_location():
		global cur_location, cur_location_name, cur_place, cur_place_name
		cur_location = cur_place = None
		cur_location_name = cur_place_name = ''
		
		global draw_location
		draw_location = None
		end_location_ambience(None)
	
	
	location_banned_exits = set()
	def ban_exit(location_name, place_name = None):
		location = rpg_locations.get(location_name, None)
		if location is None:
			out_msg('ban_exit', 'Location <%s> was not registered' % (location_name, ))
			return
		
		if place_name is not None:
			if place_name in location.places:
				location_banned_exits.add((location_name, place_name))
			else:
				out_msg('ban_exit', 'Place <%s> in location <%s> not found' % (place_name, location_name))
			return
		
		for place_name in location.places:
			location_banned_exits.add((location_name, place_name))
	
	def unban_exit(location_name, place_name = None):
		location = rpg_locations.get(location_name, None)
		if location is None:
			out_msg('unban_exit', 'Location <%s> was not registered' % (location_name, ))
			return
		if place_name is not None and place_name not in location.places:
			out_msg('unban_exit', 'Place <%s> in location <%s> not found' % (place_name, location_name))
			return
		
		tmp_banned_exits = set(location_banned_exits)
		for tmp_loc, tmp_place in tmp_banned_exits:
			if tmp_loc == location_name and (tmp_place == place_name or place_name is None):
				location_banned_exits.remove((tmp_loc, tmp_place))
	
	
	def get_location_image(directory, name, name_suffix, ext, is_free, need = True):
		cache = get_location_image.__dict__
		
		mode = times['current_name']
		key = directory, name, name_suffix, ext, is_free, mode
		if key in cache:
			return cache[key]
		
		file_name = name
		if name_suffix:
			file_name += '_' + name_suffix
		
		path = directory + file_name + '_' + mode + '.' + ext
		if not os.path.exists(path):
			path = directory + file_name + '.' + ext
			if os.path.exists(path):
				if not is_free:
					r, g, b = location_time_rgb
					path = im.recolor(path, r, g, b)
			else:
				if need:
					out_msg('get_location_image', 'File <%s> not found' % (path, ))
				path = None
		
		cache[key] = path
		return path
	
	def set_location_scales(name, min_scale, count_scales):
		if name not in rpg_locations:
			out_msg('set_location_scales', 'Location <%s> was not registered' % (name, ))
			return
		
		if type(min_scale) is not int or type(count_scales) is not int:
			out_msg('set_location_scales', 'Type min_scale or count_scales is not int')
			return
		if min_scale <= 0:
			out_msg('set_location_scales', 'min_scale <= 0')
			return
		if min_scale & (min_scale - 1):
			out_msg('set_location_scales', 'min_scale is not power of 2')
			return
		if count_scales <= 0:
			out_msg('set_location_scales', 'count_scales <= 0')
			return
		
		location = rpg_locations[name]
		location.min_scale = min_scale
		location.count_scales = count_scales
		location.path_need_update = True
	
	
	class RpgLocation(Object):
		last_coords_count = 10
		last_coords_min_diff = 3
		
		def __init__(self, name, directory, is_room, xsize, ysize):
			Object.__init__(self,
				name = name,
				directory = directory + ('' if directory.endswith('/') else '/'),
				
				is_room = is_room,
				xsize = xsize,
				ysize = ysize,
				
				places = {},
				
				ambience_paths = None,
				ambience_volume = 1.0,
				
				path_need_update = True,
				min_scale = 8,
				count_scales = 1 if is_room else 2, # more accuracy for small space
				
				cam_object = None,
				cam_object_old = None,
				
				last_coords = [],
			)
			
			objects = self.objects = [self]
			if self.over():
				objects.append(RpgLocationOver(self))
		
		def __getstate__(self):
			res = Object.__getstate__(self)
			res['path_need_update'] = True
			res['last_coords'] = []
			return res
		
		def __str__(self):
			return '<RpgLocation %s>' % (self.name, )
		
		def get_draw_data(self):
			return {
				'image':   self.main(),
				'size':   (self.xsize, self.ysize),
				'pos':    (0, 0),
				'zorder':  0,
			}
		
		def main(self):
			return get_location_image(self.directory, 'main', '', location_ext, False)
		def over(self):
			return get_location_image(self.directory, 'over', '', location_ext, False, False)
		def free(self):
			return get_location_image(self.directory, 'free', '', location_ext, True, False)
		
		def preload(self):
			for image in self.main(), self.over(), self.free():
				if image:
					load_image(image)
		
		def add_place(self, place, place_name):
			self.places[place_name] = place
		
		def get_place(self, place_name):
			return self.places.get(place_name, None)
		
		
		def update_pos(self):
			pos = get_camera_params(self)
			
			last_coords = self.last_coords
			if self.last_zoom != location_zoom:
				self.last_zoom = location_zoom
				last_coords[:] = []
			
			if last_coords:
				prev = last_coords[-1]
				dx = abs(pos[0] - prev[0])
				dy = abs(pos[1] - prev[1])
				if max(dx, dy) < self.last_coords_min_diff:
					pos = prev
			last_coords.append(pos)
			last_coords[:-self.last_coords_count] = []
			
			x = y = 0
			for pos in last_coords:
				x += pos[0]
				y += pos[1]
			l = len(last_coords)
			
			self.x = round(x / l)
			self.y = round(y / l)
	
	
	class RpgPlace(Object):
		def __init__(self, name, x, y, xsize, ysize, to):
			exit_side, to_location_name, to_place_name, to_side = to
			
			Object.__init__(self,
				name = name,
				x = x,
				y = y,
				xsize = xsize,
				ysize = ysize,
				
				exit_side = exit_side,
				to_location_name = to_location_name,
				to_place_name = to_place_name,
				to_side = to_side,
				
				inventory = [],
			)
		
		def __str__(self):
			return '<RpgPlace %s>' % (self.name, )
		
		def get_rect(self, of_exit = False):
			rect = self.x, self.y, self.xsize, self.ysize
			
			side = self.exit_side
			if not side:
				return rect
			
			x, y, w, h = rect
			w2, h2 = w // 2, h // 2
			
			if not of_exit:
				if   side == 'up':    side = 'down'
				elif side == 'down':  side = 'up'
				elif side == 'left':  side = 'right'
				elif side == 'right': side = 'left'
			
			if side == 'up':    return (x, y, w, h2)
			if side == 'down':  return (x, y + h2, w, h2)
			if side == 'left':  return (x, y, w2, h)
			if side == 'right': return (x + w2, y, w2, h)
		
		def inside(self, x, y):
			return self.x <= x and x < self.x + self.xsize and self.y <= y and y < self.y + self.ysize
		
		def inside_exit(self, px, py):
			if not self.to_location_name: return False
			
			x, y, w, h = self.get_rect(of_exit = True)
			return x <= px and px < x + w and y <= py and py < y + h
	
	
	def get_place_center(place, anchor = None):
		if isinstance(place, RpgPlace):
			x, y, w, h = place.get_rect(of_exit = False)
		else:
			x, y = place['x'], place['y']
			w = place.get('xsize', 0)
			h = place.get('ysize', 0)
		
		if anchor is None:
			if isinstance(place, Character):
				return x, y
			
			anchor = (0.5, 0.5)
		
		xa = get_absolute(place.get('xanchor', 0), w)
		ya = get_absolute(place.get('yanchor', 0), h)
		
		return x - xa + get_absolute(anchor[0], w), y - ya + get_absolute(anchor[1], h)
	
	
	def get_location_exit(character = None):
		ch = character or me
		location = ch.location
		if not location:
			return None
		ch_x, ch_y = ch.x, ch.y
		
		for place in location.places.values():
			if place.inside_exit(ch_x, ch_y):
				return place
		return None
	
	def get_location_place(character = None):
		ch = character or me
		location = ch.location
		if not location:
			return None
		ch_x, ch_y = ch.x, ch.y
		
		res = None
		res_square = 1e9
		for place in location.places.values():
			if place.inside(ch_x, ch_y):
				square = place.xsize * place.ysize
				if square < res_square:
					res = place
					res_square = square
		return res
	
	
	def path_update_locations():
		floor = math.floor
		
		for location in rpg_locations.values():
			if not location.path_need_update: continue
			location.path_need_update = False
			
			free = location.free()
			if not free:
				free = im.rect('#000', location.xsize, location.ysize)
			
			objects = []
			for obj in location.objects:
				if not isinstance(obj, RpgLocationObject): continue
				
				free_obj = obj.free()
				if not free_obj: continue
				
				xsize, ysize = get_image_size(free_obj) # not obj.[x/y]size, mb size(free) != size(main)
				x = floor(obj.x - get_absolute(obj.xanchor, xsize) + obj.xoffset)
				y = floor(obj.y - get_absolute(obj.yanchor, ysize) + obj.yoffset)
				objects.extend((free_obj, x, y))
			
			places = []
			for place in location.places.values():
				x, y = floor(place.x + place.xsize // 2), floor(place.y + place.ysize // 2)
				places.extend((place.name, x, y, place.to_location_name or '', place.to_place_name or ''))
			
			path_update_location(location.name, free, Character.radius, objects, places, location.min_scale, location.count_scales)
	
	
	class RpgLocationOver(Object):
		def __init__(self, location):
			Object.__init__(self, location = location)
		
		def get_draw_data(self):
			location = self.location
			return {
				'image':   location.over(),
				'size':   (location.xsize, location.ysize),
				'pos':    (0, 0),
				'zorder':  1e6,
			}

