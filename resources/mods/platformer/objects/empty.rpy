init -21 python:
	
	class Pltf_Empty:
		can_vertical = False
		can_horizontal = False
		
		is_dynamic = False
		image = None
	
	
	class Pltf_Start(Pltf_Empty):
		@staticmethod
		def init(x, y):
			me.x, me.y = x * cell_size, y * cell_size
	
	
	class Pltf_Exit(Pltf_Empty):
		image = im.composite((cell_size, cell_size),
		                      (0, 0), im.rect('#0A0', cell_size, cell_size),
		                      (cell_size / 4, cell_size / 2), im.rect('#000', cell_size / 4, cell_size / 2))
		
		@staticmethod
		def init(x, y):
			global exit_x, exit_y
			exit_x, exit_y = x, y
		
		@staticmethod
		def physics(cell, pixel, dx, dy):
			global next_level
			next_level = cur_level + 1
			return pixel[0] + dx, pixel[1] + dy

