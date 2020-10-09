init -10000 python:
	gui = 'images/gui/'
	
	alphabet = list(map(chr, xrange(ord('a'), ord('z') + 1))) # a-z
	numbers = range(10) # 0-9


init -998 python:
	checkboxes_inited = False
	def init_checkboxes():
		global checkboxes_inited, checkbox_yes, checkbox_no
		checkboxes_inited = True
		
		checkbox_yes = get_back_with_color(gui + 'std/checkbox/yes.png')
		checkbox_no  = get_back_with_color(gui + 'std/checkbox/no.png')
	
	
	bar_ground = gui + 'std/bar/ground.png'
	bar_hover  = gui + 'std/bar/hover.png'
	
	vbar_ground = im.Rotozoom(bar_ground, 90, 1)
	vbar_hover  = im.Rotozoom(bar_hover , 90, 1)


init -1000000 python:
	def get_numline(depth):
		frame = sys._getframe()
		while depth:
			depth -= 1
			frame = frame.f_back
		return frame.f_lineno
	def get_filename(depth):
		frame = sys._getframe()
		while depth:
			depth -= 1
			frame = frame.f_back
		return frame.f_code.co_filename
	def get_file_and_line(depth):
		frame = sys._getframe()
		while depth:
			depth -= 1
			frame = frame.f_back
		return frame.f_code.co_filename, frame.f_lineno
	
	def get_stack(depth):
		stack = traceback.format_stack()
		return stack[:-(depth + 1)]


init -100000 python:
	def quick_load():
		path = os.path.join(save_dir, config.quick_save_table, config.quick_save_name, 'py_globals')
		if os.path.exists(path):
			load(config.quick_save_table, config.quick_save_name)
	def quick_save():
		sl_save(config.quick_save_table, config.quick_save_name)
	
	def make_screenshot(width = None, height = None):
		global need_screenshot, screenshot_width, screenshot_height
		need_screenshot = True
		screenshot_width, screenshot_height = width or get_stage_width(), height or get_stage_height()
	
	
	def in_bounds(v, vmin, vmax):
		return vmin if v < vmin else vmax if v > vmax else v
	def get_absolute(value, max_value):
		if type(value) is float:
			return absolute(value * max_value)
		return absolute(value)
	def get_dist(x1, y1, x2, y2):
		dx, dy = x1 - x2, y1 - y2
		return math.sqrt(dx*dx + dy*dy)
	
	def get_image_size(image):
		return get_image_width(image), get_image_height(image)
	def get_stage_size():
		return get_stage_width(), get_stage_height()
	
	def get_from_hard_config(param, ret_type):
		res = _get_from_hard_config(str(param))
		if ret_type is bool:
			return res == "True"
		return ret_type(res)
	
	def load(table, num):
		_load(str(table), str(num))
	
	def out_msg(msg, err = ''):
		stack = get_stack(1)
		err = str(err) + '\n\nStack:\n'
		for frame in stack:
			err += frame
		
		_out_msg(str(msg), err)
	
	
	def get_k_between(start, end, value, reverse = False):
		"""
		0, 100, 70, True  -> 0.7
		10, 20, 18, False -> 0.2
		"""
		r = float(value - start) / (end - start)
		return in_bounds(1 - r if reverse else r, 0.0, 1.0)
	
	def interpolate(start, end, k, reverse = False):
		"""
		0, 20, 0.5, False -> 10
		10, 20, 0.7, True -> 13
		"""
		t = float if type(start) is float or type(end) is float else int
		return t(start + (end - start) * (1 - k if reverse else k))
	
	
	def rects_intersects(ax, ay, aw, ah, bx, by, bw, bh):
		if ax + aw < bx: return False
		if ay + ah < by: return False
		if bx + bw < ax: return False
		if by + bh < ay: return False
		return True
	
	
	def get_md5(s):
		import hashlib
		md5 = hashlib.md5()
		md5.update(s)
		return md5.hexdigest()


init -1000000 python:
	can_exec_next_check_funcs = []
	can_exec_next_skip_funcs = []
	
	def can_exec_next_command():
		for func in can_exec_next_check_funcs:
			if not func():
				return False
		return True
	
	def skip_exec_current_command():
		for func in can_exec_next_skip_funcs:
			func()

