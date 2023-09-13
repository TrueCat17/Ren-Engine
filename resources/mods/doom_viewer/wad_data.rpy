init python:
	class WADData:
		LUMP_INDICES = {
			'THINGS': 1, 'LINEDEFS': 2, 'SIDEDEFS': 3, 'VERTEXES': 4, 'SEGS': 5,
			'SSECTORS': 6, 'NODES': 7, 'SECTORS': 8, 'REJECT': 9, 'BLOCKMAP': 10
		}
		LINEDEF_FLAGS = {
			'BLOCKING': 1, 'BLOCK_MONSTERS': 2, 'TWO_SIDED': 4, 'DONT_PEG_TOP': 8,
			'DONT_PEG_BOTTOM': 16, 'SECRET': 32, 'SOUND_BLOCK': 64, 'DONT_DRAW': 128, 'MAPPED': 256
		}
		
		def __init__(self, engine, map_name):
			self.reader = WADReader(engine.wad_path)
			self.map_index = self.get_lump_index(lump_name=map_name)
			self.vertexes = self.get_lump_data(
				reader_func=self.reader.read_vertex,
				lump_index=self.map_index + self.LUMP_INDICES['VERTEXES'],
				num_bytes=4  # num bytes per vertex
			)
			self.linedefs = self.get_lump_data(
				reader_func=self.reader.read_linedef,
				lump_index=self.map_index + self.LUMP_INDICES['LINEDEFS'],
				num_bytes=14
			)
			self.nodes = self.get_lump_data(
				reader_func=self.reader.read_node,
				lump_index=self.map_index + self.LUMP_INDICES['NODES'],
				num_bytes=28
			)
			self.sub_sectors = self.get_lump_data(
				reader_func=self.reader.read_sub_sector,
				lump_index=self.map_index + self.LUMP_INDICES['SSECTORS'],
				num_bytes=4
			)
			self.segments = self.get_lump_data(
				reader_func=self.reader.read_segment,
				lump_index=self.map_index + self.LUMP_INDICES['SEGS'],
				num_bytes=12
			)
			self.things = self.get_lump_data(
				reader_func=self.reader.read_thing,
				lump_index=self.map_index + self.LUMP_INDICES['THINGS'],
				num_bytes=10
			)
			self.sidedefs = self.get_lump_data(
				reader_func=self.reader.read_sidedef,
				lump_index=self.map_index + self.LUMP_INDICES['SIDEDEFS'],
				num_bytes=30
			)
			self.sectors = self.get_lump_data(
				reader_func=self.reader.read_sector,
				lump_index=self.map_index + self.LUMP_INDICES['SECTORS'],
				num_bytes=26
			)
			
			self.update_data()
			self.asset_data = AssetData(self)
			self.reader.close()
		
		def update_data(self):
			self.update_linedefs()
			self.update_sidedefs()
			self.update_segs()
		
		def update_sidedefs(self):
			for sidedef in self.sidedefs:
				sidedef.sector = self.sectors[sidedef.sector_id]
		
		def update_linedefs(self):
			for linedef in self.linedefs:
				linedef.front_sidedef = self.sidedefs[linedef.front_sidedef_id]
				
				if linedef.back_sidedef_id == 0xFFFF:  # undefined sidedef
					linedef.back_sidedef = None
				else:
					linedef.back_sidedef = self.sidedefs[linedef.back_sidedef_id]
		
		def update_segs(self):
			for seg in self.segments:
				seg.start_vertex = self.vertexes[seg.start_vertex_id]
				seg.end_vertex = self.vertexes[seg.end_vertex_id]
				seg.linedef = self.linedefs[seg.linedef_id]
				
				if seg.direction:
					front_sidedef = seg.linedef.back_sidedef
					back_sidedef = seg.linedef.front_sidedef
				else:
					front_sidedef = seg.linedef.front_sidedef
					back_sidedef = seg.linedef.back_sidedef
				
				seg.front_sector = front_sidedef.sector
				if self.LINEDEF_FLAGS['TWO_SIDED'] & seg.linedef.flags:
					seg.back_sector = back_sidedef.sector
				else:
					seg.back_sector = None
				
				# convert angles from BAMS to degrees
				seg.angle = (seg.angle << 16) * 8.38190317e-8
				seg.angle = seg.angle + 360 if seg.angle < 0 else seg.angle
				
				# texture special case
				if seg.front_sector and seg.back_sector:
					if front_sidedef.upper_texture == '-':
						seg.linedef.front_sidedef.upper_texture = back_sidedef.upper_texture
					if front_sidedef.lower_texture == '-':
						seg.linedef.front_sidedef.lower_texture = back_sidedef.lower_texture
		
		def get_lump_data(self, reader_func, lump_index, num_bytes, header_length=0):
			lump_info = self.reader.directory[lump_index]
			count = lump_info['lump_size'] // num_bytes
			data = []
			for i in range(count):
				offset = lump_info['lump_offset'] + i * num_bytes + header_length
				data.append(reader_func(offset))
			return data
		
		def get_lump_index(self, lump_name):
			for index, lump_info in enumerate(self.reader.directory):
				if lump_name in lump_info.values():
					return index
			return False
