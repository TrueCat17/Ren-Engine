init python:
	class ViewRenderer:
		def __init__(self, engine):
			self.engine = engine
			self.asset_data = engine.wad_data.asset_data
			self.textures = self.asset_data.textures
			self.x_to_angle = self.engine.seg_handler.x_to_angle
			
			self.sky_id = self.asset_data.sky_id
			self.sky_tex = self.asset_data.sky_tex
			self.sky_inv_scale = 160 / DOOM_H
			self.sky_tex_alt = 100
			
			self.all_flats = mod_dir + 'images/flats.webp'
			self.cropped_flats = {}
			self.flat_crops = {}
			with open(mod_dir + 'images/flats.txt', 'rb') as f:
				for i, flat_name in enumerate(f):
					flat_name = flat_name.decode('utf-8').strip()
					if flat_name:
						self.flat_crops[flat_name] = crop = (i, 0, 1, 1)
						self.cropped_flats[flat_name] = im.crop(self.all_flats, crop)
			
			self.all_flats_recolored = [
				im.recolor(self.all_flats, light_level, light_level, light_level)
					for light_level in range(256)
			]
			
			self.all_textures = mod_dir + 'images/textures.webp'
			self.cropped_textures = {}
			self.texture_crops = {}
			with open(mod_dir + 'images/textures.txt', 'rb') as f:
				for s in f:
					s = s.decode('utf-8').strip()
					if s:
						texture_name, crop = s.split(': ')
						crop = crop.split(', ')
						self.texture_crops[texture_name] = crop = tuple(int(i) for i in crop)
						self.cropped_textures[texture_name] = im.crop(self.all_textures, crop)
			
			self.cache_of_repeats = {}
			self.cache_of_recolors = {}
		
		def get_texture(self, tex):
			res = self.cropped_flats.get(tex)
			if res:
				return res
			
			res = self.cropped_textures.get(tex)
			if res:
				return res
			
			return tex
		
		def draw_flat(self, tex_id, light_level, x, y1, y2, world_z):
			y1 -= 1
			y2 += 1
			if y1 >= y2:
				return
			
			if tex_id == self.sky_id:
				tex_column = 2.2 * (player.angle + self.x_to_angle[x])
				self.draw_wall_col(self.sky_tex, tex_column, x, y1 + 1, y2 - 1, self.sky_tex_alt, self.sky_inv_scale, light_level=1.0)
			else:
				flat_tex = self.textures[tex_id]
				self.draw_flat_col(flat_tex, x, y1, y2, light_level)
		
		def draw_flat_col(self, flat_tex, x, y1, y2, light_level):
			flat_crop = self.flat_crops[flat_tex]
			
			light_level = in_bounds(int(light_level * 255), 0, 255)
			flat_tex = self.all_flats_recolored[light_level]
			
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
			key = (tex, light_level)
			recolored_tex = self.cache_of_recolors.get(key)
			if not recolored_tex:
				recolored_tex = self.cache_of_recolors[key] = im.recolor(tex, light_level, light_level, light_level)
			tex = recolored_tex
			
			ysize = y2 - y1
			tex_ysize = int(inv_scale * ysize)
			
			repeats = math_ceil((tex_y + tex_ysize) / tex_h)
			if repeats <= 0:
				return
			if repeats != 1:
				key = (tex, repeats)
				repeated = self.cache_of_repeats.get(key)
				if not repeated:
					args = [(tex_w, tex_h * repeats)]
					for i in range(repeats):
						args.append((0, i * tex_h))
						args.append(tex)
					repeated = self.cache_of_repeats[key] = im.composite(*args)
				tex = repeated
				tex_h *= repeats
			
			draw_list.append((
				tex,
				(x, y1),
				ysize,
				(tex_x, tex_y, 1, tex_ysize),
			))
