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
		'xy': ('pos', 'anchor', 'align', 'size', 'zoom'),
		'crop': ('xcrop', 'ycrop', 'xsizecrop', 'ysizecrop'),
	}
	
	# alpha -> [alpha]
	# xpos  -> [xpos]
	#  pos  -> [xpos, ypos]
	def get_atl_props(prop):
		if prop in atl_props['simple'] or (prop[0] in 'xy' and prop[1:] in atl_props['xy']):
			return (prop, )
		if prop in atl_props['xy']:
			return ('x' + prop, 'y' + prop)
		return atl_props.get(prop, None)
	
	
	def get_prop_names(prop):
		props = get_atl_props(prop)
		
		if len(props) == 1:
			if prop == 'xalign':
				return ('xpos', 'xanchor')
			if prop == 'yalign':
				return ('ypos', 'yanchor')
			return prop
		
		return props
	
	
	class SpriteAnimation(Object):
		def __init__(self, actions, data = None):
			Object.__init__(self)
			
			self.ended = False
			self.repeated = {}
			
			self.action_num = 0
			self.actions = actions
			self.last_command = ''
			
			self.block = None
			self.parallels = []
			
			self.data = data
			
			self.end_pause_time = 0
			
			self.start_changing_time = 0
			self.end_changing_time = 0
		
		
		def copy(self):
			return SpriteAnimation(self.actions)
		
		
		def update(self):
			now = get_game_time()
			if now < self.end_pause_time:
				return
			
			if self.start_changing_time:
				self.update_changing()
				if now <= self.end_changing_time:
					return
				else:
					self.start_changing_time = 0
			
			
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
					action = self.actions[self.action_num]
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
					
					if command == 'pause':
						if len(args) == 2:
							self.end_pause_time = get_game_time() + float(args[1])
						else:
							out_msg('SpriteAnimation.update', 'pause expected 1 argument: time\n' + action)
						return
					elif command == 'repeat':
						if len(args) > 2:
							out_msg('SpriteAnimation.update', 'repeat expected 1 optional argument: count of repeats\n' + action)
						
						count = int(10e9) if len(args) == 1 else int(args[1])
						num = self.action_num - 1
						repeated = self.repeated.get(num, 0)
						
						if repeated < count:
							self.action_num = 0
							self.repeated[num] = self.repeated.get(num, 0) + 1
							
							for key in self.repeated.keys():
								if key < num:
									self.repeated[key] = 0
							
							return
					else:
						is_prop = get_atl_props(command) is not None
						if is_prop:
							name = args[0]
							expr = ' '.join(args[1:])
							try:
								value = eval(expr)
							except Exception as e:
								out_msg('SpriteAnimation.update', 'On Eval: ' + str(expr) + ', exception: ' + str(e))
							else:
								self.set_prop(name, value)
						else:
							try:
								evaled = eval(action)
								if isinstance(evaled, str):
									if image_was_registered(evaled):
										image_actions = get_image(evaled)
										self.action_num -= 1
										self.actions = self.actions[0:self.action_num] + image_actions + self.actions[self.action_num+1:]
									else:
										self.data.contains = []
										self.data.image = evaled
								elif isinstance(evaled, (int, float, long)):
									self.end_pause_time = get_game_time() + float(evaled)
									return
								else:
									out_msg('SpriteAnimation.update', 'Unknown command:\n' + action)
							except:
								try:
									evaled = eval(command)
									if str(type(evaled)) == "<type 'function'>":
										if len(args) % 2:
											desc = command + ' expected odd count of arguments: time, [param value]+\n' + action
											out_msg('SpriteAnimation.update', desc)
										else:
											self.change_func = evaled
											self.start_changing_time = get_game_time()
											self.end_changing_time = get_game_time() + float(args[1])
											self.save_change_params(args[2:])
										return
									else:
										out_msg('SpriteAnimation.update', 'Unknown command:\n' + action)
								except:
									out_msg('SpriteAnimation.update', 'Exception:\n' + action)
				elif type(action) is list:
					if ' ' in action[0]:
						index = action[0].index(' ')
						command = action[0][0:index]
						extra_param = action[0][index+1:]
						action = [command, extra_param, action[1:]]
					else:
						command = action[0]
						extra_param = str(self.data.sprite) + ': ' + command + '_' + str(self.action_num)
					
					
					if self.parallels and command != 'parallel':
						self.action_num -= 1
						return
					
					
					if command == 'contains':
						if self.last_command != command:
							self.data.contains = []
						spr = Sprite([], [], action[1:], None)
						spr.call_str = extra_param
						self.data.contains.append(spr)
					elif command == 'block':
						self.block = SpriteAnimation(action[1:], self.data)
						return
					elif command == 'parallel':
						if self.last_command != command:
							self.parallels = []
						self.parallels.append(SpriteAnimation(action[1:], self.data))
					else:
						out_msg('SpriteAnimation.update', 'Expected blocks <contains>, <block> or <parallel>, got <' + str(action[0]) + '>')
					
					self.last_command = command
				else:
					out_msg('SpriteAnimation.update', 'Command type is not str or list:\n' + type(action) + '\n' + str(action))
		
		def save_change_params(self, args):
			self.change_props = []
			
			for i in xrange(0, len(args), 2):
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
					for i in xrange(len(old_value)):
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
					out_msg('SpriteAnimation.set_prop', 'Unknown property <' + prop + '>')
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
			
			if len(props) == 1:
				if isinstance(value, (list, tuple)):
					out_msg('SpriteAnimation.set_prop',
							'Value expected single, got list: ' + str(value))
				elif self.data[prop] != value:
					self.data[prop] = value
					
					if prop not in self.data.except_state_props:
						self.data.state_num += 1
			else:
				err_msg = 'Value expected list with ' + str(len(props)) + ' items, got: ' + str(value)
				
				if not isinstance(value, (list, tuple)):
					if len(props) == 2:
						self.set_prop(props[0], value)
						self.set_prop(props[1], value)
					else:
						out_msg('SpriteAnimation.set_prop', err_msg)
				else:
					if len(props) == len(value):
						for i in xrange(len(props)):
							self.set_prop(props[i], value[i])
					else:
						out_msg('SpriteAnimation.set_prop', err_msg)
	
	
	
	
	
	def get_sprite_place(xpos, ypos, xanchor, yanchor):
		actions = (
			'pos ' + str((xpos, ypos)),
			'anchor ' + str((xanchor, yanchor))
		)
		
		res = SpriteAnimation(actions)
		return res
	
	fleft  = get_sprite_place(0.16,  1.0, 0.5, 1.0)
	left   = get_sprite_place(0.28,  1.0, 0.5, 1.0)
	cleft  = get_sprite_place(0.355, 1.0, 0.5, 1.0)
	center = get_sprite_place(0.50,  1.0, 0.5, 1.0)
	cright = get_sprite_place(0.645, 1.0, 0.5, 1.0)
	right  = get_sprite_place(0.72,  1.0, 0.5, 1.0)
	fright = get_sprite_place(0.84,  1.0, 0.5, 1.0)
	
	true_center = get_sprite_place(0.5, 0.5, 0.5, 0.5)
	left_center  = get_sprite_place(0.28, 0.5, 0.5, 0.5)
	right_center = get_sprite_place(0.72, 0.5, 0.5, 0.5)
	
