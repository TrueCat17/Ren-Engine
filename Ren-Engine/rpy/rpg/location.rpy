init -1002 python:
	
	location_ext = 'png'
	
	
	location_start_time = 0
	location_fade_time = 0.5
	
	
	location_was_show = True
	def location_showed():
		return location_was_show
	can_exec_next_check_funcs.append(location_showed)
	
	def location_show():
		global location_start_time
		location_start_time = time.time() - location_fade_time * 2
	can_exec_next_skip_funcs.append(location_show)
	
	
	prev_location_name = None
	cur_location = None
	cur_location_name = None
	cur_place_name = None
	cur_to_place = None
	
	
	location_zoom = 1.0
	
	
	locations = dict()
	
	def register_location(name, path_to_images, is_room, xsize, ysize):
		if locations.has_key(name):
			out_msg('register_location', 'Location <' + name + '> already registered')
		else:
			location = Location(name, path_to_images, is_room, xsize, ysize)
			locations[name] = location
	
	def register_place(location_name, place_name, x, y, xsize, ysize, to_side = None):
		if not locations.has_key(location_name):
			out_msg('register_place', 'Location <' + location_name + '> not registered')
			return
		
		location = locations[location_name]
		if location.get_place(place_name):
			out_msg('register_place', 'Place <' + place_name + '> in location <' + self.name + '> already exists')
			return
		
		place = Place(place_name, x, y, xsize, ysize, to_side)
		location.add_place(place, place_name)
		location.path_need_update = True
	
	def register_exit(location_name, to_location_name, to_place_name, x, y, xsize, ysize):
		if not locations.has_key(location_name):
			out_msg('register_exit', 'Location <' + location_name + '> not registered')
			return
		
		location = locations[location_name]
		exit = Exit(to_location_name, to_place_name, x, y, xsize, ysize)
		location.add_exit(exit)
		location.path_need_update = True
	
	
	def set_location(location_name, place):
		if not locations.has_key(location_name):
			out_msg('set_location', 'Location <' + location_name + '> not registered')
			return
		
		if type(place) is str:
			if not locations[location_name].get_place(place):
				out_msg('set_location', 'Place <' + place + '> in location <' + location_name + '> not found')
				return
			place = locations[location_name].get_place(place)
		
		if not has_screen('location'):
			show_screen('location')
			show_screen('inventory')
		
		global prev_location_name, cur_location, cur_location_name, cur_to_place
		prev_location_name = cur_location_name
		cur_location = locations[location_name]
		cur_location_name = location_name
		cur_to_place = place
		
		main = cur_location.main()
		real_width, real_height = get_image_size(main)
		reg_width, reg_height = cur_location.xsize, cur_location.ysize
		if reg_width != real_width or reg_height != real_height:
			reg_size = str(reg_width) + 'x' + str(reg_height)
			real_size = str(real_width) + 'x' + str(real_height)
			out_msg('set_location', 
					'Location sizes on registration (' + reg_size + ') not equal to real sizes (' + real_size + ')\n' + 
					'Location: <' + cur_location.name + '>\n' + 
					'Main image: <' + main + '>')
		
		end_location_ambience(cur_location)
		ignore_prev_rpg_control()
		
		time_changing = times['next_name'] is not None
		
		global draw_location, location_start_time, location_was_show, cam_object
		if (draw_location is None) or (draw_location is cur_location and not time_changing):
			location_start_time = 0
			location_was_show = True
			draw_location = cur_location
			cam_object = me
			start_location_ambience()
		else:
			location_start_time = time.time()
			location_was_show = False
			
			x, y = get_place_center(cam_object)
			cam_object = {'x': x, 'y': y}
		
		show_character(me, cur_to_place, auto_change_location = False)
		if prev_location_name is None:
			me.show_time = 0
		
		if cur_to_place.has_key('to_side'):
			me.set_direction(cur_to_place['to_side'])
	
	def hide_location():
		global cur_location, cur_location_name, cur_to_place
		cur_location = cur_location_name = None
		cur_to_place = None
		
		global draw_location
		draw_location = None
		end_location_ambience(None)
	
	
	location_banned_exit_destinations = []
	def ban_exit_destination(location_name, place_name = None):
		location = locations.get(location_name, None)
		if location is None:
			out_msg('ban_exit_destination', 'Location <' + str(location_name) + '> is not registered')
			return
		
		if place_name is not None:
			if location.places.has_key(place_name):
				location_banned_exit_destinations.append((location_name, place_name))
			else:
				out_msg('ban_exit_destination', 'Place <' + str(place_name) + '> in location <' + location_name + '> not found')
			return
		
		for place_name in location.places:
			location_banned_exit_destinations.append((location_name, place_name))
	
	def unban_exit_destination(location_name, place_name = None):
		location = locations.get(location_name, None)
		if location is None:
			out_msg('unban_exit_destination', 'Location <' + str(location_name) + '> is not registered')
			return
		if place_name is not None and not location.places.has_key(place_name):
			out_msg('unban_exit_destination', 'Place <' + str(place_name) + '> in location <' + location_name + '> not found')
			return
		
		global location_banned_exit_destinations
		i = 0
		while i < len(location_banned_exit_destinations):
			tmp_loc, tmp_place = location_banned_exit_destinations[i]
			if tmp_loc == location_name and (tmp_place == place_name or place_name is None):
				location_banned_exit_destinations = location_banned_exit_destinations[:i] + location_banned_exit_destinations[i+1:]
			else:
				i += 1
	
	
	def get_location_image(obj, directory, name, name_postfix, ext, is_free, need = True):
		if obj.cache is None:
			obj.cache = dict()
		
		mode = times['current_name']
		key = name, name_postfix, mode
		if obj.cache.has_key(key):
			return obj.cache[key]
		
		if name_postfix:
			name_postfix = '_' + name_postfix
		file_name = name + name_postfix
		
		path = directory + file_name + '_' + mode + '.' + ext
		if not os.path.exists(path):
			path = directory + file_name + '.' + ext
			if os.path.exists(path):
				if not is_free:
					r, g, b = location_time_rgb
					path = im.recolor(path, r, g, b)
			else:
				if need:
					out_msg('get_location_image', 'File <' + path + '> not found')
				path = None
		
		obj.cache[key] = path
		return path
	
	def set_location_scales(name, min_scale, count_scales):
		if not locations.has_key(name):
			out_msg('set_location_scales', 'Location <' + str(name) + '> is not registered')
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
		
		location = locations[name]
		location.min_scale = min_scale
		location.count_scales = count_scales
		location.path_need_update = True
	
	
	class Location(Object):
		def __init__(self, name, directory, is_room, xsize, ysize):
			Object.__init__(self)
			
			self.name = name
			self.directory = directory + ('' if directory.endswith('/') else '/')
			
			self.is_room = is_room
			self.xsize, self.ysize = xsize, ysize
			
			self.places = dict()
			self.exits = []
			
			self.objects = [self]
			if self.over():
				self.objects.append(LocationOver(self))
			
			self.ambience_paths = None
			self.ambience_volume = 1.0
			
			self.path_need_update = True
			self.min_scale = 8
			self.count_scales = 2
		
		def __str__(self):
			return '<Location ' + str(self.name) + '>'
		
		def get_zorder(self):
			return 0
		def get_draw_data(self):
			return [{
				'image':   self.main(),
				'size':   (self.xsize, self.ysize),
				'pos':    (0, 0),
				'anchor': (0, 0),
				'crop':   (0, 0, 1.0, 1.0),
				'alpha':   1,
				'zorder':  self.get_zorder(),
			}]
		
		def main(self):
			return get_location_image(self, self.directory, 'main', '', location_ext, False)
		def over(self):
			return get_location_image(self, self.directory, 'over', '', location_ext, False, False)
		def free(self):
			return get_location_image(self, self.directory, 'free', '', location_ext, True, False)
		
		def preload(self):
			for image in self.main(), self.over(), self.free():
				if image:
					load_image(image)
		
		def add_place(self, place, place_name):
			self.places[place_name] = place
		
		def get_place(self, place_name):
			return self.places.get(place_name, None)
		
		def add_exit(self, exit):
			self.exits.append(exit)
		
		
		def update_pos(self):
			self.x, self.y = get_camera_params(self)
	
	
	class Place(Object):
		def __init__(self, name, x, y, xsize, ysize, to_side):
			Object.__init__(self)
			self.name = name
			self.x, self.y = x, y
			self.xsize, self.ysize = xsize, ysize
			self.to_side = to_side
		
		def __str__(self):
			return '<Place ' + self.name + '>'
		
		def inside(self, x, y):
			return self.x <= x and x <= self.x + self.xsize and self.y <= y and y <= self.y + self.ysize
	
	class Exit(Object):
		def __init__(self, to_location_name, to_place_name, x, y, xsize, ysize):
			Object.__init__(self)
			self.to_location_name = to_location_name
			self.to_place_name = to_place_name
			self.x, self.y = x, y
			self.xsize, self.ysize = xsize, ysize
		
		def __str__(self):
			return '<Exit to ' + self.to_location_name + ':' + self.to_place_name + '>'
		
		def inside(self, x, y):
			return self.x <= x and x <= self.x + self.xsize and self.y <= y and y <= self.y + self.ysize
	
	
	def get_place_center(place, align = (0.5, 0.5)):
		x, y = place['x'], place['y']
		
		w = place['xsize'] if place.has_key('xsize') else 0
		h = place['ysize'] if place.has_key('ysize') else 0
		
		xa = get_absolute(place['xanchor'], w) if place.has_key('xanchor') else 0
		ya = get_absolute(place['yanchor'], h) if place.has_key('yanchor') else 0
		
		return int(x - xa + align[0] * w), int(y - ya + align[1] * h)
	
	sit_action = False
	sit_down = False
	stand_up = False
	
	exec_action = False
	was_out_exit = False
	was_out_place = False
	
	def get_location_exit():
		global was_out_exit, was_out_place
		
		if not exec_action and cur_location.is_room:
			was_out_exit = True
			return None
		
		for exit in cur_location.exits:
			if not exit.inside(me.x, me.y):
				continue
			
			destination = (exit.to_location_name, exit.to_place_name)
			if destination in location_banned_exit_destinations and destination not in me.allowed_exit_destinations:
				continue
			
			if globals().has_key('can_exit_to') and not can_exit_to(exit.to_location_name, exit.to_place_name):
				continue
			
			if not was_out_exit:
				return None
			if not exec_action and locations[exit.to_location_name].is_room:
				return None
			
			was_out_exit = False
			was_out_place = True
			return exit
		
		was_out_exit = True
		return None
	
	def get_location_place():
		global exec_action, was_out_place
		
		for place_name in cur_location.places.keys():
			place = cur_location.places[place_name]
			if place.inside(me.x, me.y):
				if was_out_place:
					exec_action = False
				if exec_action or was_out_place:
					was_out_place = False
					return place_name
				return None
		else:
			was_out_place = True
		
		return None
	
	
	def path_update_locations():
		for name, location in locations.iteritems():
			if not location.path_need_update: continue
			location.path_need_update = False
			
			free = location.free()
			if not free:
				free = im.rect('#000', location.xsize, location.ysize)
			
			objects = []
			for obj in location.objects:
				if not isinstance(obj, LocationObject): continue
				
				free_obj = obj.free()
				if not free_obj: continue
				
				x = int(obj.x - get_absolute(obj.xanchor, obj.xsize) + obj.xoffset)
				y = int(obj.y - get_absolute(obj.yanchor, obj.ysize) + obj.yoffset)
				objects.extend((free_obj, x, y))
			
			places = []
			for place_name, place in location.places.iteritems():
				x, y = get_place_center(place)
				places.extend((place_name, x, y))
			
			exits = []
			for exit in location.exits:
				x, y = get_place_center(exit)
				exits.extend((exit.to_location_name, exit.to_place_name, x, y))
			
			path_update_location(name, free, character_radius, objects, places, exits, location.min_scale, location.count_scales)
	
	
	class LocationOver(Object):
		def __init__(self, location):
			Object.__init__(self)
			self.location = location
		
		def get_zorder(self):
			return 1e6
		
		def get_draw_data(self):
			return {
				'image':   self.location.over(),
				'size':   (self.location.xsize, self.location.ysize),
				'pos':    (0, 0),
				'anchor': (0, 0),
				'crop':   (0, 0, 1.0, 1.0),
				'alpha':   1.0,
				'zorder':  self.get_zorder()
			}

