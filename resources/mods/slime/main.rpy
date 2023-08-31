init python:
	import itertools
	
	quick_menu = False
	config.has_autosave = False
	start_screens += ['slime', 'panel']
	
	AGENT_COLOR = '#F00'
	ESCAPING_AGENT_COLOR = '#00F'
	
	circle = 'mods/slime/circle.webp'
	agent_circle = im.recolor(circle, *renpy.easy.color(AGENT_COLOR))
	escaping_agent_circle = im.recolor(circle, *renpy.easy.color(ESCAPING_AGENT_COLOR))
	
	updates_before_start = 20
	
	
	def random_params():
		global agent_count, rotation_angle, sensors_angle, extra_rotation, wobble
		old_agent_count = agent_count
		
		agent_count = random.randint(100 // 10, 400 // 10) * 10
		if random.random() < 0.75:
			rotation_angle = random.randint(15 // 5, 50 // 5) * 5
			sensors_angle = random.randint(15 // 5, 50 // 5) * 5
		else:
			rotation_angle = random.randint(5 // 5, 175 // 5) * 5
			sensors_angle = random.randint(5 // 5, 175 // 5) * 5
		extra_rotation = random.randint(0, 6)
		wobble = random.randint(0, 10)
		
		if agent_count > old_agent_count:
			add_agents(agent_count - old_agent_count)
		else:
			del_agents(old_agent_count - agent_count)
		
		global step_life_time
		old = step_life_time
		step_life_time = random.randint(20 // 5, 80 // 5) * 5
		if step_life_time > old:
			add_steps(step_life_time - old)
		else:
			del_steps(old - step_life_time)
	
	def default_params(on_start = False):
		global agent_count, rotation_angle, sensors_angle, extra_rotation, wobble
		old_agent_count = -1 if on_start else agent_count
		
		agent_count = 140
		rotation_angle = 35
		sensors_angle = 45
		extra_rotation = 0
		wobble = 1
		
		if agent_count > old_agent_count:
			if old_agent_count > -1:
				add_agents(agent_count - old_agent_count)
		else:
			del_agents(old_agent_count - agent_count)
		
		global step_life_time
		if on_start:
			step_life_time = 60
		else:
			old = step_life_time
			step_life_time = 60
			if step_life_time > old:
				add_steps(step_life_time - old)
			else:
				del_steps(old - step_life_time)
	
	
	def update():
		global stage_width, stage_height
		stage_width, stage_height = get_stage_size()
		stage_width -= panel_size
		
		update_steps()
		update_agents()
	
	def start():
		init_steps()
		init_agents()
		
		for i in range(max(updates_before_start, 1)):
			update()
	
	default_params(True)
	start()
	
	
	def resized_stage():
		kx = (get_stage_width() - panel_size) / stage_width
		ky = get_stage_height() / stage_height
		
		global cells
		cells = defaultdict(list)
		
		xcount_cell = (get_stage_width() - panel_size - 1) // cell_size + 1
		ycount_cell = (get_stage_height() - 1) // cell_size + 1
		
		for i in range(-1, xcount_cell + 1):
			cells[(i, -1)] = cells[(i % xcount_cell, ycount_cell - 1)]
			cells[(i, ycount_cell)] = cells[(i % xcount_cell, 0)]
		for i in range(ycount_cell):
			cells[(-1, i)] = cells[(xcount_cell - 1, i)]
			cells[(xcount_cell, i)] = cells[(0, i)]
		
		# step = [x, y, agent_id, cell, circle, gen_num]
		for step_gen in step_generations:
			for step in step_gen:
				step[0] = int(step[0] * kx)
				step[1] = int(step[1] * ky)
				
				if step[3]:
					step[3] = cells[(step[0] // cell_size, step[1] // cell_size)]
					step[3].append(step)
		
		for agent in agents:
			agent.x *= kx
			agent.y *= ky
	
	resized_stage()
	signals.add('resized_stage', resized_stage)


screen slime:
	$ update()
	
	# step = [x, y, agent_id, cell, circle, gen_num]
	if use_colors:
		for step in itertools.chain.from_iterable(step_generations):
			image step[4]:
				align 0.5
				size 16
				xpos step[0]
				ypos step[1]
				alpha (cur_gen_num - step[5]) / step_life_time * 2
	else:
		for step in itertools.chain.from_iterable(step_generations):
			image 'mods/slime/circle.webp':
				align 0.5
				size 16
				xpos step[0]
				ypos step[1]
				alpha (cur_gen_num - step[5]) / step_life_time * 2
	
	if show_agents:
		for agent in agents:
			image (escaping_agent_circle if agent.escaping else agent_circle):
				align 0.5
				size 8
				xpos agent.x
				ypos agent.y
