init -1001 python:
	
	character_ext = 'png'
	
	
	character_walk_fps          =   4
	character_run_fps           =  12
	character_acceleration_time = 0.5
	character_walk_speed        =  50
	character_run_speed         = 150*1
	
	character_walk_acceleration = character_walk_speed / character_acceleration_time
	character_run_acceleration = character_run_speed / character_acceleration_time
	
	
	character_max_frame = 4
	character_max_direction = 4
	
	character_radius = 10 # used physics.rpy
	
	character_xsize = 48
	character_ysize = 96
	
	
	# directions
	to_forward = 3
	to_back = 0
	to_left = 1
	to_right = 2
	
	def characters_moved():
		for character in characters:
			if not character.get_auto() and not character.ended_move_waiting():
				return False
		return True
	can_exec_next_check_funcs.append(characters_moved)
	
	def characters_to_end():
		for character in characters:
			if not character.get_auto() and not character.ended_move_waiting():
				character.move_to_end()
	can_exec_next_skip_funcs.append(characters_to_end)
	
	
	character_run_allowed = True
	def set_run_allow(value):
		global character_run_allowed
		character_run_allowed = bool(value)
	def get_run_allow():
		return character_run_allowed
	
	
	def register_character_animation(character, anim_name, path, xoffset, yoffset,
	                                 count_frames, start_frame, end_frame, time = 1.0):
		if (type(xoffset), type(yoffset)) != (int, int):
			out_msg('register_character_animation',
			        'On registration of animation <' + str(anim_name) + '> of character <' + str(character) + '>\n' +
			        'set invalid offset: <' + str(xoffset) + ', ' + str(yoffset) + '>, expected ints')
			return
		
		if count_frames <= 0 or not (0 <= start_frame < count_frames) or not (0 <= end_frame < count_frames):
			out_msg('register_character_animation',
			        'On registration of animation <' + str(anim_name) + '> of character <' + str(character) + '>\n' +
			        'set invalid frames:\n' +
			        'count, start, end = ' + str(count_frames) + ', ' + str(start_frame) + ', ' + str(end_frame))
			return
		
		animations = character.animations
		if animations.has_key(anim_name):
			out_msg('register_character_animation', 'Animation <' + str(anim_name) + '> of character <' + str(character) + '> already exists')
			return
		
		animations[anim_name] = Object(
			name = anim_name,
			path = path,
			
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
	
	def characters_anim_ended():
		if cur_location:
			for obj in cur_location.objects:
				if isinstance(obj, Character) and not obj.ended_anim_waiting():
					return False
		return True
	can_exec_next_check_funcs.append(characters_anim_ended)
	
	def characters_anim_to_end():
		if cur_location:
			for obj in cur_location.objects:
				if isinstance(obj, Character):
					obj.anim_to_end()
	can_exec_next_skip_funcs.append(characters_anim_to_end)
	
	
	
	characters = []
	class Character(Object):
		def __init__(self, name, **kwargs):
			Object.__init__(self)
			characters.append(self)
			
			self.real_name = name
			self.unknown_name = kwargs.get('unknown_name', name)
			
			self.name = name
			self.name_prefix = kwargs.get('name_prefix', '')
			self.name_postfix = kwargs.get('name_postfix', '')
			self.color = kwargs.get('color', 0)
			if type(self.color) is not int:
				r, g, b, a = renpy.easy.color(self.color)
				self.color = (r << 16) + (g << 8) + b
			
			self.text_prefix = kwargs.get('text_prefix', '')
			self.text_postfix = kwargs.get('text_postfix', '')
			self.text_color = kwargs.get('text_color', 0xFFFF00)
			if type(self.text_color) is not int:
				r, g, b, a = renpy.easy.color(self.text_color)
				self.text_color = (r << 16) + (g << 8) + b
			
			# rpg-props:
			self.show_time = 0
			
			self.directory = None
			self.dress = None
			
			self.frame = 0
			self.direction = 0
			self.run = False
			self.pose = 'stay' # 'sit' | 'stay' | 'walk' | 'run'
			self.fps = character_walk_fps
			
			self.prev_update_time = get_game_time()
			self.moving_ended = True
			
			self.frame_time_offset = random.random() * 10
			
			self.x, self.y = 0, 0
			self.xoffset, self.yoffset = 0, 0
			
			self.xanchor, self.yanchor = 0.5, 0.8
			self.xsize, self.ysize = character_xsize, character_ysize
			self.crop = (0, 0, self.xsize, self.ysize)
			self.alpha = 0
			
			self.allowed_exits = set()
			
			self.location = None
			self.paths = []
			self.paths_index = 0
			self.point_index = 0
			
			self.animations = Object()
			self.animation = None
			self.start_anim_time = None
			self.end_wait_anim_time = None
			
			self.auto_actions = False
			self.actions = None
		
		def __str__(self):
			return str(self.name)
		
		def __call__(self, text):
			name = None if self.name is None else _(self.name)
			show_text(name, self.name_prefix, self.name_postfix, self.color,
			          text, self.text_prefix, self.text_postfix, self.text_color)
		
		
		# rpg-funcs:
		
		def make_rpg(self, directory, rpg_name, start_dress):
			self.directory = directory + ('' if directory.endswith('/') else '/')
			self.rpg_name = rpg_name
			self.set_dress(start_dress)
		
		def get_zorder(self):
			return self.y + self.yoffset
		def get_draw_data(self):
			return get_usual_location_object_data(self)
		
		def main(self):
			if self.start_anim_time is not None:
				return get_location_image(self.animation, self.animation.path, '', '', character_ext, False)
			return get_location_image(self, self.directory, self.rpg_name, self.dress, character_ext, False)
		
		
		def get_dress(self):
			return self.dress
		def set_dress(self, dress):
			self.dress = dress
		
		def get_direction(self):
			return self.direction
		def set_direction(self, direction):
			if direction is not None:
				self.direction = direction % character_max_direction
		
		def rotate_to(self, dx, dy):
			if abs(dx) > abs(dy):
				self.set_direction(to_left if dx < 0 else to_right)
			else:
				self.set_direction(to_forward if dy < 0 else to_back)
		
		def get_pose(self):
			return self.pose
		def set_pose(self, pose):
			expected = ('sit', 'stay', 'walk', 'run')
			if pose in expected:
				if pose != 'sit' and self.sit_object:
					self.stand_up()
				self.pose = pose
			else:
				self.pose = 'stay'
				out_msg('Character.set_pose', 'Unexpected pose <' + str(pose) + '>\n' + 'Expected: ' + ', '.join(expected))
		
		
		def set_frame(self, frame):
			self.frame = frame
		
		def update_crop(self):
			frame = self.frame
			if self.start_anim_time is None:
				pose = self.get_pose()
				if pose == 'sit':
					frame = character_max_frame
				elif pose == 'stay':
					frame = character_max_frame - 1
				else:
					frame %= character_max_frame
				
				y = self.direction * self.ysize
			else:
				y = 0
			x = frame * self.xsize
			
			self.crop = (x, y, self.xsize, self.ysize)
		
		
		def sit_down(self, obj):
			if not obj.sit_places:
				out_msg('Character.sit_down', 'obj.sit_places is empty')
				return False
			
			place_index = -1
			min_dist = 1e6
			for i in xrange(len(obj.sit_places)):
				if obj.on[i]:
					continue
				place_x, place_y, to_side = obj.sit_places[i]
				
				dist = get_dist(obj.x + place_x, obj.y - obj.ysize + place_y, self.x, self.y)
				if dist < min_dist:
					min_dist = dist
					place_index = i
			
			if place_index < 0:
				return False
			
			obj.on[place_index] = self
			place_x, place_y, to_side = obj.sit_places[place_index]
			
			self.old_x, self.old_y = self.x, self.y
			self.x, self.y = obj.x + place_x, obj.y - obj.ysize + place_y
			self.invisible = True
			self.set_direction(to_side)
			self.set_pose('sit')
			self.sit_object = obj
			
			return True
		
		def stand_up(self):
			sit_object = self.sit_object
			if not sit_object:
				return
			
			for i in xrange(len(sit_object.on)):
				if sit_object.on[i] is self:
					sit_object.on[i] = None
					break
			self.sit_object = None
			
			self.invisible = False
			if self.get_pose() == 'sit':
				self.set_pose('stay')
			
			if self.old_x is not None and self.old_y is not None:
				self.x, self.y = self.old_x, self.old_y
				self.old_x, self.old_y = None, None
		
		
		def allow_exit(self, location_name, place_name = None):
			location = rpg_locations.get(location_name, None)
			if location is None:
				out_msg('Character.allow_exit', 'Location <' + str(location_name) + '> is not registered')
				return
			
			if place_name is not None:
				if location.places.has_key(place_name):
					self.allowed_exits.add((location_name, place_name))
				else:
					out_msg('Character.allow_exit', 'Place <' + str(place_name) + '> in location <' + location_name + '> not found')
				return
			
			for place_name in location.places:
				self.allowed_exits.add((location_name, place_name))
		
		def disallow_exit(location_name, place_name = None):
			location = rpg_locations.get(location_name, None)
			if location is None:
				out_msg('Character.disallow_exit', 'Location <' + str(location_name) + '> is not registered')
				return
			if place_name is not None and not location.places.has_key(place_name):
				out_msg('Character.disallow_exit', 'Place <' + str(place_name) + '> in location <' + location_name + '> not found')
				return
			
			tmp_allowed_exits = set(self.allowed_exits)
			for tmp_loc, tmp_place in tmp_allowed_exits:
				if tmp_loc == location_name and (tmp_place == place_name or place_name is None):
					self.allowed_exits.remove((tmp_loc, tmp_place))
		
		
		
		def move_to_place(self, place_name, run = False, wait_time = -1, brute_force = False):
			place_names = [place_name] if place_name else None
			return self.move_to_places(place_names, run, wait_time, brute_force)
		
		
		def move_to_places(self, place_names, run = False, wait_time = -1, brute_force = False):
			self.paths = []
			self.paths_index = 0
			self.point_index = 0
			self.moving_ended = True
			self.set_pose('stay')
			if place_names is None:
				return False
			
			if self.location is None:
				out_msg('Character.move_to_places', str(self) + ' not on location')
				return False
			
			if self is me:
				ignore_prev_rpg_control()
				set_rpg_control(False)
			
			path_update_locations()
			
			if isinstance(place_names, tuple):
				place_names = list(place_names)
			
			last = place_names[-1]
			if type(last) is int:
				next = place_names[last]
				place_names.insert(-1, next)
			
			res = True # all paths found?
			from_location_name = self.location.name
			from_x, from_y = self.x, self.y
			
			banned = [exit for exit in location_banned_exits if exit not in self.allowed_exits]
			
			for i in xrange(len(place_names)):
				place_elem = place_names[i]
				
				if type(place_elem) is int:
					self.paths.append((place_elem % len(place_names) + 1))
					break
				
				if isinstance(place_elem, (list, tuple)):
					location_name, place_elem = place_elem
				else:
					location_name = from_location_name
				
				location = rpg_locations.get(location_name, None)
				if not location:
					out_msg('Character.move_to_places', 'Location <' + str(location_name) + '> is not registered')
					return False
				
				if type(place_elem) is str:
					place = location.places.get(place_elem, None)
					if place is None:
						out_msg('Character.move_to_places', 'Place <' + str(place_elem) + '> in location <' + location_name + '> not found')
						return False
				else:
					place = place_elem
				
				to_x, to_y = get_place_center(place)
				
				path = path_between_locations(from_location_name, int(from_x), int(from_y), location_name, int(to_x), int(to_y), banned, bool(brute_force))
				if not path:
					res = False
					if from_location_name == location_name:
						path = (to_x, to_y)
					else:
						path = (location_name, {'x': to_x, 'y': to_y}, to_x, to_y)
				self.paths.append(path)
				
				from_location_name = location_name
				from_x, from_y = to_x, to_y
			
			
			self.moving_ended = False
			
			pose = 'run' if run else 'walk'
			self.set_pose(pose)
			g = globals()
			for prop in ('fps', 'speed', 'acceleration'):
				self[prop] = g['character_' + pose + '_' + prop] # for example: self.fps = character_run_fps
			
			self.prev_update_time = get_game_time()
			if wait_time >= 0:
				self.end_stop_time = self.prev_update_time + wait_time
			else:
				self.end_stop_time = None
			
			return res
		
		def update_moving(self, dtime):
			if not self.paths:
				if not (self is me and get_rpg_control()):
					self.set_pose('stay')
				self.moving_ended = True
				return
			
			xstart, ystart = self.x, self.y
			
			path = self.paths[self.paths_index]
			location_name, place = self.location.name, None
			first_step = 0
			
			while dtime > 0:
				to_x, to_y = path[self.point_index], path[self.point_index + 1]
				
				if type(to_x) is str:
					place = get_location_place(self)
					if place:
						self.set_direction(place.to_side)
					
					location_name, place_name = to_x, to_y
					place = rpg_locations[location_name].places[place_name]
					coords_before_exit = self.x, self.y
					self.x, self.y = place.x + place.xsize / 2, place.y + place.ysize / 2
					first_step = self.point_index + 2
					
				else:
					dx, dy = to_x - self.x, to_y - self.y
					if dx or dy:
						# rotation
						if not (dx and dy) or self.point_index == first_step: # not diagonal or first step
							if abs(dx) + abs(dy) > 3: # not very short step
								self.rotate_to(dx, dy)
							else:
								first_step += 2
						
						need_dist = math.sqrt(dx*dx + dy*dy)
						need_time = need_dist / self.speed
						
						if need_time < dtime:
							self.x, self.y = to_x, to_y
							dtime -= need_time
						else:
							self.x += dx * dtime / need_time
							self.y += dy * dtime / need_time
							dtime = 0
							break
				
				self.point_index += 2
				if self.point_index == len(path):
					self.point_index = 0
					
					self.paths_index += 1
					if self.paths_index == len(self.paths):
						self.paths = []
						self.set_pose('stay')
						self.moving_ended = True
						
						dtime = 0
						self.prev_update_time = get_game_time()
						break
					
					path = self.paths[self.paths_index]
					if type(path) is int:
						if path < 0:
							path -= 1
						self.paths_index = path
						break
			
			self.prev_update_time -= dtime
			if self.location.name != location_name:
				x, y = self.x, self.y
				self.x, self.y = coords_before_exit
				show_character(self, {'x': x, 'y': y}, location_name)
		
		def move_to_end(self):
			if self.end_stop_time and self.end_stop_time > get_game_time():
				dtime = (get_game_time() - self.prev_update_time) + (self.end_stop_time - get_game_time())
			else:
				dtime = 1000
			
			self.end_stop_time = None
			self.update_moving(dtime)
		
		def ended_move_waiting(self):
			if self.end_stop_time:
				return self.end_stop_time <= get_game_time()
			if self.paths and type(self.paths[-1]) is int: # cycled path
				return True
			return self.moving_ended
		
		
		def start_animation(self, anim_name, repeat = 0, wait_time = -1):
			if not self.animations.has_key(anim_name):
				out_msg('start_animation', 'Animation <' + str(anim_name) + '> not found in character <' + str(self) + '>')
				return
			
			animation = self.animation = self.animations[anim_name]
			animation.first_update = True
			
			self.xoffset, self.yoffset = animation.xoffset, animation.yoffset
			
			self.start_anim_time = get_game_time()
			self.repeat = int(repeat)
			
			if wait_time >= 0:
				self.end_wait_anim_time = get_game_time() + wait_time
			else:
				self.end_wait_anim_time = None
		
		def remove_animation(self):
			self.start_anim_time = None
			self.end_wait_anim_time = None
			self.xoffset, self.yoffset = 0, 0
			self.xsize, self.ysize = character_xsize, character_ysize
		
		def anim_to_end(self):
			self.remove_animation()
		
		def ended_anim_waiting(self):
			if self.start_anim_time is None:
				return True
			if self.end_wait_anim_time is not None and self.end_wait_anim_time < get_game_time():
				return True
			
			if self.repeat < 0:
				return True
			if self.repeat > 0:
				return False
			return self.start_anim_time + self.animation.time <= get_game_time()
		
		
		def get_auto(self):
			return self.auto_actions
		def set_auto(self, value):
			if not value and self.actions:
				self.actions.stop()
			self.auto_actions = bool(value)
		
		def get_actions(self):
			return self.actions
		def set_actions(self, actions):
			if self.actions:
				self.actions.stop()
			self.actions = actions and actions.copy(self)
			return self.actions
		
		
		def update(self):
			dtime = get_game_time() - self.prev_update_time
			self.prev_update_time = get_game_time()
			
			if self.get_auto() and self.actions:
				self.actions.update()
			self.alpha = min((get_game_time() - self.show_time) / location_fade_time, 1)
			
			if self.start_anim_time is None:
				self.set_frame(int((get_game_time() + self.frame_time_offset) * self.fps))
			else:
				animation = self.animation
				anim_dtime = get_game_time() - self.start_anim_time
				
				time_k = 1
				if animation.time > 0:
					if anim_dtime > animation.time:
						if self.repeat:
							self.start_anim_time = get_game_time()
							time_k = 0
						if self.repeat > 0:
							self.repeat -= 1
					else:
						time_k = anim_dtime / animation.time
				
				if animation.first_update:
					animation.first_update = False
					
					if not animation.loaded:
						animation.loaded = True
						animation.xsize, animation.ysize = get_image_size(self.main())
						animation.xsize = int(math.ceil(animation.xsize / animation.count_frames))
					
					self.xsize, self.ysize = animation.xsize, animation.ysize
				
				start_frame = animation.start_frame
				end_frame = animation.end_frame
				if end_frame < start_frame:
					frame = start_frame - int((start_frame - end_frame + 1) * time_k)
					frame = in_bounds(frame, end_frame, start_frame)
				else:
					frame = start_frame + int((end_frame - start_frame + 1) * time_k)
					frame = in_bounds(frame, start_frame, end_frame)
				
				self.set_frame(frame)
			
			if self.get_pose() in ('walk', 'run'):
				self.update_moving(dtime)
			self.update_crop()
	
	
	def set_name(who, name):
		g = globals()
		if g.has_key(who):
			g[who].name = str(name)
		else:
			out_msg('set_name', 'Character <' + str(who) + '> not found')
	meet = set_name
	
	def make_names_unknown():
		for character in characters:
			character.name = character.unknown_name
	def make_names_known():
		for character in characters:
			character.name = character.real_name
	
	def forget_character(character):
		characters.remove(character)
		if character.location:
			character.location.objects.remove(character)
			character.location = None
	
	
	
	def show_character(character, place, location = None, auto_change_location = True, hiding = True):
		prev_character_location = character.location
		prev_character_pos = get_place_center(character)
		
		if location is None:
			if cur_location is None:
				out_msg('show_character', 'Current location is not defined, need to call set_location')
				return
			location = cur_location
		elif type(location) is str:
			if not rpg_locations.has_key(location):
				out_msg('show_character', 'Location <' + location + '> not registered')
				return
			location = rpg_locations[location]
		
		if type(place) is str:
			place_name = place
			place = location.get_place(place)
			if not place:
				out_msg('show_character', 'Place <' + place_name + '> not found in location <' + str(location.name) + '>')
				return
		
		location_disappearance = draw_location is not cur_location
		if character is (cam_object if not location_disappearance else me) and auto_change_location:
			set_location(location.name, place)
			return
		
		place_pos = get_place_center(place)
		is_old_place = (prev_character_pos == place_pos) and (prev_character_location is location)
		
		character.show_time = -100
		character.alpha = 1
		if not prev_character_location:
			location.objects.append(character)
		elif not is_old_place:
			character.show_time = get_game_time()
			character.alpha = 0
			if hiding:
				create_hiding_object(character)
			prev_character_location.objects.remove(character)
			location.objects.append(character)
		character.location = location
		
		character.stand_up() # forget coords to stand_up, if sit
		character.x, character.y = place_pos
	
	def hide_character(character):
		if character.location:
			create_hiding_object(character)
			character.location.objects.remove(character)
			character.location = None
			character.paths = []
		else:
			out_msg('hide_character', 'Character <' + character.real_name + ', ' + character.unknow_name + '> not shown')
	
	
	tmp_character = Character('TMP', color = 0xFFFFFF)
	
	narrator = Character('')
	th = Character('', text_prefix='~', text_postfix='~')
	extend = Character(None)

