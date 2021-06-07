init -1002 python:
	
	location_ext = 'png'
	
	
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
	
	
	cur_location = None
	cur_location_name = None
	cur_exit = None
	cur_place = None
	cur_place_name = None
	
	
	location_zoom = 1.0
	
	
	rpg_locations = {}
	
	def register_location(name, path_to_images, is_room, xsize, ysize):
		if rpg_locations.has_key(name):
			out_msg('register_location', 'Location <' + name + '> already registered')
		else:
			rpg_locations[name] = RpgLocation(name, path_to_images, is_room, xsize, ysize)
	
	def register_place(location_name, place_name, x, y, xsize, ysize, to = None):
		if not rpg_locations.has_key(location_name):
			out_msg('register_place', 'Location <' + location_name + '> not registered')
			return
		
		location = rpg_locations[location_name]
		if location.get_place(place_name):
			out_msg('register_place', 'Place <' + place_name + '> in location <' + self.name + '> already exists')
			return
		
		
		if to is not None:
			if type(to) not in (list, tuple):
				out_msg('register_place', '<to> must be None, list or tuple')
				to = None
			else:
				if len(to) == 3:
					to = to[0], to[1], to[2], None
				if len(to) != 4:
					out_msg('register_place', 'Expects <to> = None, (exit_side, to_location_name, to_place_name) or (exit_side, to_location_name, to_place_name, to_side)')
					to = None
		if not to:
			to = None, None, None, None
		
		place = RpgPlace(place_name, x, y, xsize, ysize, to)
		location.add_place(place, place_name)
		location.path_need_update = True
	
	
	def set_location(location_name, place, character = None):
		character = character or me
		
		if not rpg_locations.has_key(location_name):
			out_msg('set_location', 'Location <' + location_name + '> not registered')
			return
		
		if type(place) is str:
			if not rpg_locations[location_name].get_place(place):
				out_msg('set_location', 'Place <' + place + '> in location <' + location_name + '> not found')
				return
			place = rpg_locations[location_name].get_place(place)
		
		if not has_screen('location'):
			show_screen('location')
		
		global cur_location, cur_location_name, cur_place, cur_place_name
		prev_location = cur_location
		cur_location = rpg_locations[location_name]
		cur_location_name = location_name
		cur_place = place
		cur_place_name = place['name'] if place.has_key('name') else None
		
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
		
		global draw_location, location_start_time, location_was_show
		if (draw_location is None) or (draw_location is cur_location and not time_changing):
			location_start_time = -100
			location_was_show = True
			draw_location = cur_location
			cur_location.cam_object = prev_location.cam_object if prev_location else character
			start_location_ambience()
		else:
			location_start_time = get_game_time()
			location_was_show = False
			cur_location.cam_object = character
		cur_location.cam_object_old = None
		
		show_character(character, place, auto_change_location = False)
		if prev_location is None:
			character.show_time = -100
		
		if character is me:
			for place in cur_location.places.itervalues():
				if place.inside(me.x, me.y):
					break
			else:
				global was_out_exit
				was_out_exit = True
	
	def hide_location():
		global cur_location, cur_location_name, cur_place, cur_place_name
		cur_location = cur_location_name = None
		cur_place = cur_place_name = None
		
		global draw_location
		draw_location = None
		end_location_ambience(None)
	
	
	location_banned_exits = set()
	def ban_exit(location_name, place_name = None):
		location = rpg_locations.get(location_name, None)
		if location is None:
			out_msg('ban_exit', 'Location <' + str(location_name) + '> is not registered')
			return
		
		if place_name is not None:
			if location.places.has_key(place_name):
				location_banned_exits.add((location_name, place_name))
			else:
				out_msg('ban_exit', 'Place <' + str(place_name) + '> in location <' + location_name + '> not found')
			return
		
		for place_name in location.places:
			location_banned_exits.add((location_name, place_name))
	
	def unban_exit(location_name, place_name = None):
		location = rpg_locations.get(location_name, None)
		if location is None:
			out_msg('unban_exit', 'Location <' + str(location_name) + '> is not registered')
			return
		if place_name is not None and not location.places.has_key(place_name):
			out_msg('unban_exit', 'Place <' + str(place_name) + '> in location <' + location_name + '> not found')
			return
		
		tmp_banned_exits = set(location_banned_exits)
		for tmp_loc, tmp_place in tmp_banned_exits:
			if tmp_loc == location_name and (tmp_place == place_name or place_name is None):
				location_banned_exits.remove((tmp_loc, tmp_place))
	
	
	def get_location_image(obj, directory, name, name_postfix, ext, is_free, need = True):
		if obj.cache is None:
			obj.cache = {}
		
		mode = times['current_name']
		key = name, name_postfix, mode
		if obj.cache.has_key(key):
			return obj.cache[key]
		
		file_name = name
		if name_postfix:
			file_name += '_' + name_postfix
		
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
		if not rpg_locations.has_key(name):
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
		
		location = rpg_locations[name]
		location.min_scale = min_scale
		location.count_scales = count_scales
		location.path_need_update = True
	
	
	class RpgLocation(Object):
		def __init__(self, name, directory, is_room, xsize, ysize):
			Object.__init__(self)
			
			self.name = name
			self.directory = directory + ('' if directory.endswith('/') else '/')
			
			self.is_room = is_room
			self.xsize, self.ysize = xsize, ysize
			
			self.places = {}
			
			self.objects = [self]
			if self.over():
				self.objects.append(RpgLocationOver(self))
			
			self.ambience_paths = None
			self.ambience_volume = 1.0
			
			self.path_need_update = True
			self.min_scale = 8
			self.count_scales = 2
			
			self.cam_object = None
			self.cam_object_old = None
		
		def __str__(self):
			return '<RpgLocation ' + str(self.name) + '>'
		
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
		
		
		def update_pos(self):
			self.x, self.y = get_camera_params(self)
	
	
	class RpgPlace(Object):
		def __init__(self, name, x, y, xsize, ysize, to):
			Object.__init__(self)
			self.name = name
			self.x, self.y = x, y
			self.xsize, self.ysize = xsize, ysize
			self.exit_side, self.to_location_name, self.to_place_name, self.to_side = to
		
		def __str__(self):
			return '<RpgPlace ' + self.name + '>'
		
		def get_rect(self, of_exit = False):
			x, y, w, h = self.x, self.y, self.xsize, self.ysize
			w2, h2 = w / 2, h / 2
			
			side = self.exit_side
			if not side:
				return x, y, w, h
			
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
			w = place['xsize'] if place.has_key('xsize') else 0
			h = place['ysize'] if place.has_key('ysize') else 0
		
		xa = get_absolute(place['xanchor'], w) if place.has_key('xanchor') else 0
		ya = get_absolute(place['yanchor'], h) if place.has_key('yanchor') else 0
		
		if anchor is None:
			if isinstance(place, Character):
				anchor = (xa, ya)
			else:
				anchor = (0.5, 0.5)
		
		return int(x - xa + get_absolute(anchor[0], w)), int(y - ya + get_absolute(anchor[1], h))
	
	rpg_event = None
	rpg_event_object = None
	rpg_events = set()
	
	was_out_exit = False
	def get_location_exit():
		global was_out_exit
		
		if 'action' not in rpg_events and cur_location.is_room:
			was_out_exit = True
			return None
		
		for place in cur_location.places.itervalues():
			if not place.inside_exit(me.x, me.y):
				continue
			
			if cur_location.is_room or rpg_locations[place.to_location_name].is_room:
				if 'action' not in rpg_events:
					return None
				rpg_events.remove('action')
			
			loc_place = (cur_location.name, place.name)
			if loc_place in location_banned_exits and loc_place not in me.allowed_exits:
				rpg_events.add('no_exit')
				continue
			
			if not was_out_exit:
				return None
			
			was_out_exit = False
			return place
		
		was_out_exit = True
		return None
	
	def get_location_place(character = None):
		ch = character or me
		location = ch.location
		
		for place in location.places.itervalues():
			if place.inside(ch.x, ch.y):
				return place
		return None
	
	
	def path_update_locations():
		for location in rpg_locations.itervalues():
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
				
				x = int(obj.x - get_absolute(obj.xanchor, obj.xsize) + obj.xoffset)
				y = int(obj.y - get_absolute(obj.yanchor, obj.ysize) + obj.yoffset)
				objects.extend((free_obj, x, y))
			
			places = []
			for place in location.places.itervalues():
				x, y = int(place.x + place.xsize / 2), int(place.y + place.ysize / 2)
				places.extend((place.name, x, y, place.to_location_name or '', place.to_place_name or ''))
			
			path_update_location(location.name, free, character_radius, objects, places, location.min_scale, location.count_scales)
	
	
	class RpgLocationOver(Object):
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

