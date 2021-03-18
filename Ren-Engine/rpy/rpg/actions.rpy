init -1000 python:
	# rpg_action funcs
	
	
	def rpg_random_free_point(location_names):
		if not location_names:
			return None
		
		black_color = 255 # r, g, b, a = 0, 0, 0, 255
		
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
				obj_free = obj.free()
				if not obj_free: continue
				
				w, h = obj.xsize, obj.ysize
				obj_xanchor, obj_yanchor = get_absolute(obj.xanchor, w), get_absolute(obj.yanchor, h)
				obj_x, obj_y = int(obj.x + obj.xoffset - obj_xanchor), int(obj.y + obj.yoffset - obj_yanchor)
				
				dx, dy = x - obj_x, y - obj_y
				if dx < 0 or dy < 0 or dx >= w or dy >= h: continue
				
				if get_image_pixel(obj_free, dx, dy) == black_color:
					ok = False
					break
		
		if ok:
			return location_name, x, y
		return None
	
	
	def rpg_action_spawn(character, state):
		if state != 'end':
			return 'end'
		
		actions = character.get_actions()
		location_names = actions.interesting_places or rpg_locations.keys()
		
		for i in xrange(100):
			res = rpg_random_free_point(location_names)
			if res:
				location_name, x, y = res
				show_character(character, {'x': x, 'y': y}, location_name)
				return 'end'
		
		out_msg('rpg_action_spawn', 'Spawn point for <' + str(character) + '> not found')
		return 'end'
	
	
	def rpg_action_sit(character, state):
		actions = character.get_actions()
		sit_start_time = actions.sit_start_time or 0
		
		if state == 'start':
			if get_game_time() - sit_start_time < 10:
				return 'end'
			
			objs = get_near_sit_objects(character, 1e9)
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
			
			return 'end'
		
		if state == 'moving':
			if character.ended_move_waiting():
				return 'start'
			return 'moving'
		
		if state == 'sitting':
			if get_game_time() - sit_start_time > 3:
				return 'end'
			return 'sitting'
		
		if state == 'end':
			character.move_to_place(None)
			character.stand_up()
			return 'end'
	
	
	def rpg_action_other_place(character, state, location_names = None):
		actions = character.get_actions()
		
		if state == 'start':
			location_names = location_names or character.location.name
			if not isinstance(location_names, (tuple, list)):
				location_names = [location_names]
			
			for i in xrange(3):
				res = rpg_random_free_point(location_names)
				if not res: continue
				
				location_name, x, y = res
				path_found = character.move_to_place([location_name, {'x': x, 'y': y}])
				if path_found:
					return 'moving'
			
			return 'end'
		
		if state == 'moving':
			if character.ended_move_waiting():
				return 'end'
			return 'moving'
		
		if state == 'end':
			character.move_to_place(None)
			return 'end'
	
	
	def rpg_action_near_location(character, state):
		location_names = [exit.to_location_name for exit in character.location.exits]
		character.get_actions().cur_action = rpg_action_other_place
		return rpg_action_other_place(character, state, location_names)
	
	
	def rpg_action_random_location(character, state):
		location_names = rpg_locations.keys()
		character.get_actions().cur_action = rpg_action_other_place
		return rpg_action_other_place(character, state, location_names)
	
	
	def rpg_action_interesting_place(character, state):
		location_names = character.get_actions().interesting_places
		character.get_actions().cur_action = rpg_action_other_place
		return rpg_action_other_place(character, state, location_names)
	
	
	def rpg_action_look_around(character, state):
		actions = character.get_actions()
		
		if state == 'start':
			rotation = character.get_direction()
			actions.rotation = rotation
			actions.rotation_start_time = get_game_time()
			actions.rotation_time = random.random() * 1.1 + 0.4
			
			if random.random() < 0.25:
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
		
		if state == 'start':
			if not friend and get_game_time() - (actions.to_friend_end_time or 0) < 2:
				return 'end'
			
			if not friend:
				friends = []
				for friend in actions.friends or []:
					if not friend.get_auto(): continue
					if not friend.location: continue
					friend_actions = friend.get_actions()
					if friend_actions:
						if friend_actions.friend: continue
						if friend_actions.allow and 'to_friend' not in friend_actions.allow: continue
						if friend_actions.block and 'to_friend'     in friend_actions.block: continue
					friends.append(friend)
				if not friends:
					return 'end'
				friend = random.choice(friends)
			
			path_found = character.move_to_place([friend.location.name, friend])
			if not path_found:
				return 'end'
			
			friend_actions = friend.get_actions()
			if friend_actions and friend_actions.cur_action is not rpg_action_to_friend:
				friend_actions.stop()
			
			path_found = friend.move_to_place([character.location.name, character])
			if not path_found:
				friend.move_to_place(None)
				return 'end'
			
			actions.friend = friend
			actions.to_friend_start_time = get_game_time()
			
			if friend_actions:
				friend_actions.cur_action = rpg_action_to_friend
				friend_actions.state = 'moving'
				friend_actions.friend = character
				friend_actions.to_friend_start_time = get_game_time()
			
			return 'moving'
		
		if state == 'moving':
			same_location = character.location is friend.location
			dist = get_dist(character.x, character.y, friend.x, friend.y)
			if same_location and dist < character_radius * 3:
				character.move_to_place(None)
				character.rotate_to(friend.x - character.x, friend.y - character.y)
				
				friend.move_to_place(None)
				friend.rotate_to(character.x - friend.x, character.y - friend.y)
				
				actions.to_friend_end_time = get_game_time() + 3
				return 'conversation'
			
			if same_location:
				if dist < 150:
					update_time = 0.25
				elif dist < 400:
					update_time = 0.5
				else:
					update_time = 2.0
			else:
				update_time = 4.0
			if random.random() < 1.0 / (get_fps() * 10) or get_game_time() - actions.to_friend_start_time > update_time:
				return 'start'
			
			return 'moving'
		
		if state == 'conversation':
			if actions.to_friend_end_time < get_game_time():
				return 'end'
			return 'conversation'
		
		if state == 'end':
			character.move_to_place(None)
			actions.friend = None
			if friend and friend.get_actions():
				friend.get_actions().stop()
			return 'end'
	
	
	class RpgActions(Object):
		def __init__(self):
			Object.__init__(self)
			
			self.funcs = {}
			self.chances = {}
			
			self.allow = None
			self.block = None
		
		def copy(self, character):
			res = RpgActions()
			Object.__init__(res, self)
			res.character = character
			res.funcs = self.funcs.copy()
			res.chances = self.chances.copy()
			res.allow = self.allow and list(self.allow)
			res.block = self.block and list(self.block)
			return res
		
		def set_action(self, name, func, chance_min, chance_max = None):
			self.funcs[name] = func
			self.chances[name] = chance_min, chance_max
		
		def get_names(self):
			return self.funcs.keys()
		
		def update(self):
			if not self.stopped():
				self.state = self.exec_action()
				if self.state == 'end':
					self.stop()
			
			if self.stopped():
				self.random()
		
		def start(self, action_name):
			if self.allow and action_name not in self.allow: return
			if self.block and action_name     in self.block: return
			
			self.stop()
			self.cur_action = self.funcs[action_name]
			self.state = 'start'
		
		def stop(self):
			if self.cur_action:
				self.cur_action(self.character, 'end')
				self.cur_action = None
		def stopped(self):
			return self.cur_action is None
		
		def exec_action(self):
			return self.cur_action(self.character, self.state)
		
		def random(self):
			scores = 0
			chances = []
			for action, (chance_min, chance_max) in self.chances.iteritems():
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
	
	
	def get_std_rpg_actions():
		actions = (
			('spawn',              0),
			('sit',               30),
			('other_place',       10),
			('near_location',     10),
			('random_location',   10),
			('interesting_place', 20),
			('look_around',       20),
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
	
