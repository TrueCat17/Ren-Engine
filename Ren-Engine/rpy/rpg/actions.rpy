init -1000 python:
	# rpg_action funcs
	
	
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
		
		out_msg('rpg_action_spawn', 'Spawn point for <%s> not found' % character)
		return 'end'
	
	
	def rpg_action_sit(character, state):
		actions = character.get_actions()
		
		if state == 'start':
			if get_game_time() - (actions.sit_start_time or -10) < 10:
				return 'end'
			
			objs = get_near_sit_objects(character, 1e9)
			st = time.time()
			for obj, point in objs:
				if point == (character.x, character.y):
					if not character.sit_down(obj):
						if random.random() < 0.2:
							return 'end'
						continue
					actions.sit_start_time = get_game_time()
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
			if get_game_time() - actions.sit_start_time > 5:
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
			if not isinstance(location_names, (tuple, list)):
				location_names = [location_names]
			
			st = time.time()
			for i in range(3):
				res = rpg_random_free_point(location_names)
				if not res: continue
				
				location_name, x, y = res
				path_found = character.move_to_place([location_name, {'x': x, 'y': y}], run=run)
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
			actions.rotation_time = random.random() * 1.1 + 0.4
			
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
		
		if home_is_fake_location:
			if not isinstance(home, (tuple, list)) or len(home) != 2:
				out_msg('rpg_action_home', 'data <home> is not [location, place]')
				return 'end'
			
			location_name, place = home
			if location_name not in rpg_locations:
				out_msg('rpg_action_home', 'Location <' + str(location_name) + '> not registered')
				return 'end'
			
			location = rpg_locations[location_name]
			if type(place) is str:
				place_name = place
				place = location.get_place(place_name)
				if place is None:
					out_msg('rpg_action_home', 'Place ' + place + '> in location <' + location_name + '> not found')
					return 'end'
		
		
		if state == 'start':
			if 0 < get_game_time() - (actions.home_end_time or 0) < 15:
				return 'end'
			
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
		
		no_start_friend = friend is None
		
		if state == 'start':
			if friend:
				if not friend.location:
					return 'end'
			else:
				if get_game_time() - (actions.to_friend_end_time or -2) < 2:
					return 'end'
			
			if not friend:
				friends = []
				for friend in actions.friends or []:
					if not friend.get_auto(): continue
					if not friend.location: continue
					friend_actions = friend.get_actions()
					if friend_actions:
						if not friend_actions.interruptable: continue
						if friend_actions.allow and 'to_friend' not in friend_actions.allow: continue
						if friend_actions.block and 'to_friend'     in friend_actions.block: continue
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
				
				actions.to_friend_end_time = get_game_time() + 3
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
	
	def rpg_action_follow(character, state, to):
		actions = character.get_actions()
		
		same_location = character.location is to.location
		dist = get_dist(character.x, character.y, to.x, to.y)
		min_dist = 50 # stop
		max_dist = 150 # run
		
		if state == 'start':
			if same_location and dist <= min_dist:
				character.move_to_place(None)
				character.rotate_to(to)
				return 'start'
			if actions.follow_start_time and get_game_time() - actions.follow_start_time < 0.25:
				return 'start'
			actions.follow_start_time = get_game_time()
			
			run = dist > max_dist
			path_found = character.move_to_place([to.location.name, to], run=run)
			if path_found:
				return 'moving'
			character.move_to_place(None)
			return 'start'
		
		if state == 'moving':
			if not same_location or dist <= min_dist:
				return 'start'
			
			if same_location:
				if dist < 100:
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
			
			return 'moving'
		
		if state in ('rewind_to_min', 'rewind_to_max'):
			path_found = character.move_to_place([to.location.name, to])
			if not path_found:
				return 'start'
			need_dist = min_dist if 'min' in state else max_dist
			while character.location is not to.location or get_dist(character.x, character.y, to.x, to.y) > need_dist:
				character.update_moving(0.05)
			return 'start'
		
		if state == 'end':
			character.move_to_place(None)
			actions.follow_start_time = None
			return 'end'
	
	
	
	IGNORE_STATE = 'ignore state'
	
	class RpgActions(Object):
		def __init__(self):
			Object.__init__(self)
			
			self.funcs = {}
			self.chances = {}
			
			self.allow = []
			self.block = []
			
			self.queue = []
			
			self.interruptable = True
			self.default_interruptable = True
		
		def copy(self, character):
			res = RpgActions()
			Object.__init__(res, self)
			res.character = character
			for prop in ('funcs', 'chances', 'allow', 'block', 'queue'):
				res[prop] = self[prop].copy()
			return res
		
		def set_action(self, name, func, chance_min, chance_max = None):
			self.funcs[name] = func
			self.chances[name] = chance_min, chance_max
		
		def get_names(self):
			return list(self.funcs.keys())
		
		def update(self, state = None):
			if self.last_update == get_game_time():
				return
			self.last_update = get_game_time()
			
			if not self.stopped():
				if state is not None:
					self.state = state
				old_action, old_state = self.cur_action, self.state
				
				new_state = self.exec_action()
				if new_state == IGNORE_STATE:
					return
				if type(new_state) is not str:
					params = (old_action, self.character, new_state, old_state)
					msg = 'Action <%s> of character <%s> returned non-str new state <%s> (old state = <%s>)' % params
					out_msg('RpgActions.update', msg)
					new_state = 'end'
				
				self.state = new_state
				if self.state == 'end':
					self.stop()
			
			if self.stopped() and self.character.get_auto():
				if not self.next():
					self.random()
		
		def set(self, action_name, *args, **kwargs):
			if self.blocked(action_name): return
			
			self.interruptable = self.default_interruptable
			if type(action_name) is str:
				self.cur_action = self.funcs[action_name]
			else:
				self.cur_action = action_name
			
			self.save_params(args, kwargs)
			
			self.character.set_auto(True)
			
			self.last_update = None
			self.update()
		
		def start(self, action_name, *args, **kwargs):
			if self.blocked(action_name): return
			
			self.stop()
			self.set(action_name, *args, **kwargs)
		
		def stop(self):
			if self.cur_action:
				self.state = 'end'
				new_state = self.exec_action()
				if new_state == IGNORE_STATE:
					return
				self.cur_action = None
			self.interruptable = True
		
		def stopped(self):
			return self.cur_action is None
		
		def exec_action(self):
			return self.cur_action(self.character, self.state, *self.cur_args, **self.cur_kwargs)
		
		def random(self):
			scores = 0
			chances = []
			for action, (chance_min, chance_max) in self.chances.items():
				if self.allow and action not in self.allow: continue
				if self.block and action     in self.block: continue
				
				chance = chance_min if chance_max is None else random.randint(chance_min, chance_max)
				
				scores += chance
				chances.append((action, scores))
			
			r = random.random() * scores
			for action, score in chances:
				if r < score:
					self.start(action)
					break
		
		def blocked(self, action):
			return (
				(self.allow and action not in self.allow)
				or
				(self.block and action     in self.block)
			)
		
		def save_params(self, args, kwargs):
			self.state = kwargs.get('state', 'start')
			if 'state' in kwargs:
				del kwargs['state']
			self.cur_args, self.cur_kwargs = args, kwargs
		
		def add(self, action_name, *args, **kwargs):
			self.queue.append([action_name, args, kwargs])
		
		def next(self):
			while True:
				if not self.queue:
					return False
				
				action_name, args, kwargs = self.queue[0]
				self.queue.pop(0)
				if not self.blocked(action_name):
					self.start(action_name, *args, **kwargs)
					return True
	
	
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
		
		return res
	
