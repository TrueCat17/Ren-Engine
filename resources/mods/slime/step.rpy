init -1 python:
	# step = [x, y, agent_id, only_view, circle, gen_num]
	
	cell_size = sensors_dist
	remove_in_cell_dist2 = (cell_size / 6) ** 2
	use_colors = False
	
	def init_steps():
		global cur_gen_num, step_generations, cells
		cur_gen_num = 0
		step_generations = [[] for i in range(step_life_time)]
		cells = defaultdict(list)
	
	def add_steps(count):
		for i in range(count):
			step_generations.insert(0, [])
		global step_life_time
		step_life_time = len(step_generations)
	
	def del_steps(count):
		step_generations[:count] = []
		global step_life_time
		step_life_time = len(step_generations)
	
		
	def remove_step_from_cell(step):
		if not step[3]:
			cells[(step[0] // cell_size, step[1] // cell_size)].remove(step)
	
	def add_step(agent):
		x = int(round(agent.x))
		y = int(round(agent.y))
		new_step = [x, y, agent.agent_id, False, agent.circle, cur_gen_num]
		cur_step_generation.append(new_step)
		
		cell = cells[(x // cell_size, y // cell_size)]
		i = 0
		while i < len(cell):
			old_step = cell[i]
			tdx = old_step[0] - x
			tdy = old_step[1] - y
			if tdx*tdx + tdy*tdy < remove_in_cell_dist2:
				old_step[3] = True
				cell.pop(i)
			else:
				i += 1
		cell.append(new_step)
	
	def update_steps():
		global cur_gen_num
		cur_gen_num += 1
		
		step_generations.append([])
		global cur_step_generation
		cur_step_generation = step_generations[-1]
		
		for step in step_generations[0]:
			remove_step_from_cell(step)
		step_generations.pop(0)
	
	
	def get_power_from(x, y, agent_id):
		sensors_dist2 = sensors_dist * sensors_dist
		center_xcell = int(x) // cell_size
		center_ycell = int(y) // cell_size
		
		res = 0
		for dy in (-1, 0, +1):
			for dx in (-1, 0, +1):
				cell = cells.get((center_xcell + dx, center_ycell + dy))
				if not cell: continue
				
				for step in cell:
					if step[2] == agent_id:
						continue
					
					tdx = step[0] - x
					tdy = step[1] - y
					dist = tdx*tdx + tdy*tdy
					if dist <= sensors_dist2:
						#res = max((cur_gen_num - step[5]) / (dist or 0.01), res)
						res += (cur_gen_num - step[5]) / (dist or 0.01)
		return res
