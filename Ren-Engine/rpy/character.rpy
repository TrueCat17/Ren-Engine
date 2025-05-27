init -1001 python:
	
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
			msg = 'On registration of animation <%s> of character <%s>\n' + 'set invalid pos: <%s, %s>, expected ints'
			out_msg('register_character_animation', msg % (anim_name, character, xoffset, yoffset))
			return
		
		if (type(count_frames), type(start_frame), type(end_frame)) != (int, int, int):
			msg = 'On registration of animation <%s> of character <%s>\n'
			msg += 'params count_frame, start_frame and end_frame must be ints\n'
			msg += '(got %s, %s, %s)'
			out_msg('register_character_animation', msg % (anim_name, character, count_frames, start_frame, end_frame))
			return
		if count_frames <= 0 or not (0 <= start_frame < count_frames) or not (0 <= end_frame < count_frames):
			msg = 'On registration of animation <%s> of character <%s>\n' + 'set invalid frames:\n' + 'count, start, end = %s, %s, %s'
			out_msg('register_character_animation', msg % (anim_name, character, count_frames, start_frame, end_frame))
			return
		
		animations = character.animations
		if anim_name in animations:
			out_msg('register_character_animation', 'Animation <%s> of character <%s> already exists' % (anim_name, character))
			return
		
		obj = animations[anim_name] = SimpleObject()
		obj.name = anim_name
		obj.path = path
		
		obj.count_frames = count_frames
		obj.start_frame  = start_frame
		obj.end_frame    = end_frame
		obj.time         = float(time)
		
		obj.xoffset = xoffset
		obj.yoffset = yoffset
		obj.xsize = 0
		obj.ysize = 0
		obj.loaded = False
	
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
				if isinstance(obj, Character) and not obj.ended_anim_waiting():
					obj.anim_to_end()
	can_exec_next_skip_funcs.append(characters_anim_to_end)
	
	
	
	characters = []
	class Character(SimpleObject):
		radius = 10 # used in physics.rpy
		
		def __init__(self, name, **properties):
			characters.append(self)
			
			SimpleObject.__init__(self)
			
			self.real_name = name
			self.unknown_name = properties.get('unknown_name', name)
			self.dynamic = properties.get('dynamic', False)
			self.name = Eval(name) if self.dynamic and type(name) is str else name
			
			
			# rpg-props:
			
			self.type = None # this is not rpg-location-object
			
			self.walk_fps =  4
			self.run_fps  = 12
			self.walk_speed =  50
			self.run_speed  = 150
			
			self.sit_frame = 4 # first (left) = 0
			self.stay_frame = 3
			self.count_of_moving_frames = 4
			self.count_of_directions = 4
			
			
			self.show_time = 0
			
			self.directory = None
			self.rpg_name = None
			self.dress = None
			self.overs = []
			
			self.frame = 0
			self.direction = 0
			self.run = False
			self.pose = 'stay' # 'sit' | 'stay' | 'walk' | 'run'
			self.fps = 0
			self.speed = 0
			
			self.prev_update_time = get_game_time()
			self.end_stop_time = None
			self.moving_ended = True
			
			self.frame_time_offset = random.random() * 10
			
			self.x = 0
			self.y = 0
			self.xoffset = 0
			self.yoffset = 0
			
			self.xanchor = 0.5
			self.yanchor = 0.8
			self.xsize = 48
			self.ysize = 96
			self.crop = (0, 0, self.xsize, self.ysize)
			self.alpha = 0
			
			self.allowed_exits = set()
			
			self.location = None
			self.paths = []
			self.path_index = 0
			self.point_index = 0
			
			self.animations = {}
			self.animation = None
			self.start_anim_time = None
			self.end_wait_anim_time = None
			
			self.auto_actions = False
			self.actions = None
			
			self.sit_object = None
			self.invisible = False
			
			
			for prefix_from, prefix_to in [('who_', 'name_text_'), ('what_', 'dialogue_text_')]:
				for prop in ('font', 'size', 'color', 'outlinecolor', 'background', 'prefix', 'suffix'):
					prop_from = prefix_from + prop
					prop_to   = prefix_to   + prop
					
					if prop_from not in properties and prefix_from == 'who_': # for example, try check <color> instead of <who_color>
						prop_from = prop
					if prop_from not in properties:
						continue
					
					value = properties[prop_from]
					if prop.endswith('color'):
						value = color_to_int(value)
					self[prop_to] = value
			
			for prop, value in properties.items():
				if prop.startswith('dialogue_'):
					self[prop] = value
			
			self.inventory = None
			self.inventories = {}
		
		def __str__(self):
			return str(self.name)
		
		def __call__(self, text):
			name = self.name
			if callable(name):
				name = name()
			if name is not None:
				name = _(name)
			
			db.show_text(name, text, self)
		
		
		# rpg-funcs:
		
		def make_rpg(self, directory, rpg_name, start_dress):
			self.directory = make_sure_dir(directory)
			self.rpg_name = rpg_name
			self.set_dress(start_dress)
		
		def add_over(self, images):
			self.overs.append(images)
		def remove_over(self, images):
			if images in self.overs:
				self.overs.remove(images)
		
		
		def get_zorder(self):
			return self.y + self.yoffset
		def get_draw_data(self):
			main = get_usual_location_object_data(self)
			res = [main]
			for images in self.overs:
				if callable(images):
					images = images(self)
				if type(images) not in (list, tuple):
					images = [images]
				for image in images:
					part = SimpleObject(main)
					part.image = image
					res.append(part)
			return res
		
		def main(self):
			if self.start_anim_time is not None:
				return get_location_image(self.animation.path, '', '', False)
			return get_location_image(self.directory, self.rpg_name, self.dress, False)
		
		
		def get_dress(self):
			return self.dress
		def set_dress(self, dress):
			if dress == self.dress:
				return
			
			self.dress = dress
			if dress not in self.inventories:
				size = inventory.dress_sizes[dress if dress in inventory.dress_sizes else 'default']
				self.inventories[dress] = [['', 0] for i in range(size)]
			
			old_inventory = self.inventory
			self.inventory = self.inventories[dress]
			if old_inventory and self.inventory:
				inventory.change(old_inventory, self.inventory, show_on_fail = self is me)
		
		def get_direction(self):
			return self.direction
		def set_direction(self, direction):
			if direction is not None:
				self.direction = direction % self.count_of_directions
		
		def rotate_to(self, obj):
			x, y = get_place_center(obj)
			dx, dy = x - self.x, y - self.y
			if abs(dx) > abs(dy):
				self.set_direction(to_left if dx < 0 else to_right)
			else:
				self.set_direction(to_forward if dy < 0 else to_back)
		
		def rotate_in_place(self, place = None):
			if not place:
				place = get_location_place(self)
			if type(place) is not str:
				place = place['name']
			
			if '_left'    in place: self.set_direction(to_left)
			if '_right'   in place: self.set_direction(to_right)
			if '_forward' in place: self.set_direction(to_forward)
			if '_back'    in place: self.set_direction(to_back)
		
		
		def get_pose(self):
			return self.pose
		def set_pose(self, pose):
			expected = ('sit', 'stay', 'walk', 'run')
			if pose in expected:
				if pose != 'sit' and self.sit_object:
					self.stand_up()
				if pose in ('walk', 'run'):
					self.fps   = self[pose + '_fps']
					self.speed = self[pose + '_speed']
				self.pose = pose
			else:
				self.pose = 'stay'
				msg = (
					'Unexpected pose <%s>\n'
					'Expected: %s'
				)
				params = (pose, ', '.join(expected))
				out_msg('Character.set_pose', msg % params)
		
		
		def set_frame(self, frame):
			self.frame = frame
		
		def update_crop(self):
			xsize, ysize = self.xsize, self.ysize
			
			frame = self.frame
			if self.start_anim_time is None:
				pose = self.get_pose()
				if pose == 'sit':
					frame = self.sit_frame
				elif pose == 'stay':
					frame = self.stay_frame
				else:
					frame %= self.count_of_moving_frames
				
				y = self.direction * ysize
			else:
				y = 0
			x = frame * xsize
			
			self.crop = (x, y, xsize, ysize)
		
		
		def sit_down(self, obj, place_index = -1):
			if not obj.sit_places:
				out_msg('Character.sit_down', 'obj.sit_places is empty')
				return False
			
			self.stand_up()
			
			if place_index < 0:
				min_dist = 1e6
				for i in range(len(obj.sit_places)):
					if obj.on[i]:
						continue
					
					place_x, place_y, to_side, can_use = obj.sit_places[i]
					if not can_use:
						continue
					
					dist = get_dist(obj.x + place_x, obj.y - obj.ysize + place_y, self.x, self.y)
					if dist < min_dist:
						min_dist = dist
						place_index = i
				
				if place_index < 0:
					return False
			else:
				if place_index >= len(obj.on):
					out_msg('Character.sit_down', 'place_index >= len(obj.sit_places)')
					return False
				if obj.on[place_index]:
					obj.on[place_index].stand_up()
			
			obj.on[place_index] = self
			place_x, place_y, to_side, can_use = obj.sit_places[place_index]
			
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
			
			for i in range(len(sit_object.on)):
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
				out_msg('Character.allow_exit', 'Location <%s> was not registered' % (location_name, ))
				return
			
			if place_name is not None:
				if place_name in location.places:
					self.allowed_exits.add((location_name, place_name))
				else:
					out_msg('Character.allow_exit', 'Place <%s> in location <%s> not found' % (place_name, location_name))
				return
			
			for place_name in location.places:
				self.allowed_exits.add((location_name, place_name))
		
		def disallow_exit(self, location_name, place_name = None):
			location = rpg_locations.get(location_name, None)
			if location is None:
				out_msg('Character.disallow_exit', 'Location <%s> was not registered' % (location_name, ))
				return
			if place_name is not None and place_name not in location.places:
				out_msg('Character.disallow_exit', 'Place <%s> in location <%s> not found' % (place_name, location_name))
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
			self.path_index = 0
			self.point_index = 0
			self.moving_ended = True
			self.set_pose('stay')
			self.rotate_after_moving = None
			if not place_names:
				return False
			
			if self.location is None:
				out_msg('Character.move_to_places', '%s not on location' % (self, ))
				return False
			
			if self is me:
				ignore_prev_rpg_control()
				set_rpg_control(False)
			
			path_update_locations()
			
			res = True # all paths found?
			from_location_name = self.location.name
			from_x, from_y = self.x, self.y
			
			banned = [exit for exit in location_banned_exits if exit not in self.allowed_exits]
			
			last_is_int = type(place_names[-1]) is int
			if last_is_int:
				if place_names[-1] < 0:
					place_names[-1] = len(place_names) + place_names[-1] - 2 # 2 = 1 (int) + 1 (path from last to need place)
			
			for i in range(len(place_names)):
				place_elem = place_names[i]
				
				if type(place_elem) is int:
					if i != len(place_names) - 1:
						out_msg('Character.move_to_places', 'Index for repeated place must be in the end')
						self.move_to_place(None)
						return False
					place_elem = place_names[place_elem]
				
				pdx = pdy = 0
				if type(place_elem) in (list, tuple):
					if len(place_elem) == 2:
						location_name, place_elem = place_elem
					elif len(place_elem) == 3:
						location_name, place_elem, offset = place_elem
						if type(offset) in (list, tuple) and len(offset) == 2 and type(offset[0]) is int and type(offset[1]) is int:
							pdx, pdy = offset
						else:
							out_msg('Character.move_to_places', 'Expected tuple or list with 2 ints in offset of place: <%s>' % (place_elem, ))
					else:
						out_msg('Character.move_to_places', 'Expected tuple or list with len 2 or 3, got <%s>' % (place_elem, ))
						return False
					if not location_name:
						location_name = from_location_name
				else:
					location_name = from_location_name
				
				location = rpg_locations.get(location_name, None)
				if not location:
					out_msg('Character.move_to_places', 'Location <%s> was not registered' % (location_name, ))
					return False
				
				if type(place_elem) is str:
					place = location.places.get(place_elem, None)
					if place is None:
						out_msg('Character.move_to_places', 'Place <%s> in location <%s> not found' % (place_elem, location_name))
						return False
				else:
					place = place_elem
				
				to_x, to_y = get_place_center(place)
				to_x += pdx
				to_y += pdy
				
				path = path_between_locations(from_location_name, int(from_x), int(from_y), location_name, float(to_x), float(to_y), banned, bool(brute_force))
				if not path:
					res = False
					if from_location_name == location_name:
						path = (to_x, to_y)
					else:
						path = (location_name, {'x': to_x, 'y': to_y}, to_x, to_y)
				self.paths.append(path)
				
				from_location_name = location_name
				from_x, from_y = to_x, to_y
			
			if last_is_int:
				self.paths.append(place_names[-1])
			
			self.moving_ended = False
			
			self.set_pose('run' if run else 'walk')
			
			self.prev_update_time = get_game_time()
			if wait_time >= 0:
				self.end_stop_time = self.prev_update_time + wait_time
			else:
				self.end_stop_time = None
			
			return res
		
		
		def remove_path_end(self, dist):
			if not self.paths:
				return
			
			path = self.paths[-1] = list(self.paths[-1])
			end_x = path[-2]
			end_y = path[-1]
			
			def get_coords(index):
				to_x = path[index * 2]
				to_y = path[index * 2 + 1]
				if type(to_x) is str:
					location_name, place = to_x, to_y
					if type(place) is str:
						place = rpg_locations[location_name].places[place]
					to_x, to_y = get_place_center(place)
					return to_x, to_y, True
				return to_x, to_y, False
			
			# points: start, before, end - dist, after, end
			
			index_before = len(path) // 2 - 1
			while index_before:
				x, y, is_new_loc = get_coords(index_before)
				
				if is_new_loc:
					path[index_before * 2 + 2:] = []
					return
				
				tmp_dist = get_dist(x, y, end_x, end_y)
				if tmp_dist > dist:
					break
				
				index_before -= 1
			
			if not index_before:
				path[2:] = []
				return
			
			index_after = index_before + 1
			
			before_x, before_y, _ = get_coords(index_before)
			before_dist = get_dist(before_x, before_y, end_x, end_y)
			after_x, after_y, _ = get_coords(index_after)
			after_dist = get_dist(after_x, after_y, end_x, end_y)
			
			while before_dist - after_dist > 1:
				mid_x = int(before_x + after_x) // 2
				mid_y = int(before_y + after_y) // 2
				mid_dist = get_dist(mid_x, mid_y, end_x, end_y)
				
				if mid_dist < dist:
					before_x, before_y = mid_x, mid_y
					before_dist = mid_dist
				else:
					after_x, after_y = mid_x, mid_y
					after_dist = mid_dist
			
			path[index_after * 2 + 2:] = []
			path[-2:] = int(after_x), int(after_y)
		
		
		def update_moving(self, dtime):
			if not self.paths:
				if not (self is me and get_rpg_control()):
					self.set_pose('stay')
				self.moving_ended = True
				return
			
			path = self.paths[self.path_index]
			location_name, place = self.location.name, None
			coords_before_exit = None
			
			last_direction = self.get_direction()
			while dtime > 0:
				to_x, to_y = path[self.point_index], path[self.point_index + 1]
				
				if type(to_x) is str:
					exit = get_location_place(self)
					if exit and exit.to_side is not None:
						last_direction = exit.to_side
					
					location_name, place = to_x, to_y
					if type(place) is str:
						place = rpg_locations[location_name].places[place]
					
					if not coords_before_exit:
						coords_before_exit = self.location, self.x, self.y
					self.x, self.y = get_place_center(place)
					
				else:
					dx, dy = to_x - self.x, to_y - self.y
					if dx or dy:
						# rotation
						adx, ady = abs(dx), abs(dy)
						if adx + ady > 3: # not very short step
							dx_direction = to_left if dx < 0 else to_right
							dy_direction = to_forward if dy < 0 else to_back
							kx = adx / max(ady, 0.01)
							ky = ady / max(adx, 0.01)
							main_directions = ([dx_direction] if kx > 0.5 else []) + ([dy_direction] if ky > 0.5 else [])
							if last_direction not in main_directions:
								last_direction = dx_direction if adx > ady else dy_direction
						
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
					
					self.path_index += 1
					if self.path_index == len(self.paths):
						self.paths = []
						self.set_pose('stay')
						self.moving_ended = True
						
						if self.rotate_after_moving is not None:
							last_direction = self.rotate_after_moving
							self.rotate_after_moving = None
						
						dtime = 0
						self.prev_update_time = get_game_time()
						break
					
					path = self.paths[self.path_index]
					if type(path) is int:
						self.path_index = path + 1 # 1 for skip path from start-place to first-place
						break
			
			self.prev_update_time -= dtime
			if coords_before_exit:
				x, y = self.x, self.y
				self.location, self.x, self.y = coords_before_exit
				show_character(self, {'x': x, 'y': y}, location_name)
			self.set_direction(last_direction)
		
		def move_to_end(self):
			if self.end_stop_time and self.end_stop_time > get_game_time():
				dtime = self.end_stop_time - self.prev_update_time
				self.end_stop_time = get_game_time()
			else:
				dtime = 1000
			
			self.update_moving(dtime)
		
		def ended_move_waiting(self):
			if self.end_stop_time:
				return self.end_stop_time <= get_game_time()
			if self.paths and type(self.paths[-1]) is int: # cycled path
				return True
			return self.moving_ended
		
		
		def start_animation(self, anim_name, repeat = 0, wait_time = -1):
			if anim_name not in self.animations:
				out_msg('Character.start_animation', 'Animation <%s> not found in character <%s>' % (anim_name, self))
				return
			
			if self.animation is None:
				self.orig_size = self.xsize, self.ysize
			
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
			if self.orig_size:
				self.xsize, self.ysize = self.orig_size
		
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
			old_value = self.auto_actions
			if not value and old_value and self.actions:
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
			if not self.rpg_name:
				return
			
			self.x, self.y = float(self.x), float(self.y)
			
			dtime = get_game_time() - self.prev_update_time
			self.prev_update_time = get_game_time()
			if has_screen('pause'):
				return
			
			if self.get_auto():
				actions = self.actions
				if actions:
					actions.update()
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
						if animation.xsize % animation.count_frames:
							msg = 'Animation <%s> of character <%s> has xsize (%s) that is not divisible by the count of frames (%s)'
							params = (animation.name, self, animation.xsize, animation.count_frames)
							out_msg('Character.update', msg % params)
						animation.xsize = math.ceil(animation.xsize / animation.count_frames)
					
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
	
	def get_name(who):
		g = globals()
		if who in g:
			return g[who].name
		out_msg('get_name', 'Character <%s> not found' % (who, ))
	
	def set_name(who, name):
		g = globals()
		if who in g:
			g[who].name = str(name)
		else:
			out_msg('set_name', 'Character <%s> not found' % (who, ))
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
	
	
	
	def show_character(character, place = None, location = None, auto_change_location = True, hiding = True):
		prev_character_location = character.location
		prev_character_pos = get_place_center(character)
		
		if location is None:
			if cur_location is None:
				out_msg('show_character', 'Current location is not defined, need to call set_location')
				return
			location = cur_location
		elif type(location) is str:
			if location not in rpg_locations:
				out_msg('show_character', 'Location <%s> not registered' % location)
				return
			location = rpg_locations[location]
		
		place = place or character
		if type(place) is str:
			place_name = place
			place = location.get_place(place)
			if not place:
				out_msg('show_character', 'Place <%s> not found in location <%s>' % (place_name, location.name))
				return
		
		if cur_location and cur_location.cam_object is character and auto_change_location:
			set_location(location.name, place, character)
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
				obj = create_hiding_object(character)
				if prev_character_location.cam_object is character:
					prev_character_location.cam_object = obj
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
			out_msg('hide_character', 'Character <%s, %s> not shown' % (character.real_name, character.unknow_name))
	
	
	tmp_character = Character('TMP')
	
	narrator = Character('')
	th = Character('', what_prefix = '~ ', what_suffix = ' ~')
	extend = Character(None)
