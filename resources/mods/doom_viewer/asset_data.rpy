init python:
	
	class AssetData:
		def __init__(self, wad_data):
			self.wad_data = wad_data
			self.reader = wad_data.reader
			self.get_lump_index = wad_data.get_lump_index
			
			# wall textures
			texture_maps = self.load_texture_maps(texture_lump_name='TEXTURE1')
			if self.get_lump_index('TEXTURE2'):
				texture_maps += self.load_texture_maps(texture_lump_name='TEXTURE2')
			
			self.textures = {}
			for tex_map in texture_maps:
				self.textures[tex_map.name] = tex_map.name
			
			self.textures |= self.get_flats()
			
			self.sky_id = 'F_SKY1'
			self.sky_tex_name = 'SKY1'
			self.sky_tex = self.textures[self.sky_tex_name]
		
		def get_flats(self, start_marker='F_START', end_marker='F_END'):
			idx1 = self.get_lump_index(start_marker) + 1
			idx2 = self.get_lump_index(end_marker)
			flat_lumps = self.reader.directory[idx1: idx2]
			
			flats = {}
			for flat_lump in flat_lumps:
				offset = flat_lump['lump_offset']
				size = flat_lump['lump_size']  # 64 x 64
				
				flat_data = []
				for i in range(size):
					flat_data.append(self.reader.read_1_byte(offset + i, byte_format='B'))
				
				flat_name = flat_lump['lump_name']
				flats[flat_name] = flat_name
			return flats
		
		def load_texture_maps(self, texture_lump_name):
			tex_idx = self.get_lump_index(texture_lump_name)
			offset = self.reader.directory[tex_idx]['lump_offset']
			
			texture_header = self.reader.read_texture_header(offset)
			
			texture_maps = []
			for i in range(texture_header.texture_count):
				tex_map = self.reader.read_texture_map(offset + texture_header.texture_data_offset[i])
				texture_maps.append(tex_map)
			return texture_maps
