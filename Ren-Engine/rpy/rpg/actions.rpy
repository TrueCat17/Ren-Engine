init -1000 python:
	# rpg_action funcs
	
	
	rpg_action_spent_time = defaultdict(int)
	signals.add('enter_frame', rpg_action_spent_time.clear)
	
	def rpg_action_spent_time_exec(func, cur_action):
		st = time.time()
		res = func()
		dtime = time.time() - st
		
		rpg_action_spent_time[cur_action] += dtime
		rpg_action_spent_time[None] += dtime
		return res
	
	
	def rpg_random_free_point(location_names):
		if not location_names:
			return None
		
		black_color = 255 # r, g, b, a = 0, 0, 0, 255
		cr = Character.radius
		
		location_name = random.choice(location_names)
		location = rpg_locations[location_name]
		free = location.free()
		
		i = 0
		ok = False
		while i < 10 and not ok:
			i += 1
			
			x = random.randint(0, location.xsize - 1)
			y = random.randint(0, location.ysize - 1)
			if not free:
				ok = True
				break
			
			if get_image_pixel(free, x, y) != black_color:
				continue
			
			ok = True
			for obj in location.objects:
				if not isinstance(obj, RpgLocationObject): continue
				if not obj.free(): continue
				
				w, h = obj.xsize, obj.ysize
				obj_xanchor, obj_yanchor = get_absolute(obj.xanchor, w), get_absolute(obj.yanchor, h)
				obj_x, obj_y = int(obj.x + obj.xoffset - obj_xanchor), int(obj.y + obj.yoffset - obj_yanchor)
				
				dx, dy = x - obj_x, y - obj_y
				if dx >= -cr and dy >= -cr and dx < w + cr and dy < h + cr:
					ok = False
					break
		
		if ok:
			return location_name, x, y
		return None
	
	
	def rpg_action_spawn(character, state):
		if state != 'end':
			return 'end'
		
		actions = character.get_actions()
		location_names = actions.interesting_places or list(rpg_locations.keys())
		
		for i in range(100):
			res = rpg_random_free_point(location_names)
			if res:
				location_name, x, y = res
				show_character(character, {'x': x, 'y': y}, location_name)
				return 'end'
		
		out_msg('rpg_action_spawn', 'Spawn point for <%s> not found', character)
		return 'end'
	
	
	def rpg_action_sit(character, state):
		actions = character.get_actions()
		
		if state == 'start':
			objs = get_near_sit_objects(character, 1e9)
			st = time.time()
			for obj, point in objs:
				if point == (character.x, character.y):
					if not character.sit_down(obj):
						if random.random() < 0.2:
							return 'end'
						continue
					actions.sit_end_time = get_game_time() + actions.get_random_param('sit')
					return 'sitting'
				
				to_x, to_y = point
				path_found = character.move_to_place({'x': to_x, 'y': to_y})
				if path_found:
					return 'moving'
				if time.time() - st > 0.015:
					break
			
			return 'end'
		
		if state == 'moving':
			if character.ended_move_waiting():
				return 'start'
			return 'moving'
		
		if state == 'sitting':
			if get_game_time() > actions.sit_end_time:
				return 'end'
			return 'sitting'
		
		if state == 'end':
			character.move_to_place(None)
			character.stand_up()
			return 'end'
	
	
	def rpg_action_other_place(character, state, location_names = None, run = False):
		actions = character.get_actions()
		
		if state == 'start':
			location_names = location_names or character.location.name
			if type(location_names) not in (tuple, list):
				location_names = [location_names]
			
			st = time.time()
			for i in range(3):
				res = rpg_random_free_point(location_names)
				if not res: continue
				
				location_name, x, y = res
				path_found = character.move_to_place([location_name, {'x': x, 'y': y}], run = run)
				if path_found:
					return 'moving'
				
				if time.time() - st > 0.015: break
			
			return 'end'
		
		if state == 'moving':
			if character.ended_move_waiting():
				return 'end'
			return 'moving'
		
		if state == 'end':
			character.move_to_place(None)
			return 'end'
	
	
	def rpg_action_near_location(character, state):
		location_names = [place.to_location_name for place in character.location.places.values() if place.to_location_name]
		character.get_actions().set('other_place', location_names, state = state)
		return IGNORE_STATE
	
	
	def rpg_action_random_location(character, state):
		location_names = list(rpg_locations.keys())
		character.get_actions().set('other_place', location_names, state = state)
		return IGNORE_STATE
	
	
	def rpg_action_interesting_place(character, state):
		location_names = character.get_actions().interesting_places
		character.get_actions().set('other_place', location_names, state = state)
		return IGNORE_STATE
	
	
	def rpg_action_look_around(character, state, turn_at_the_end = None):
		actions = character.get_actions()
		
		if state == 'start':
			rotation = character.get_direction()
			actions.rotation = rotation
			actions.rotation_start_time = get_game_time()
			actions.rotation_time = actions.get_random_param('look_around')
			
			if turn_at_the_end is None:
				turn_at_the_end = random.random() < 0.25
			if turn_at_the_end:
				forward = [to_left, to_back, to_right, to_forward]
				back    = [to_right, to_forward, to_left, to_back]
				rotation = back[forward.index(rotation)]
			actions.rotation_end = rotation
			
			return 'rotation'
		
		if state == 'rotation':
			rotations = {
				to_left:    (to_forward, to_back),
				to_back:    (to_left, to_right),
				to_right:   (to_back, to_forward),
				to_forward: (to_right, to_left),
			}
			
			right, left = rotations[actions.rotation]
			
			dtime = get_game_time() - actions.rotation_start_time
			rotation_time = actions.rotation_time
			
			if dtime < rotation_time:
				character.set_direction(right)
			elif dtime < rotation_time * 1.2:
				character.set_direction(actions.rotation)
			elif dtime < rotation_time * 2.2:
				character.set_direction(left)
			elif dtime < rotation_time * 3.2:
				character.set_direction(actions.rotation_end)
			elif dtime > rotation_time * 5.2:
				return 'end'
			return 'rotation'
		
		if state == 'end':
			character.set_direction(actions.rotation_end)
			return 'end'
	
	
	def rpg_action_home(character, state):
		actions = character.get_actions()
		home = actions.home
		if not home:
			return 'end'
		
		home_is_fake_location = type(home) is not str
		
		if home_is_fake_location and state in ('start', 'end'):
			if type(home) not in (tuple, list) or len(home) != 2:
				out_msg('rpg_action_home', 'Data <home> of character <%s> is not [location, place] (got %s)', character, home)
				return 'end'
			
			location_name, place = home
			if location_name not in rpg_locations:
				out_msg('rpg_action_home', 'Location <%s> was not registered', location_name)
				return 'end'
			
			location = rpg_locations[location_name]
			if type(place) is str:
				place_name = place
				place = location.get_place(place_name)
				if place is None:
					out_msg('rpg_action_home', 'Place <%s> in location <%s> not found', place_name, location_name)
					return 'end'
		
		
		if state == 'start':
			if not home_is_fake_location:
				if character.location.name == home:
					actions.home_end_time = get_game_time() + 2
					actions.old_rotation = character.get_direction()
					return 'home_walking'
				return rpg_action_other_place(character, 'start', home)
			
			
			if character.location is location and (character.x, character.y) == get_place_center(place):
				actions.home_end_time = get_game_time() + 5
				actions.old_rotation = character.get_direction()
				hide_character(character)
				return 'home_walking'
			
			path_found = character.move_to_place([location_name, place])
			if path_found:
				return 'moving'
			return 'end'
		
		if state == 'moving':
			if character.ended_move_waiting():
				return 'start'
			return 'moving'
		
		if state == 'home_walking':
			if not character.ended_move_waiting():
				return 'home_walking'
			
			if get_game_time() < actions.home_end_time:
				if character.location:
					rpg_action_other_place(character, 'start')
				return 'home_walking'
			
			return 'end'
		
		if state == 'end':
			opposite_sides = {
				to_left:    to_right,
				to_right:   to_left,
				to_back:    to_forward,
				to_forward: to_back,
				None:       None,
			}
			character.set_direction(opposite_sides[actions.old_rotation])
			
			if not character.location:
				show_character(character, place, location_name)
			
			character.move_to_place(None)
			return 'end'
	
	def rpg_action_to_friend(character, state):
		actions = character.get_actions()
		friend = actions.friend
		friend_actions = friend and friend.get_actions()
		if friend and state != 'end':
			# check for changed friend action
			if not friend_actions: return 'end'
			if friend_actions.cur_action is not rpg_action_to_friend: return 'end'
			if friend_actions.friend is not character: return 'end'
		
		if state == 'start':
			if friend and not friend.location:
				return 'end'
			
			no_start_friend = friend is None
			if no_start_friend:
				friends = []
				for friend in actions.friends or []:
					if not friend.get_auto(): continue
					if not friend.location: continue
					friend_actions = friend.get_actions()
					if friend_actions:
						if not friend_actions.interruptable: continue
						if friend_actions.blocked('to_friend'): continue
					friends.append(friend)
				if not friends:
					return 'end'
				friend = random.choice(friends)
				friend_actions = friend.get_actions()
			
			path_found = character.move_to_place([friend.location.name, friend])
			if not path_found:
				return 'end'
			
			actions.friend = friend
			actions.to_friend_start_time = get_game_time()
			
			if no_start_friend:
				if friend_actions and friend_actions.cur_action is not rpg_action_to_friend:
					friend_actions.stop()
				
				path_found = friend.move_to_place([character.location.name, character])
				if not path_found:
					friend.move_to_place(None)
					return 'end'
				
				if friend_actions:
					friend_actions.friend = character
					friend_actions.to_friend_start_time = get_game_time()
					friend_actions.set(rpg_action_to_friend, state = 'moving')
			
			return 'moving'
		
		if state == 'moving':
			same_location = character.location is friend.location
			dist = get_dist(character.x, character.y, friend.x, friend.y)
			if same_location and dist < Character.radius * 3:
				character.move_to_place(None)
				character.rotate_to(friend)
				
				friend.move_to_place(None)
				friend.rotate_to(character)
				
				actions.to_friend_end_time = get_game_time() + actions.get_random_param('to_friend')
				return 'conversation'
			
			if same_location:
				if dist < 100:
					update_time = 0.25
				elif dist < 200:
					update_time = 0.5
				elif dist < 400:
					update_time = 1.0
				else:
					update_time = 3.0
			else:
				update_time = 10.0
			
			if get_game_time() - actions.to_friend_start_time > update_time:
				return 'start'
			
			return 'moving'
		
		if state == 'conversation':
			if actions.to_friend_end_time < get_game_time():
				return 'end'
			return 'conversation'
		
		if state == 'end':
			character.move_to_place(None)
			actions.friend = None
			if friend_actions:
				friend_actions.stop()
			return 'end'
	
	# stop on min_dist, run on max_dist
	def rpg_action_follow(character, state, to, min_dist = 50, max_dist = 150, keep_dist = None):
		actions = character.get_actions()
		
		same_location = character.location is to.location
		to_sit_object = to.sit_object
		sit_object = None
		to_x, to_y = to.x, to.y
		if to_sit_object:
			if to_sit_object is character.sit_object:
				sit_object = to_sit_object
				to_x, to_y = character.x, character.y
			else:
				objs = get_near_sit_objects(to, max_dist)
				if objs:
					sit_object, (to_x, to_y) = objs[0]
				else:
					to_x, to_y = to.old_x, to.old_y
		dist = get_dist(character.x, character.y, to_x, to_y)
		
		if keep_dist is None:
			keep_dist = min_dist * 0.7
		
		if sit_object:
			min_dist = 1
		else:
			if dist < keep_dist:
				dx, dy = {
					to_left:    (-1, 0),
					to_right:   (+1, 0),
					to_forward: (0, -1),
					to_back:    (0, +1),
				}[to.get_direction()]
				to_x = in_bounds(round(to_x + dx * min_dist), 0, to.location.xsize - 1)
				to_y = in_bounds(round(to_y + dy * min_dist), 0, to.location.ysize - 1)
				
				free = to.location.free()
				black_color = 255 # RGBA
				if not free or get_image_pixel(free, to_x, to_y) == black_color:
					dist = get_dist(character.x, character.y, to_x, to_y)
					min_dist = 1
				else:
					to_x, to_y = character.x, character.y
		
		def fix_speed(need = False):
			if need or get_game_time() - actions.last_speed_update > 1:
				actions.last_speed_update = get_game_time()
				run = not same_location or dist > max_dist
				pose = 'run' if run else 'walk'
				character.set_pose(pose)
				character.speed = character[pose + '_speed']
				character.fps   = character[pose + '_fps']
		
		
		if state == 'start':
			actions.interruptable = False
			
			if sit_object and not character.sit_object and dist < 1:
				character.sit_down(sit_object)
			if not sit_object and character.sit_object:
				character.stand_up()
			if sit_object and character.sit_object is sit_object:
				return 'start'
			
			if same_location and dist <= min_dist:
				character.move_to_place(None)
				character.rotate_to(to)
				return 'start'
			if actions.follow_start_time and get_game_time() - actions.follow_start_time < 0.25:
				return 'start'
			actions.follow_start_time = get_game_time()
			
			path_found = character.move_to_place([to.location.name, {'x': to_x, 'y': to_y}])
			if path_found:
				fix_speed(True)
				return 'moving'
			
			character.move_to_place(None)
			return 'start'
		
		if state == 'moving':
			if character.ended_move_waiting():
				return 'start'
			
			if same_location:
				if dist < max(100, keep_dist, min_dist):
					update_time = 0.25
				elif dist < 200:
					update_time = 0.5
				elif dist < 400:
					update_time = 1.0
				else:
					update_time = 2.0
			else:
				update_time = 2.0
			
			if get_game_time() - actions.follow_start_time > update_time:
				return 'start'
			
			fix_speed()
			return 'moving'
		
		if state in ('rewind_to_min', 'rewind_to_max'):
			path_found = character.move_to_place([to.location.name, {'x': to_x, 'y': to_y}])
			if not path_found:
				return 'start'
			need_dist = min_dist if state == 'rewind_to_min' else max_dist
			if need_dist < keep_dist:
				need_dist = keep_dist
			while character.location is not to.location or get_dist(character.x, character.y, to_x, to_y) > need_dist:
				character.update_moving(0.05)
			return 'start'
		
		if state == 'end':
			character.move_to_place(None)
			actions.follow_start_time = None
			return 'end'
	
	
	
	IGNORE_STATE = 'ignore state'
	
	class RpgActions(Object):
		# for any frame, in seconds
		MAX_ACTION_FRAME_TIME = 0.020
		MAX_ALL_ACTIONS_FRAME_TIME = 0.100
		
		def __init__(self):
			Object.__init__(self)
			
			self.funcs = {}
			self.chances = {}
			
			self.last_calls = {}
			self.cooldowns = {}
			
			self.random_param_bounds = {}
			
			self.allow = []
			self.block = []
			
			self.queue = []
			
			self.interruptable = True
			self.default_interruptable = True
		
		def copy(self, character):
			res = RpgActions()
			Object.__init__(res, self)
			res.character = character
			for prop in ('funcs', 'chances', 'last_calls', 'cooldowns', 'random_param_bounds', 'allow', 'block', 'queue'):
				res[prop] = self[prop].copy()
			return res
		
		def set_action(self, name, func, chance_min, chance_max = None):
			self.funcs[name] = func
			self.chances[name] = chance_min, chance_max
		
		def get_names(self):
			return list(self.funcs.keys())
		
		def update(self, state = None):
			if self.last_update == get_game_time() and state is None:
				return
			self.last_update = get_game_time()
			
			if not self.stopped():
				if state is not None:
					self.state = state
				
				if rpg_action_spent_time[self.cur_action] > self.MAX_ACTION_FRAME_TIME or rpg_action_spent_time[None] > self.MAX_ALL_ACTIONS_FRAME_TIME:
					if self.state != 'end' and state is None:
						return
				
				if self.state != 'end':
					old_action, old_state = self.cur_action, self.state
					
					new_state = rpg_action_spent_time_exec(self.exec_action, self.cur_action)
					if new_state == IGNORE_STATE:
						return
					if type(new_state) is not str:
						msg = 'Action <%s> of character <%s> returned non-str new state <%s> (old state = <%s>)'
						params = (old_action, self.character, new_state, old_state)
						out_msg('RpgActions.update', msg, *params)
						new_state = 'end'
					self.state = new_state
				
				if self.state == 'end':
					self.stop(directly = False)
			
			if self.stopped() and self.character.get_auto():
				if not self.next():
					self.random()
		
		def set(self, action, *args, **kwargs):
			if self.blocked(action): return
			
			self.interruptable = self.default_interruptable
			if type(action) is str:
				self.cur_action = self.funcs[action]
			else:
				self.cur_action = action
			
			self.character.set_auto(True)
			
			state = kwargs.pop('state', None)
			self.state = 'start'
			self.cur_args = args
			self.cur_kwargs = kwargs
			
			self.last_update = None
			self.update(state)
		
		def start(self, action, *args, **kwargs):
			if self.blocked(action): return
			
			self.stop()
			self.set(action, *args, **kwargs)
		
		def stop(self, directly = True):
			if self.cur_action:
				self.last_calls[self.cur_action] = get_game_time()
				self.state = 'end'
				new_state = rpg_action_spent_time_exec(self.exec_action, self.cur_action)
				
				# if self.exec_action() on 'end' state calls self.set()
				i = 0
				while directly and new_state != 'end' and i < 10:
					i += 1
					self.state = 'end'
					new_state = rpg_action_spent_time_exec(self.exec_action, self.cur_action)
				
				if new_state == IGNORE_STATE and not directly:
					return
				self.cur_action = None
			self.interruptable = True
		
		def stopped(self):
			return self.cur_action is None
		
		def exec_action(self):
			return self.cur_action(self.character, self.state, *self.cur_args, **self.cur_kwargs)
		
		def random(self):
			now = get_game_time()
			
			scores = 0
			chances = []
			for action, (chance_min, chance_max) in self.chances.items():
				if self.blocked(action):
					continue
				
				func = self.funcs[action]
				last_call = self.last_calls.get(func, -1e9)
				cooldown = self.cooldowns.get(func, 0)
				if now - last_call < cooldown:
					continue
				
				chance = chance_min if chance_max is None else random.randint(chance_min, chance_max)
				scores += chance
				chances.append((action, scores))
			
			r = random.random() * scores
			for action, score in chances:
				if r < score:
					self.start(action)
					break
		
		def blocked(self, action):
			func = self.funcs.get(action)
			if self.allow and action not in self.allow and func not in self.allow:
				return True
			return action in self.block or func in self.block
		
		def add(self, action, *args, **kwargs):
			self.queue.append([action, args, kwargs])
		
		def next(self):
			while True:
				if not self.queue:
					return False
				
				action, args, kwargs = self.queue[0]
				self.queue.pop(0)
				if not self.blocked(action):
					self.start(action, *args, **kwargs)
					return True
		
		
		def set_cooldown(self, action, time):
			if type(action) is str:
				action = self.funcs[action]
			self.cooldowns[action] = time
		
		
		def set_random_param(self, name, min_value, max_value):
			self.random_param_bounds[name] = (min_value, max_value)
		
		def get_random_param(self, name):
			min_value, max_value = self.random_param_bounds[name]
			return random.uniform(min_value, max_value)
	
	
	def get_std_rpg_actions():
		actions = (
			('spawn',              0),
			('follow',             0),
			('sit',               70),
			('other_place',       40),
			('near_location',     20),
			('random_location',   10),
			('interesting_place', 20),
			('look_around',       40),
			('home',              10, 50),
			('to_friend',         20, 40),
		)
		g = globals()
		
		res = RpgActions()
		for action in actions:
			name = action[0]
			chance_min = action[1]
			chance_max = None if len(action) == 2 else action[2]
			
			res.set_action(name, g['rpg_action_' + name], chance_min, chance_max)
		
		res.set_cooldown('sit', 5)
		res.set_cooldown('home', 10)
		res.set_cooldown('to_friend', 2)
		
		res.set_random_param('sit', 5, 50)
		res.set_random_param('look_around', 0.4, 1.5)
		res.set_random_param('to_friend', 2, 5)
		
		return res
	
