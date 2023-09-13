init python:
	class ViewRenderer:
		def __init__(self, engine):
			self.engine = engine
			self.asset_data = engine.wad_data.asset_data
			self.textures = self.asset_data.textures
			self.player = engine.player
			self.x_to_angle = self.engine.seg_handler.x_to_angle
			
			self.sky_id = self.asset_data.sky_id
			self.sky_tex = self.asset_data.sky_tex
			self.sky_inv_scale = 160 / DOOM_H
			self.sky_tex_alt = 100
			
			self.all_flats = mod_dir + 'images/flats.webp'
			self.flat_crops = {}
			for i, flat_name in enumerate(open(mod_dir + 'images/flats.txt', 'r')):
				flat_name = flat_name.strip()
				if flat_name:
					self.flat_crops[flat_name] = (i, 0, 1, 1)
			
			self.all_textures = mod_dir + 'images/textures.webp'
			self.texture_crops = {}
			for s in open(mod_dir + 'images/textures.txt', 'r'):
				s = s.strip()
				if s:
					texture_name, crop = s.split(': ')
					crop = crop.split(', ')
					self.texture_crops[texture_name] = tuple(int(i) for i in crop)
			
			self.repeats = {}
		
		def get_texture(self, tex):
			if tex in self.flat_crops:
				flat_crop = self.flat_crops[tex]
				tex = im.crop(self.all_flats, flat_crop)
			elif tex in self.texture_crops:
				tex_crop = self.texture_crops[tex]
				tex = im.crop(self.all_textures, tex_crop)
			return tex
		
		def draw_flat(self, tex_id, light_level, x, y1, y2, world_z):
			y1 -= 1
			y2 += 1
			if y1 >= y2:
				return
			
			if tex_id == self.sky_id:
				tex_column = 2.2 * (self.player.angle + self.x_to_angle[x])
				self.draw_wall_col(self.sky_tex, tex_column, x, y1 + 1, y2 - 1, self.sky_tex_alt, self.sky_inv_scale, light_level=1.0)
			else:
				flat_tex = self.textures[tex_id]
				self.draw_flat_col(flat_tex, x, y1, y2, light_level,
				                   world_z, self.player.angle, self.player.pos[0], self.player.pos[1])
		
		def draw_flat_col(self, flat_tex, x, y1, y2, light_level, world_z, player_angle, player_x, player_y):
			flat_crop = self.flat_crops[flat_tex]
			
			light_level = in_bounds(int(light_level * 255), 0, 255)
			flat_tex = im.recolor(self.all_flats, light_level, light_level, light_level)
			
			draw_list.append((
				flat_tex,
				(x, y1),
				y2 - y1,
				flat_crop,
			))
		
		def draw_wall_col(self, tex, tex_x, x, y1, y2, tex_alt, inv_scale, light_level):
			y1 -= 1
			y2 += 1
			if y1 >= y2:
				return
			
			tex = self.get_texture(tex)
			tex_w, tex_h = get_image_size(tex)
			tex_x = int(tex_x) % tex_w
			tex_y = int(tex_alt + (y1 - H_HEIGHT) * inv_scale) % tex_h
			
			light_level = in_bounds(int(light_level * 255), 0, 255)
			tex = im.recolor(tex, light_level, light_level, light_level)
			
			ysize = y2 - y1
			tex_ysize = int(inv_scale * ysize)
			
			repeats = int(math.ceil((tex_y + tex_ysize) / tex_h))
			if repeats <= 0:
				return
			if repeats != 1:
				key = (tex, repeats)
				if key not in self.repeats:
					args = [(tex_w, tex_h * repeats)]
					for i in range(repeats):
						args.append((0, i * tex_h))
						args.append(tex)
					self.repeats[key] = im.composite(*args)
				tex = self.repeats[key]
				tex_h *= repeats
			
			draw_list.append((
				tex,
				(x, y1),
				ysize,
				(tex_x, tex_y, 1, tex_ysize),
			))
