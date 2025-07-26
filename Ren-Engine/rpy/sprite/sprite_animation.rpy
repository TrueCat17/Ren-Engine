init -9000 python:
	
	def linear(t):
		return t
	def ease(t):
		return 0.5 - math.cos(math.pi * t) / 2
	def easein(t):
		return math.cos((1.0 - t) * math.pi / 2)
	def easeout(t):
		return 1.0 - math.cos(t * math.pi / 2)
	
	
	atl_props = {
		'simple': ('alpha', 'rotate'),
		'xy': ('pos', 'anchor', 'align', 'size', 'size_min', 'size_max', 'zoom'), # size_min and size_max - just for class <Style>, not for ATL
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
		if props is None:
			return props
		
		if len(props) == 1:
			if prop == 'xalign':
				return ('xpos', 'xanchor')
			if prop == 'yalign':
				return ('ypos', 'yanchor')
		
		return props
	
	
	sprite_animation_code_cache = DontSave()
	
	class SpriteAnimation(SimpleObject):
		def __init__(self, actions, sprite = None):
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
			
			self.sprite = sprite
			
			self.end_pause_time = 0
			
			self.start_changing_time = None
			self.end_changing_time = None
		
		def copy(self, sprite):
			res = SpriteAnimation.__new__(SpriteAnimation)
			SimpleObject.__init__(res, obj = self)
			res.sprite = sprite
			
			if res.block:
				res.block = res.block.copy(sprite)
			res.parallels = [parallel.copy(sprite) for parallel in res.parallels]
			
			return res
		
		
		def out_msg(self, func, msg, *args):
			out_msg(func, 'Place of transform command: %s:%s\n\n%s', self.action_filename, self.action_numline, msg % tuple(args))
		
		
		def eval(self, code):
			compiled = sprite_animation_code_cache.get(code)
			if not compiled:
				compiled = sprite_animation_code_cache[code] = compile(code, 'None', 'eval')
			
			return eval(compiled)
		
		def update(self):
			now = get_game_time()
			if now < self.end_pause_time:
				return
			
			if self.start_changing_time is not None:
				self.update_changing_params()
				if now <= self.end_changing_time:
					return
				
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
			
			
			actions = self.actions
			len_actions = len(actions)
			
			if self.action_num >= len_actions:
				self.ended = True
				return
			
			
			
			while self.action_num < len_actions:
				action = ''
				while not action and self.action_num < len_actions:
					action, self.action_filename, self.action_numline = actions[self.action_num]
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
								pause_time = self.eval(args[1])
							except Exception as e:
								self.out_msg('SpriteAnimation.update', 'On Eval: %s, exception: %s', args[1], e)
							else:
								self.end_pause_time = get_game_time() + pause_time
						else:
							self.out_msg('SpriteAnimation.update', '<pause> expected 1 argument: time\n%s', action)
						return
					
					if command == 'repeat':
						if len(args) > 2:
							self.out_msg('SpriteAnimation.update', '<repeat> expected 1 optional argument: count of repeats\n%s', action)
						
						count = int(1e9 if len(args) == 1 else args[1])
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
						i = action.index(command)
						expr = action[i + len(command):].strip()
						try:
							value = self.eval(expr)
						except Exception as e:
							self.out_msg('SpriteAnimation.update', 'On Eval: %s, exception: %s', expr, e)
						else:
							self.set_prop(command, value)
						continue
					
					try:
						evaled = self.eval(action)
						if type(evaled) in number_types:
							self.end_pause_time = get_game_time() + evaled
							return
						
						if type(evaled) is str:
							if image_was_registered(evaled):
								image_actions = get_image(evaled)
								self.action_num -= 1
								actions = self.actions = actions[:self.action_num] + image_actions + actions[self.action_num+1:]
								len_actions = len(actions)
							else:
								self.sprite.contains = []
								self.sprite.image = evaled
						else:
							self.out_msg('SpriteAnimation.update', 'Unknown command:\n%s', action)
					except:
						try:
							evaled = self.eval(command)
							if callable(evaled):
								if len(args) % 2:
									desc = '%s expected odd count of arguments: time, [param value]+\n%s'
									self.out_msg('SpriteAnimation.update', desc, command, action)
								else:
									self.change_func = evaled
									self.start_changing_time = get_game_time()
									self.end_changing_time = get_game_time() + self.eval(args[1])
									self.save_changing_params(args[2:])
								return
							self.out_msg('SpriteAnimation.update', 'Unknown command:\n%s', action)
						except:
							self.out_msg('SpriteAnimation.update', 'Exception on:\n%s', action)
					continue
				
				
				if type(action) in (tuple, list):
					first, self.action_filename, self.action_numline = action[0]
					if ' ' in first:
						i = first.index(' ')
						command = first[:i]
						extra_param = first[i+1:]
						action = [command, extra_param, action[1:]]
					else:
						command = first
						extra_param = '%s: %s_%s' % (self.sprite, command, self.action_num)
					
					
					if self.parallels and command != 'parallel':
						self.action_num -= 1
						return
					
					if command == 'block':
						self.block = SpriteAnimation(action[1:], self.sprite)
						return
					
					if command == 'contains':
						if self.last_command != command:
							self.sprite.contains = []
						self.sprite.calculate_props()
						spr = Sprite(None, extra_param, (), (), action[1:], None)
						spr.parent = self.sprite
						self.sprite.contains.append(spr)
					elif command == 'parallel':
						if self.last_command != command:
							self.parallels = []
						self.parallels.append(SpriteAnimation(action[1:], self.sprite))
					else:
						self.out_msg('SpriteAnimation.update', 'Expected blocks <contains>, <block> or <parallel>, got <%s>', command)
					
					self.last_command = command
					continue
				
				
				self.out_msg('SpriteAnimation.update', 'Command type is not str or list:\ntype = %s\n%s', type(action).__name__, action)
		
		def save_changing_params(self, args):
			self.change_props = []
			
			sequences = (list, tuple)
			for i in range(0, len(args), 2):
				names = get_prop_names(args[i])
				if names is None:
					self.out_msg('SpriteAnimation.save_changing_params', 'Unknown property <%s>', args[i])
					continue
				
				new_values = self.eval(args[i + 1])
				if type(new_values) not in sequences:
					new_values = [new_values] * len(names)
				elif len(names) == 1:
					self.out_msg('SpriteAnimation.save_changing_params', 'Expected single value, got list: %s', new_values)
				elif len(names) != len(new_values):
					self.out_msg('SpriteAnimation.save_changing_params', 'Expected %s values, got: %s', len(names), new_values)
				
				if names == ['xalign', 'yalign']:
					names = ['xpos', 'ypos', 'xanchor', 'yanchor']
					new_values *= 2
				old_values = [self.sprite[name] for name in names]
				
				self.change_props.append((names, old_values, new_values))
		
		def update_changing_params(self):
			dtime = get_game_time() - self.start_changing_time
			all_time = max(self.end_changing_time - self.start_changing_time, 0.001)
			
			t = in_bounds(dtime / all_time, 0.0, 1.0)
			t = in_bounds(self.change_func(t), 0.0, 1.0)
			
			
			def get_abs(prop, value):
				side = prop[0]
				prop = prop[1:]
				if type(value) is not float or side not in 'xy' or prop not in ('anchor', 'pos', 'size'):
					return absolute(value)
				
				# see Sprite.calculate_props
				
				sprite = self.sprite
				
				if prop == 'anchor':
					zoom = sprite['real_' + side + 'zoom']
					max = sprite['real_' + side + 'size'] / zoom
				
				else:
					parent = sprite.parent
					if parent:
						p_zoom = parent['real_' + side + 'zoom']
						p_size = parent['real_' + side + 'size'] / p_zoom
					else:
						if side == 'x':
							p_size = config.width or get_stage_width()
						else:
							p_size = config.height or get_stage_height()
					
					max = p_size
				
				return absolute(value * max)
			
			
			for prop in self.change_props:
				for name, old_value, new_value in zip(*prop):
					if t == 1.0:
						value = new_value
					else:
						old_value = get_abs(name, old_value)
						new_value = get_abs(name, new_value)
						value = old_value + (new_value - old_value) * t
					
					self.set_prop(name, value)
		
		
		def set_prop(self, prop, value):
			sequences = (list, tuple)
			
			if type(prop) is str:
				props = get_atl_props(prop)
				if props is None:
					self.out_msg('SpriteAnimation.set_prop', 'Unknown property <%s>', prop)
					return
			else:
				if type(prop) not in sequences:
					self.out_msg('SpriteAnimation.set_prop', 'Expected str, list or tuple as type(prop), got %s', type(prop).__name__)
					return
				
				props = prop
			
			if len(props) == 1:
				if prop == 'xalign':
					props = ['xpos', 'xanchor']
					value = [value, value]
				elif prop == 'yalign':
					props = ['ypos', 'yanchor']
					value = [value, value]
			
			
			def out_not_number_err(expected, value):
				self.out_msg('SpriteAnimation.set_prop', 'Expected %s, got <%s>', expected, type(value).__name__)
			
			if len(props) == 1:
				if type(value) in sequences:
					self.out_msg('SpriteAnimation.set_prop', 'Expected single value, got list: %s', value)
				else:
					if type(value) not in number_types:
						out_not_number_err('int, float or absolute', value)
						return
					
					sprite = self.sprite
					if sprite[prop] != value:
						sprite[prop] = value
			else:
				if type(value) not in sequences:
					if type(value) not in number_types:
						out_not_number_err('int, float, absolute or list/tuple of them', value)
						return
					
					if len(props) == 2:
						self.set_prop(props[0], value)
						self.set_prop(props[1], value)
					else:
						self.out_msg('SpriteAnimation.set_prop', 'Expected 1 or 2 props, got: %s', props)
				else:
					if len(props) == len(value):
						for prop, value in zip(props, value):
							self.set_prop(prop, value)
					else:
						self.out_msg('SpriteAnimation.set_prop', 'Expected list with len %s, got: %s', len(props), value)
	
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
	transform right_center:
		pos (0.72, 0.5)
		anchor (0.5, 0.5)
