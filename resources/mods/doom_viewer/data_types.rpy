init python:
	# H - uint16, h - int16, I - uint32, i - int32, c - char
	
	class TextureMap:
		__slots__ = [
			'name',
			'flags',
			'width',
			'height',
			'column_dir',  # unused
			'patch_count',
			'patch_maps',
		]
	
	class PatchMap:
		__slots__ = [
			'x_offset',
			'y_offset',
			'p_name_index',
			'step_dir',  # unused
			'color_map',  # unused
		]
	
	class TextureHeader:
		__slots__ = [
			'texture_count',
			'texture_offset',
			'texture_data_offset',
		]
	
	class PatchColumn:
		__slots__ = [
			'top_delta',  # B
			'length',  # B
			'padding_pre',  # B - unused
			'data',  # length x B
			'padding_post'  # B - unused
		]
	
	
	class PatchHeader:
		__slots__ = [
			'width',  # H
			'height',  # H
			'left_offset',  # h
			'top_offset',  # h
			'column_offset'  # width x I
		]
	
	
	class Thing:
		# 10 bytes
		__slots__ = [
			'pos',  # pos.x, pos.y - 4h
			'angle',  # 2H
			'type',  # 2H
			'flags'  # 2H
		]
	
	
	class Sector:
		# 26 bytes = 2h + 2h + 8c + 8c + 2H x 3
		__slots__ = [
			'floor_height',
			'ceil_height',
			'floor_texture',
			'ceil_texture',
			'light_level',
			'type',
			'tag'
		]
	
	
	class Sidedef:
		# 30 bytes = 2h + 2h + 8c + 8c + 8c + 2H
		__slots__ = [
			'x_offset',
			'y_offset',
			'upper_texture',
			'lower_texture',
			'middle_texture',
			'sector_id',
		]
		__slots__ += ['sector']
	
	
	class Seg:
		# 12 bytes = 2h x 6
		__slots__ = [
			'start_vertex_id',
			'end_vertex_id',
			'angle',
			'linedef_id',
			'direction',
			'offset',
		]
		__slots__ += ['start_vertex', 'end_vertex', 'linedef', 'front_sector', 'back_sector']
	
	
	class Linedef:
		# 14 bytes = 2H x 7
		__slots__ = [
			'start_vertex_id',
			'end_vertex_id',
			'flags',
			'line_type',
			'sector_tag',
			'front_sidedef_id',
			'back_sidedef_id'
		]
		__slots__ += ['front_sidedef', 'back_sidedef']
	
	
	class SubSector:
		# 4 bytes = 2h + 2h
		__slots__ = [
			'seg_count',
			'first_seg_id'
		]
	
	
	class Node:
		# 28 bytes = 2h x 12 + 2H x 2
		
		class BBox:
			__slots__ = ['top', 'bottom', 'left', 'right']
		
		__slots__ = [
			'x_partition',
			'y_partition',
			'dx_partition',
			'dy_partition',
			'bbox',  # 8h
			'front_child_id',
			'back_child_id',
		]
		def __init__(self):
			self.bbox = {'front': self.BBox(), 'back': self.BBox()}
