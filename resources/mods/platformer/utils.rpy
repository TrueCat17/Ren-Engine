init -100 python:
	def get_invert_and_tint_matrix(color):
		r, g, b, a = renpy.easy.color(color)
		return im.matrix.invert() * im.matrix.tint(r / 255.0, g / 255.0, b / 255.0, a / 255.0)


init -1 python:
	level_dynamic_objects = []
	
	prev_level = cur_level = -1
	def set_level(level):
		level_dynamic_objects.clear()
		
		global cur_level
		cur_level = level
		
		global level_w, level_h
		level_map = levels[level]
		level_w, level_h = len(level_map[0]) - 1, len(level_map) # 1 - each line ends with space
		
		
		for cls in level_classes.values():
			if hasattr(cls, 'start'):
				cls.start()
		
		global exit_x, exit_y
		exit_x = exit_y = -100
		
		args = [(level_w * cell_size, level_h * cell_size)]
		for y, line in enumerate(level_map):
			for x, symbol in enumerate(line):
				if symbol not in level_classes:
					out_msg('set_level', 'Object <%s> not in level_classes', symbol)
					continue
				
				obj_type = level_classes[symbol]
				if hasattr(obj_type, 'init'):
					obj_type.init(x, y)
				
				if obj_type.image is not None:
					if obj_type.is_dynamic:
						level_dynamic_objects.append((obj_type, x, y))
					else:
						args += [(x * cell_size, y * cell_size), obj_type.image]
		
		global level_image
		level_image = im.composite(*args)
		
		global prev_level
		prev_level = cur_level
		
		global me_vx, me_vy
		me_vx = me_vy = 0
		me.set_direction(to_back)
	
	
	def check_fail():
		if me.y / cell_size > level_h + 2: # fail
			msg('FAIL', '#F40')
			set_level(cur_level) # restart current level
