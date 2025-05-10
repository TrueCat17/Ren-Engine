init -9000 python:
	
	def linear(t):
		return t
	def ease(t):
		return 0.5 - math.cos(math.pi * t) / 2.0
	def easein(t):
		return math.cos((1.0 - t) * math.pi / 2.0)
	def easeout(t):
		return 1.0 - math.cos(t * math.pi / 2.0)
	
	
	atl_props = {
		'simple': ('alpha', 'rotate'),
		'xy': ('pos', 'anchor', 'align', 'size', 'size_min', 'size_max', 'zoom'),
		'crop': ['xcrop', 'ycrop', 'xsizecrop', 'ysizecrop'],
	}
	
	# alpha -> [alpha]
	# xpos  -> [xpos]
	#  pos  -> [xpos, ypos]
	def get_atl_props(prop):
		if prop in atl_props['simple'] or (prop[0] in 'xy' and prop[1:] in atl_props['xy']):
			return [prop]
		if prop in atl_props['xy']:
			return ['x' + prop, 'y' + prop]
		if prop in atl_props['crop']:
			return [prop]
		if prop == 'crop':
			return atl_props['crop']
		return None
	
	
	def get_prop_names(prop):
		props = get_atl_props(prop)
		
		if len(props) == 1:
			if prop == 'xalign':
				return ('xpos', 'xanchor')
			if prop == 'yalign':
				return ('ypos', 'yanchor')
			return prop
		
		return props
	
	
	class SpriteAnimation(SimpleObject):
		def __init__(self, actions, data = None):
			SimpleObject.__init__(self)
			
			self.ended = False
			self.repeated = {}
			
			self.action_num = 0
			self.action_filename = None
			self.action_numline = -1
			self.actions = actions
			self.last_command = ''
			
			self.block = None
			self.parallels = []
			
			self.data = data
			
			self.end_pause_time = 0
			
			self.start_changing_time = None
			self.end_changing_time = None
		
		
		def copy(self, data):
			return SpriteAnimation(self.actions, data = data)
		
		
		def out_msg(self, func, msg):
			out_msg(func, 'Place of transform command: %s:%s\n\n%s' % (self.action_filename, self.action_numline, msg))
		
		
		def update(self):
			now = get_game_time()
			if now < self.end_pause_time:
				return
			
			if self.start_changing_time is not None:
				self.update_changing()
				if now <= self.end_changing_time:
					return
				else:
					self.start_changing_time = None
			
			
			if self.block:
				self.block.update()
				if self.block.ended:
					self.block = None
				
				return
			
			
			if self.parallels:
				for parallel in self.parallels:
					parallel.update()
				
				for parallel in self.parallels:
					if not parallel.ended:
						break
				else:
					self.parallels = []
				
				return
			
			
			
			if self.action_num >= len(self.actions):
				self.ended = True
				return
			
			
			
			while self.action_num < len(self.actions):
				action = ''
				while not action and self.action_num < len(self.actions):
					action, self.action_filename, self.action_numline = self.actions[self.action_num]
					self.action_num += 1
				
				if not action:
					return
				
				if type(action) is str:
					if self.parallels:
						self.action_num -= 1
						return
					
					args = get_args(action)
					if len(args) == 0:
						continue
				
					command = args[0]
					self.last_command = command
					
					if command == 'pass':
						continue
					
					if command == 'pause':
						if len(args) == 2:
							try:
								pause_time = eval(args[1])
							except Exception as e:
								self.out_msg('SpriteAnimation.update', 'On Eval: %s, exception: %s' % (args[1], e))
							else:
								self.end_pause_time = get_game_time() + pause_time
						else:
							self.out_msg('SpriteAnimation.update', '<pause> expected 1 argument: time\n' + action)
						return
					
					if command == 'repeat':
						if len(args) > 2:
							self.out_msg('SpriteAnimation.update', '<repeat> expected 1 optional argument: count of repeats\n' + action)
						
						count = int(10e9 if len(args) == 1 else args[1])
						num = self.action_num - 1
						repeated = self.repeated.get(num, 0)
						
						if repeated < count:
							self.action_num = 0
							self.repeated[num] = self.repeated.get(num, 0) + 1
							
							for key in self.repeated:
								if key < num:
									self.repeated[key] = 0
						continue
					
					is_prop = get_atl_props(command) is not None
					if is_prop:
						name = args[0]
						expr = ' '.join(args[1:])
						try:
							value = eval(expr)
						except Exception as e:
							self.out_msg('SpriteAnimation.update', 'On Eval: %s, exception: %s' % (expr, e))
						else:
							self.set_prop(name, value)
						continue
					
					try:
						evaled = eval(action)
						if isinstance(evaled, number_types):
							self.end_pause_time = get_game_time() + evaled
							return
						
						if isinstance(evaled, str):
							if image_was_registered(evaled):
								image_actions = get_image(evaled)
								self.action_num -= 1
								self.actions = self.actions[:self.action_num] + image_actions + self.actions[self.action_num+1:]
							else:
								self.data.contains = []
								self.data.image = evaled
						else:
							self.out_msg('SpriteAnimation.update', 'Unknown command:\n' + action)
					except:
						try:
							evaled = eval(command)
							if callable(evaled):
								if len(args) % 2:
									desc = command + ' expected odd count of arguments: time, [param value]+\n' + action
									self.out_msg('SpriteAnimation.update', desc)
								else:
									self.change_func = evaled
									self.start_changing_time = get_game_time()
									self.end_changing_time = get_game_time() + eval(args[1])
									self.save_change_params(args[2:])
								return
							self.out_msg('SpriteAnimation.update', 'Unknown command:\n' + action)
						except:
							self.out_msg('SpriteAnimation.update', 'Exception on:\n' + action)
					continue
				
				
				if type(action) in (tuple, list):
					first, self.action_filename, self.action_numline = action[0]
					if ' ' in first:
						index = first.index(' ')
						command = first[:index]
						extra_param = first[index+1:]
						action = [command, extra_param, action[1:]]
					else:
						command = first
						extra_param = '%s: %s_%s' % (self.data.sprite, command, self.action_num)
					
					
					if self.parallels and command != 'parallel':
						self.action_num -= 1
						return
					
					if command == 'block':
						self.block = SpriteAnimation(action[1:], self.data)
						return
					
					if command == 'contains':
						if self.last_command != command:
							self.data.contains = []
						spr = Sprite(extra_param, (), (), action[1:], None)
						self.data.contains.append(spr)
					elif command == 'parallel':
						if self.last_command != command:
							self.parallels = []
						self.parallels.append(SpriteAnimation(action[1:], self.data))
					else:
						self.out_msg('SpriteAnimation.update', 'Expected blocks <contains>, <block> or <parallel>, got <%s>' % (command, ))
					
					self.last_command = command
					continue
				
				
				self.out_msg('SpriteAnimation.update', 'Command type is not str or list:\ntype = %s\n%s' % (type(action), action))
		
		def save_change_params(self, args):
			self.change_props = []
			
			for i in range(0, len(args), 2):
				names = get_prop_names(args[i])
				
				new_value = eval(args[i + 1])
				if isinstance(names, (list, tuple)):
					if not isinstance(new_value, (list, tuple)):
						new_value = [new_value] * len(names)
					
					old_props = names
					if old_props == ('xalign', 'yalign'):
						old_props = ('xpos', 'ypos')
					old_value = [self.data[name] for name in old_props]
				else:
					old_value = self.data[names]
				
				self.change_props.append((names, old_value, new_value))
		
		def update_changing(self):
			dtime = get_game_time() - self.start_changing_time
			all_time = max(self.end_changing_time - self.start_changing_time, 0.001)
			
			t = in_bounds(dtime / all_time, 0.0, 1.0)
			t = in_bounds(self.change_func(t), 0.0, 1.0)
			for prop in self.change_props:
				name, old_value, new_value = prop
				
				if isinstance(new_value, (list, tuple)):
					value = []
					for i in range(len(old_value)):
						new_v = new_value[i]
						old_v = old_value[i]
						type_v = float if type(old_v) is float else type(new_v)
						
						v = type_v((new_v - old_v) * t + old_v)
						value.append(v)
				else:
					type_v = float if type(old_value) is float else type(new_value)
					value = type_v((new_value - old_value) * t + old_value)
				self.set_prop(name, value)
		
		
		def set_prop(self, prop, value):
			if isinstance(prop, str):
				props = get_atl_props(prop)
				if props is None:
					self.out_msg('SpriteAnimation.set_prop', 'Unknown property <%s>' % (prop, ))
					return
			else:
				props = prop
			
			if props == ['xalign']:
				self.data.xalign = value
				props = ['xpos', 'xanchor']
				value = [value, value]
			elif props == ['yalign']:
				self.data.yalign = value
				props = ['ypos', 'yanchor']
				value = [value, value]
			
			
			def out_not_number_err(expected, value):
				self.out_msg('SpriteAnimation.set_prop', 'Expected %s, got <%s>' % (expected, type(value).__name__))
			
			if len(props) == 1:
				if isinstance(value, (list, tuple)):
					self.out_msg('SpriteAnimation.set_prop', 'Expected single value, got list: %s' % (value, ))
				else:
					if type(value) not in number_types:
						out_not_number_err('int, float or absolute', value)
						return
					
					if self.data[prop] != value:
						self.data[prop] = value
						
						if prop not in self.data.except_state_props:
							self.data.state_num += 1
			else:
				def out_len_err():
					self.out_msg('SpriteAnimation.set_prop', 'Expected list with len %s, got: %s' % (len(props), value))
				
				if not isinstance(value, (list, tuple)):
					if type(value) not in number_types:
						out_not_number_err('int, float, absolute or list/tuple of them', value)
						return
					
					if len(props) == 2:
						self.set_prop(props[0], value)
						self.set_prop(props[1], value)
					else:
						out_len_err()
				else:
					if len(props) == len(value):
						for i in range(len(props)):
							self.set_prop(props[i], value[i])
					else:
						out_len_err()
	
	def get_default_transform_actions():
		at = globals().get('default_decl_at')
		if not at:
			return ()
		
		t = type(at)
		if t is SpriteAnimation:
			return at.actions
		
		if t in (tuple, list) and len(at) == 1 and at[0] in ('size 1.0', 'size (1.0, 1.0)'):
			# const tuple (without file & line) = const ref to this tuple
			#   (optimization for cache in Sprite::registerImage, parseAction)
			return (('size 1.0', 'None', 0), )
		
		out_msg(
			'get_default_transform_actions',
			'Use <transform> statement (not list or tuple in python), examples - <Ren-Engine/rpy/vn/sprite_animation.rpy>'
		)
		return ()


init -9000:
	transform empty_transform
	
	
	transform fleft:
		pos (0.16, 1.0)
		anchor (0.5, 1.0)
	transform left:
		pos (0.28, 1.0)
		anchor (0.5, 1.0)
	transform cleft:
		pos (0.355, 1.0)
		anchor (0.5, 1.0)
	
	transform center:
		pos (0.5, 1.0)
		anchor (0.5, 1.0)
	
	transform cright:
		pos (0.645, 1.0)
		anchor (0.5, 1.0)
	transform right:
		pos (0.72, 1.0)
		anchor (0.5, 1.0)
	transform fright:
		pos (0.84, 1.0)
		anchor (0.5, 1.0)
	
	
	transform true_center:
		pos (0.5, 0.5)
		anchor (0.5, 0.5)
	transform left_center:
		pos (0.28, 0.5)
		anchor (0.5, 0.5)
	transform true_center:
		pos (0.72, 0.5)
		anchor (0.5, 0.5)
