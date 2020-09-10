init -100 python:
	black_color = 'images/bg/black.jpg'
	def get_invert_and_tint_matrix(color):
		r, g, b, a = renpy.easy.color(color)
		return im.matrix.invert() * im.matrix.tint(r / 255.0, g / 255.0, b / 255.0, a / 255.0)


init -1 python:
	level_class_props = {}
	level_dynamic_objects = []
	
	prev_level = cur_level = -1
	def set_level(level):
		if not level_class_props:
			for symbol, obj_type in level_classes.iteritems():
				level_class_props[symbol] = dir(level_classes[symbol])
		
		global level_dynamic_objects
		level_dynamic_objects = []
		
		global cur_level, level_start_score
		cur_level = level
		level_start_score = score
		
		global level_w, level_h
		level_map = levels[level]
		level_w, level_h = len(level_map[0]) - 1, len(level_map) # 1 is space that ends each line
		
		
		for symbol, props in level_classes.iteritems():
			if 'start' in level_class_props[symbol]:
				level_classes[symbol].start()
		
		global exit_x, exit_y
		exit_x = exit_y = -100
		
		args = [(level_w * cell_size, level_h * cell_size)]
		for y in xrange(level_h):
			line = level_map[y]
			
			for x in xrange(level_w):
				symbol = line[x]
				
				if symbol not in level_classes:
					out_msg('set_level', 'Object <' + symbol + '> not in level_classes')
					obj_type = level_classes[' ']
				else:
					obj_type = level_classes[symbol]
					if 'init' in level_class_props[symbol]:
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
		me.move_kind = 'stay'
	
	
	def check_fail():
		if me.y / cell_size > level_h + 2: # fail
			msg('FAIL', 0xFF4000)
			set_level(cur_level) # restart current level
			
