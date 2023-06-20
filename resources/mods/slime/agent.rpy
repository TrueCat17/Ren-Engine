init -2 python:
	speed = 4
	sensors_dist = 32
	
	escaping_power = 200
	escaping_time = 20
	escaping_chance = 0.003
	
	agent_random_colors = [
		[0, 255, 0],
		[0, 0, 255],
		[0, 255, 255],
		[255, 0, 0],
		[255, 0, 255],
		[255, 255, 0],
		[255, 255, 255],
	]
	agent_index = 0
	
	class Agent:
		def __init__(self):
			global agent_index
			agent_index += 1
			self.agent_id = agent_index
			
			group = agent_index // 20
			r, g, b = agent_random_colors[group % len(agent_random_colors)]
			self.circle = im.recolor(circle, r, g, b)
			
			self.x = random.randint(0, get_stage_width() - 1)
			self.y = random.randint(0, get_stage_height() - 1)
			self.angle = random.randint(0, 359)
			
			self.escaping = 0
		
		def update(self):
			x, y, angle, agent_id = self.x, self.y, self.angle, self.agent_id
			
			lx = x + _cos(angle - sensors_angle) * sensors_dist
			ly = y + _sin(angle - sensors_angle) * sensors_dist
			
			cx = x + _cos(angle) * sensors_dist
			cy = y + _sin(angle) * sensors_dist
			
			rx = x + _cos(angle + sensors_angle) * sensors_dist
			ry = y + _sin(angle + sensors_angle) * sensors_dist
			
			lpower = get_power_from(lx, ly, agent_id)
			cpower = get_power_from(cx, cy, agent_id)
			rpower = get_power_from(rx, ry, agent_id)
			
			if not self.escaping:
				mpower = max(lpower, cpower, rpower)
				if mpower >= escaping_power and random.random() < escaping_chance:
					self.escaping = escaping_time
			
			if self.escaping:
				self.escaping -= 1
				mpower = min(lpower, cpower, rpower)
			
			if mpower == cpower:
				pass
			elif mpower == lpower:
				self.angle -= rotation_angle
			else:
				self.angle += rotation_angle
			
			self.angle += extra_rotation + random.randint(-wobble, +wobble)
			self.x = (self.x + _cos(self.angle) * speed) % stage_width
			self.y = (self.y + _sin(self.angle) * speed) % stage_height
			
			add_step(self)
	
	def init_agents():
		global agents
		agents = []
		add_agents(agent_count)
	
	
	def add_agents(count):
		for i in range(count):
			agents.append(Agent())
		global agent_count
		agent_count = len(agents)
	
	def del_agents(count):
		agents[len(agents) - count:] = []
		global agent_count
		agent_count = len(agents)
	
	
	def update_agents():
		for agent in agents:
			agent.update()
