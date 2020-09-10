init -20 python:
	
	wall_color = '#08F'
	
	class Pltf_Wall:
		can_vertical = False
		can_horizontal = True
		
		is_dynamic = False
		image = im.rect(wall_color, cell_size, cell_size)
		
		@staticmethod
		def physics(cell, pixel, dx, dy):
			return pixel
	
	class Pltf_LeftWall:
		can_vertical = False
		can_horizontal = True
		
		is_dynamic = False
		image = None
		
		@staticmethod
		def init(x, y):
			if Pltf_LeftWall.image is not None:
				return
			
			args = [(cell_size, cell_size)]
			for i in xrange(cell_size):
				args.append((i, cell_size - 1 - i))
				args.append(im.scale(black_color, 1, i + 1))
			Pltf_LeftWall.image = im.matrix_color(im.composite(*args), get_invert_and_tint_matrix(wall_color))
		
		@staticmethod
		def physics(cell, pixel, dx, dy):
			x, y = pixel
			if x == cell_size or y == cell_size:
				return pixel
			free = cell_size - 1 - (x + dx) > y + dy
			if not free:
				if dy > 0:
					return x - 1, y + 1
				if dx > 0:
					return x + 1, y - 1
			return x + dx, y + dy
	
	class Pltf_RightWall:
		can_vertical = False
		can_horizontal = True
		
		is_dynamic = False
		image = None
		
		@staticmethod
		def init(x, y):
			if Pltf_RightWall.image is not None:
				return
			
			args = [(cell_size, cell_size)]
			for i in xrange(cell_size):
				args.append((i, i))
				args.append(im.scale(black_color, 1, cell_size - i))
			Pltf_RightWall.image = im.matrix_color(im.composite(*args), get_invert_and_tint_matrix(wall_color))
		
		@staticmethod
		def physics(cell, pixel, dx, dy):
			x, y = pixel
			x, y = Pltf_LeftWall.physics(cell, (cell_size - 1 - x, y), -dx, dy)
			return cell_size - 1 - x, y
	
	class Pltf_VertexWall:
		can_vertical = False
		can_horizontal = True
		
		is_dynamic = False
		image = None
		
		@staticmethod
		def init(x, y):
			if Pltf_VertexWall.image is not None:
				return
			
			args = [(cell_size, cell_size)]
			for i in xrange(cell_size / 2):
				args.append((i, cell_size - 1 - i))
				args.append(im.scale(black_color, 1, i + 1))
				args.append((i + cell_size / 2, i + cell_size / 2))
				args.append(im.scale(black_color, 1, cell_size / 2 - i))
			Pltf_VertexWall.image = im.matrix_color(im.composite(*args), get_invert_and_tint_matrix(wall_color))
		
		@staticmethod
		def physics(cell, pixel, dx, dy):
			x, y = pixel
			if y == cell_size:
				return pixel
			
			tx, ty = x + dx, y + dy
			if ty >= cell_size / 2:
				if tx < cell_size / 2:
					free = cell_size - 1 - tx > ty
					if not free:
						if dy > 0:
							return x - 1, y
						if dx > 0:
							return x, y - 1
				else:
					free = tx > ty
					if not free:
						if dy > 0:
							return x + 1, y
						if dx < 0:
							return x, y - 1
			return tx, ty

