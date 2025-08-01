init -20 python:
	
	ladder_color = '#F80'
	
	class Pltf_Ladder:
		can_vertical = True
		can_horizontal = True
		
		is_dynamic = False
		image = None
		
		@staticmethod
		def init(x, y):
			if Pltf_Ladder.image is not None:
				return
			
			main_size = cell_size / 4
			main = im.rect('#000', main_size, cell_size) # vertical line
			
			size = cell_size / 8
			count = 4
			width = cell_size - 2 * main_size
			extra = im.rect('#000', width, size) # horizontal line
			
			args = [(cell_size, cell_size)]
			args += [(0, 0),                     main]
			args += [(cell_size - main_size, 0), main]
			
			step = cell_size / count
			for i in range(count):
				args.append((main_size, round(step * i + size / 2)))
				args.append(extra)
			
			Pltf_Ladder.image = im.matrix_color(im.composite(*args), get_invert_and_tint_matrix(ladder_color))
