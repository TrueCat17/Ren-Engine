init python:
	def norm_angle(angle):
		return angle % 360
	
	def angle_to_x(angle):
		x = SCREEN_DIST - math_tan(math_radians(angle)) * H_WIDTH
		return int(x)
	
	def point_to_angle(vertex):
		player_x, player_y = player.pos
		dx = vertex[0] - player_x
		dy = vertex[1] - player_y
		return math_degrees(math_atan2(dy, dx))
	
	def is_on_back_side(node):
		player_x, player_y = player.pos
		dx = player_x - node.x_partition
		dy = player_y - node.y_partition
		return dx * node.dy_partition <= dy * node.dx_partition
	
	
	class BSP:
		SUB_SECTOR_IDENTIFIER = 0x8000  # 2**15 = 32768
		
		def __init__(self, engine):
			self.engine = engine
			self.nodes = engine.wad_data.nodes
			self.sub_sectors = engine.wad_data.sub_sectors
			self.segs = engine.wad_data.segments
			self.root_node_id = len(self.nodes) - 1
			self.is_traverse_bsp = True
		
		def update(self):
			self.is_traverse_bsp = True
			self.render_bsp_node(node_id=self.root_node_id)
		
		def get_sub_sector_height(self):
			sub_sector_id = self.root_node_id
			
			while not sub_sector_id >= self.SUB_SECTOR_IDENTIFIER:
				node = self.nodes[sub_sector_id]
				
				if is_on_back_side(node):
					sub_sector_id = self.nodes[sub_sector_id].back_child_id
				else:
					sub_sector_id = self.nodes[sub_sector_id].front_child_id
			
			sub_sector = self.sub_sectors[sub_sector_id - self.SUB_SECTOR_IDENTIFIER]
			seg = self.segs[sub_sector.first_seg_id]
			return seg.front_sector.floor_height
		
		def add_segment_to_fov(self, vertex1, vertex2):
			angle1 = point_to_angle(vertex1)
			angle2 = point_to_angle(vertex2)
			
			span = norm_angle(angle1 - angle2)
			# backface culling
			if span >= 180.0:
				return False
			
			# needed for further calculations
			rw_angle1 = angle1
			
			angle1 -= player.angle
			angle2 -= player.angle
			
			span1 = norm_angle(angle1 + H_FOV)
			if span1 > FOV:
				if span1 >= span + FOV:
					return False
				# clipping
				angle1 = H_FOV
			
			span2 = norm_angle(H_FOV - angle2)
			if span2 > FOV:
				if span2 >= span + FOV:
					return False
				# clipping
				angle2 = -H_FOV
			
			x1 = angle_to_x(angle1)
			x2 = angle_to_x(angle2)
			return x1, x2, rw_angle1
		
		def render_sub_sector(self, sub_sector_id):
			sub_sector = self.sub_sectors[sub_sector_id]
			classify_segment = self.engine.seg_handler.classify_segment
			
			for i in range(sub_sector.seg_count):
				seg = self.segs[sub_sector.first_seg_id + i]
				if result := self.add_segment_to_fov(seg.start_vertex, seg.end_vertex):
					classify_segment(seg, *result)
		
		def check_bbox(self, bbox):
			a, b = (bbox.left, bbox.bottom), (bbox.left, bbox.top)
			c, d = (bbox.right, bbox.top), (bbox.right, bbox.bottom)
			
			px, py = player.pos
			if px < bbox.left:
				if py > bbox.top:
					bbox_sides = (b, a), (c, b)
				elif py < bbox.bottom:
					bbox_sides = (b, a), (a, d)
				else:
					bbox_sides = (b, a),
			elif px > bbox.right:
				if py > bbox.top:
					bbox_sides = (c, b), (d, c)
				elif py < bbox.bottom:
					bbox_sides = (a, d), (d, c)
				else:
					bbox_sides = (d, c),
			else:
				if py > bbox.top:
					bbox_sides = (c, b),
				elif py < bbox.bottom:
					bbox_sides = (a, d),
				else:
					return True
			
			for v1, v2 in bbox_sides:
				angle1 = point_to_angle(v1)
				angle2 = point_to_angle(v2)
				
				span = norm_angle(angle1 - angle2)
				
				angle1 -= player.angle
				span1 = norm_angle(angle1 + H_FOV)
				if span1 > FOV:
					if span1 >= span + FOV:
						continue
				return True
			return False
		
		def render_bsp_node(self, node_id):
			if self.is_traverse_bsp:
				
				if node_id >= self.SUB_SECTOR_IDENTIFIER:
					sub_sector_id = node_id - self.SUB_SECTOR_IDENTIFIER
					self.render_sub_sector(sub_sector_id)
					return None
				
				node = self.nodes[node_id]
				
				if is_on_back_side(node):
					self.render_bsp_node(node.back_child_id)
					if self.check_bbox(node.bbox['front']):
						self.render_bsp_node(node.front_child_id)
				else:
					self.render_bsp_node(node.front_child_id)
					if self.check_bbox(node.bbox['back']):
						self.render_bsp_node(node.back_child_id)
