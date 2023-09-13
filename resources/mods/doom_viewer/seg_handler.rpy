init python:
	class SegHandler:
		MAX_SCALE = 64.0
		MIN_SCALE = 0.00390625
		
		def __init__(self, engine):
			self.engine = engine
			self.wad_data = engine.wad_data
			self.player = engine.player
			self.textures = self.wad_data.asset_data.textures
			self.sky_id = self.wad_data.asset_data.sky_id
			
			self.seg = None
			self.rw_angle1 = None
			self.screen_range: set = None
			self.x_to_angle = self.get_x_to_angle_table()
			self.upper_clip, self.lower_clip = [], []
		
		def update(self):
			self.init_floor_ceil_clip_height()
			self.init_screen_range()
		
		def init_floor_ceil_clip_height(self):
			self.upper_clip = [-1] * DOOM_W
			self.lower_clip = [DOOM_H] * DOOM_W
		
		@staticmethod
		def get_x_to_angle_table():
			x_to_angle = []
			for i in range(DOOM_W + 1):
				angle = math.degrees(math.atan((H_WIDTH - i) / SCREEN_DIST))
				x_to_angle.append(angle)
			return x_to_angle
		
		def scale_from_global_angle(self, x, rw_normal_angle, rw_distance):
			x_angle = self.x_to_angle[x]
			num = SCREEN_DIST * math.cos(math.radians(rw_normal_angle - x_angle - self.player.angle))
			den = rw_distance * math.cos(math.radians(x_angle))
			
			scale = num / den
			return in_bounds(scale, self.MIN_SCALE, self.MAX_SCALE)
		
		def init_screen_range(self):
			self.screen_range = set(range(DOOM_W))
		
		def draw_solid_wall_range(self, x1, x2):
			# some aliases to shorten the following code
			seg = self.seg
			front_sector = seg.front_sector
			line = seg.linedef
			side = line.front_sidedef
			renderer = self.engine.view_renderer
			upper_clip = self.upper_clip
			lower_clip = self.lower_clip
			
			# textures
			wall_texture_id = side.middle_texture
			ceil_texture_id = front_sector.ceil_texture
			floor_texture_id = front_sector.floor_texture
			light_level = front_sector.light_level
			
			# calculate the relative plane heights of front sector
			world_front_z1 = front_sector.ceil_height - self.player.height
			world_front_z2 = front_sector.floor_height - self.player.height
			
			# check which parts must be rendered
			b_draw_wall = side.middle_texture != '-'
			b_draw_ceil = world_front_z1 > 0 or front_sector.ceil_texture == self.sky_id
			b_draw_floor = world_front_z2 < 0
			
			# calculate the scaling factors of the left and right edges of the wall range
			rw_normal_angle = seg.angle + 90
			offset_angle = rw_normal_angle - self.rw_angle1
			
			hypotenuse = math.dist(self.player.pos, seg.start_vertex)
			rw_distance = hypotenuse * math.cos(math.radians(offset_angle))
			
			rw_scale1 = self.scale_from_global_angle(x1, rw_normal_angle, rw_distance)
			
			# lol try to fix the stretched line bug
			if math.isclose(offset_angle % 360, 90, abs_tol=1):
				rw_scale1 *= 0.01
			
			if x1 < x2:
				scale2 = self.scale_from_global_angle(x2, rw_normal_angle, rw_distance)
				rw_scale_step = (scale2 - rw_scale1) / (x2 - x1)
			else:
				rw_scale_step = 0
			
			# -------------------------------------------------------------------------- #
			# determine how the wall texture are vertically aligned
			wall_texture = renderer.get_texture(self.textures[wall_texture_id])
			if line.flags & self.wad_data.LINEDEF_FLAGS['DONT_PEG_BOTTOM']:
				v_top = front_sector.floor_height + get_image_height(wall_texture)
				middle_tex_alt = v_top - self.player.height
			else:
				middle_tex_alt = world_front_z1
			middle_tex_alt += side.y_offset
			
			# determine how the wall textures are horizontally aligned
			rw_offset = hypotenuse * math.sin(math.radians(offset_angle))
			rw_offset += seg.offset + side.x_offset
			#
			rw_center_angle = rw_normal_angle - self.player.angle
			# -------------------------------------------------------------------------- #
			
			# determine where on the screen the wall is drawn
			wall_y1 = H_HEIGHT - world_front_z1 * rw_scale1
			wall_y1_step = -rw_scale_step * world_front_z1
			
			wall_y2 = H_HEIGHT - world_front_z2 * rw_scale1
			wall_y2_step = -rw_scale_step * world_front_z2
			
			# now the rendering is carried out
			for x in range(x1, x2 + 1):
				draw_wall_y1 = wall_y1 - 1
				draw_wall_y2 = wall_y2
				
				if b_draw_ceil:
					cy1 = upper_clip[x] + 1
					cy2 = int(min(draw_wall_y1 - 1, lower_clip[x] - 1))
					renderer.draw_flat(ceil_texture_id, light_level, x, cy1, cy2, world_front_z1)
				
				if b_draw_wall:
					wy1 = int(max(draw_wall_y1, upper_clip[x] + 1))
					wy2 = int(min(draw_wall_y2, lower_clip[x] - 1))
					
					if wy1 < wy2:
						angle = rw_center_angle - self.x_to_angle[x]
						texture_column = rw_distance * math.tan(math.radians(angle)) - rw_offset
						inv_scale = 1.0 / rw_scale1
						
						renderer.draw_wall_col(wall_texture, texture_column, x, wy1, wy2, middle_tex_alt, inv_scale, light_level)
				
				if b_draw_floor:
					fy1 = int(max(draw_wall_y2 + 1, upper_clip[x] + 1))
					fy2 = lower_clip[x] - 1
					renderer.draw_flat(floor_texture_id, light_level, x, fy1, fy2, world_front_z2)
				
				rw_scale1 += rw_scale_step
				wall_y1 += wall_y1_step
				wall_y2 += wall_y2_step
		
		def draw_portal_wall_range(self, x1, x2):
			# some aliases to shorten the following code
			seg = self.seg
			front_sector = seg.front_sector
			back_sector = seg.back_sector
			line = seg.linedef
			side = line.front_sidedef
			renderer = self.engine.view_renderer
			upper_clip = self.upper_clip
			lower_clip = self.lower_clip
			
			# textures
			upper_wall_texture = side.upper_texture
			lower_wall_texture = side.lower_texture
			tex_ceil_id = front_sector.ceil_texture
			tex_floor_id = front_sector.floor_texture
			light_level = front_sector.light_level
			
			# calculate the relative plane heights of front and back sector
			world_front_z1 = front_sector.ceil_height - self.player.height
			world_back_z1 = back_sector.ceil_height - self.player.height
			world_front_z2 = front_sector.floor_height - self.player.height
			world_back_z2 = back_sector.floor_height - self.player.height
			
			# sky hack
			if front_sector.ceil_texture == back_sector.ceil_texture == self.sky_id:
				world_front_z1 = world_back_z1
			
			# check which parts must be rendered
			if (world_front_z1 != world_back_z1 or
					front_sector.light_level != back_sector.light_level or
					front_sector.ceil_texture != back_sector.ceil_texture):
				b_draw_upper_wall = side.upper_texture != '-' and world_back_z1 < world_front_z1
				b_draw_ceil = world_front_z1 >= 0 or front_sector.ceil_texture == self.sky_id
			else:
				b_draw_upper_wall = False
				b_draw_ceil = False
			
			if (world_front_z2 != world_back_z2 or
					front_sector.floor_texture != back_sector.floor_texture or
					front_sector.light_level != back_sector.light_level):
				b_draw_lower_wall = side.lower_texture != '-' and world_back_z2 > world_front_z2
				b_draw_floor = world_front_z2 <= 0
			else:
				b_draw_lower_wall = False
				b_draw_floor = False
			
			# if nothing must be rendered, we can skip this seg
			if not b_draw_upper_wall and not b_draw_ceil and not b_draw_lower_wall and not b_draw_floor:
				return
			
			# calculate the scaling factors of the left and right edges of the wall range
			rw_normal_angle = seg.angle + 90
			offset_angle = rw_normal_angle - self.rw_angle1
			
			hypotenuse = math.dist(self.player.pos, seg.start_vertex)
			rw_distance = hypotenuse * math.cos(math.radians(offset_angle))
			
			rw_scale1 = self.scale_from_global_angle(x1, rw_normal_angle, rw_distance)
			if x2 > x1:
				scale2 = self.scale_from_global_angle(x2, rw_normal_angle, rw_distance)
				rw_scale_step = (scale2 - rw_scale1) / (x2 - x1)
			else:
				rw_scale_step = 0
			
			# determine how the wall textures are vertically aligned
			if b_draw_upper_wall:
				upper_wall_texture = renderer.get_texture(self.textures[side.upper_texture])
				
				if line.flags & self.wad_data.LINEDEF_FLAGS['DONT_PEG_TOP']:
					upper_tex_alt = world_front_z1
				else:
					v_top = back_sector.ceil_height + get_image_height(upper_wall_texture)
					upper_tex_alt = v_top - self.player.height
				upper_tex_alt += side.y_offset
			
			if b_draw_lower_wall:
				lower_wall_texture = self.textures[side.lower_texture]
				
				if line.flags & self.wad_data.LINEDEF_FLAGS['DONT_PEG_BOTTOM']:
					lower_tex_alt = world_front_z1
				else:
					lower_tex_alt = world_back_z2
				lower_tex_alt += side.y_offset
			
			# determine how the wall textures are horizontally aligned
			if seg_textured:= b_draw_upper_wall or b_draw_lower_wall:
				rw_offset = hypotenuse * math.sin(math.radians(offset_angle))
				rw_offset += seg.offset + side.x_offset
				#
				rw_center_angle = rw_normal_angle - self.player.angle
			
			# the y positions of the top / bottom edges of the wall on the screen
			wall_y1 = H_HEIGHT - world_front_z1 * rw_scale1
			wall_y1_step = -rw_scale_step * world_front_z1
			wall_y2 = H_HEIGHT - world_front_z2 * rw_scale1
			wall_y2_step = -rw_scale_step * world_front_z2
			
			# the y position of the top edge of the portal
			if b_draw_upper_wall:
				if world_back_z1 > world_front_z2:
					portal_y1 = H_HEIGHT - world_back_z1 * rw_scale1
					portal_y1_step = -rw_scale_step * world_back_z1
				else:
					portal_y1 = wall_y2
					portal_y1_step = wall_y2_step
			
			if b_draw_lower_wall:
				if world_back_z2 < world_front_z1:
					portal_y2 = H_HEIGHT - world_back_z2 * rw_scale1
					portal_y2_step = -rw_scale_step * world_back_z2
				else:
					portal_y2 = wall_y1
					portal_y2_step = wall_y1_step
			
			# now the rendering is carried out
			for x in range(x1, x2 + 1):
				draw_wall_y1 = wall_y1 - 1
				draw_wall_y2 = wall_y2
				
				if seg_textured:
					angle = rw_center_angle - self.x_to_angle[x]
					texture_column = rw_distance * math.tan(math.radians(angle)) - rw_offset
					inv_scale = 1.0 / rw_scale1
				
				if b_draw_upper_wall:
					draw_upper_wall_y1 = wall_y1 - 1
					draw_upper_wall_y2 = portal_y1
					
					if b_draw_ceil:
						cy1 = upper_clip[x] + 1
						cy2 = int(min(draw_wall_y1 - 1, lower_clip[x] - 1))
						renderer.draw_flat(tex_ceil_id, light_level, x, cy1, cy2, world_front_z1)
					
					wy1 = int(max(draw_upper_wall_y1, upper_clip[x] + 1))
					wy2 = int(min(draw_upper_wall_y2, lower_clip[x] - 1))
					
					renderer.draw_wall_col(upper_wall_texture, texture_column, x, wy1, wy2, upper_tex_alt, inv_scale, light_level)
					
					if upper_clip[x] < wy2:
						upper_clip[x] = wy2
					
					portal_y1 += portal_y1_step
				
				if b_draw_ceil:
					cy1 = upper_clip[x] + 1
					cy2 = int(min(draw_wall_y1 - 1, lower_clip[x] - 1))
					renderer.draw_flat(tex_ceil_id, light_level, x, cy1, cy2, world_front_z1)
					
					if upper_clip[x] < cy2:
						upper_clip[x] = cy2
				
				if b_draw_lower_wall:
					
					if b_draw_floor:
						fy1 = int(max(draw_wall_y2 + 1, upper_clip[x] + 1))
						fy2 = lower_clip[x] - 1
						renderer.draw_flat(tex_floor_id, light_level, x, fy1, fy2, world_front_z2)
					
					draw_lower_wall_y1 = portal_y2 - 1
					draw_lower_wall_y2 = wall_y2
					
					wy1 = int(max(draw_lower_wall_y1, upper_clip[x] + 1))
					wy2 = int(min(draw_lower_wall_y2, lower_clip[x] - 1))
					#
					renderer.draw_wall_col(lower_wall_texture, texture_column, x, wy1, wy2, lower_tex_alt, inv_scale, light_level)
					
					if lower_clip[x] > wy1:
						lower_clip[x] = wy1
					
					portal_y2 += portal_y2_step
				
				if b_draw_floor:
					
					fy1 = int(max(draw_wall_y2 + 1, upper_clip[x] + 1))
					fy2 = lower_clip[x] - 1
					renderer.draw_flat(tex_floor_id, light_level, x, fy1, fy2, world_front_z2)
					
					if lower_clip[x] > draw_wall_y2 + 1:
						lower_clip[x] = fy1
				
				rw_scale1 += rw_scale_step
				wall_y1 += wall_y1_step
				wall_y2 += wall_y2_step
		
		def clip_portal_walls(self, x_start, x_end):
			curr_wall = set(range(x_start, x_end))
			if intersection := curr_wall & self.screen_range:
				if len(intersection) == len(curr_wall):
					self.draw_portal_wall_range(x_start, x_end - 1)
				else:
					arr = sorted(intersection)
					x = arr[0]
					for x1, x2 in zip(arr, arr[1:]):
						if x2 - x1 > 1:
							self.draw_portal_wall_range(x, x1)
							x = x2
					self.draw_portal_wall_range(x, arr[-1])
		
		def clip_solid_walls(self, x_start, x_end):
			if self.screen_range:
				curr_wall = set(range(x_start, x_end))
				if intersection := curr_wall & self.screen_range:
					if len(intersection) == len(curr_wall):
						self.draw_solid_wall_range(x_start, x_end - 1)
					else:
						arr = sorted(intersection)
						x, x2 = arr[0], arr[-1]
						for x1, x2 in zip(arr, arr[1:]):
							if x2 - x1 > 1:
								self.draw_solid_wall_range(x, x1)
								x = x2
						self.draw_solid_wall_range(x, x2)
					self.screen_range -= intersection
			else:
				self.engine.bsp.is_traverse_bsp = False
		
		def classify_segment(self, segment, x1, x2, rw_angle1):
			# add seg data
			self.seg = segment
			self.rw_angle1 = rw_angle1
			
			# does not cross a pixel?
			if x1 == x2:
				return
			
			back_sector = segment.back_sector
			front_sector = segment.front_sector
			
			# handle solid walls
			if back_sector is None:
				self.clip_solid_walls(x1, x2)
				return
			
			# wall with window
			if (front_sector.ceil_height, front_sector.floor_height) != (back_sector.ceil_height, back_sector.floor_height):
				self.clip_portal_walls(x1, x2)
				return
			
			# reject empty lines used for triggers and special events.
			# identical floor and ceiling on both sides, identical
			# light levels on both sides, and no middle texture.
			if (back_sector.ceil_texture == front_sector.ceil_texture and
					back_sector.floor_texture == front_sector.floor_texture and
					back_sector.light_level == front_sector.light_level and
					self.seg.linedef.front_sidedef.middle_texture == '-'):
				return
			
			# borders with different light levels and textures
			self.clip_portal_walls(x1, x2)
