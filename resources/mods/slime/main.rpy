init python:
	quick_menu = False
	config.has_autosave = False
	
	start_screens += ['slime', 'panel']
	start_screens.remove('dialogue_box')
	
	AGENT_COLOR = '#F00'
	ESCAPING_AGENT_COLOR = '#00F'
	
	circle = 'mods/slime/circle.webp'
	agent_circle          = im.recolor(circle, *renpy.easy.color(AGENT_COLOR))
	escaping_agent_circle = im.recolor(circle, *renpy.easy.color(ESCAPING_AGENT_COLOR))
	
	updates_before_start = 20
	
	
	def random_params():
		global agent_count, rotation_angle, sensors_angle, extra_rotation, wobble
		old_agent_count = agent_count
		
		def random_with_step(min, max, step):
			return random.randint(min // step, max // step) * step
		
		agent_count = random_with_step(100, 400, 10)
		if random.random() < 0.75:
			rotation_angle = random_with_step(15, 50, 5)
			sensors_angle  = random_with_step(15, 50, 5)
		else:
			rotation_angle = random_with_step(5, 175, 5)
			sensors_angle  = random_with_step(5, 175, 5)
		extra_rotation = random.randint(0, 6)
		wobble = random.randint(0, 10)
		
		if agent_count > old_agent_count:
			add_agents(agent_count - old_agent_count)
		else:
			del_agents(old_agent_count - agent_count)
		
		global step_life_time
		old = step_life_time
		step_life_time = random_with_step(20, 80, 5)
		if step_life_time > old:
			add_step_generations(step_life_time - old)
		else:
			del_step_generations(old - step_life_time)
	
	def default_params(on_start = False):
		global agent_count, rotation_angle, sensors_angle, extra_rotation, wobble
		old_agent_count = 0 if on_start else agent_count
		
		agent_count = 140
		rotation_angle = 35
		sensors_angle = 45
		extra_rotation = 0
		wobble = 1
		
		if agent_count > old_agent_count:
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
				add_step_generations(step_life_time - old)
			else:
				del_step_generations(old - step_life_time)
	
	
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
		for gen_num, step_gen in step_generations:
			for step in step_gen:
				step[0] = int(step[0] * kx)
				step[1] = int(step[1] * ky)
				
				if step[3]:
					step[3].remove(step)
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
		for gen_num, step_gen in step_generations:
			null:
				alpha (cur_gen_num - gen_num) / step_life_time * 2
				
				for step in step_gen:
					image step[4]:
						xpos step[0]
						ypos step[1]
						anchor 0.5
						size 16
	
	else:
		for gen_num, step_gen in step_generations:
			null:
				alpha (cur_gen_num - gen_num) / step_life_time * 2
				
				for step in step_gen:
					image 'mods/slime/circle.webp':
						xpos step[0]
						ypos step[1]
						anchor 0.5
						size 16
	
	if show_agents:
		for agent in agents:
			image (escaping_agent_circle if agent.escaping else agent_circle):
				align 0.5
				size 8
				xpos agent.x
				ypos agent.y
