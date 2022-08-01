init -1000000 python:
	alphabet = list(map(chr, xrange(ord('a'), ord('z') + 1))) # a-z
	numbers = range(10) # 0-9
	
	
	def build_object(name):
		"""
		name__prop -> name.prop
		name__sub1__sub2__prop -> name.sub1.sub2.prop
		"""
		g = globals()
		
		start_str = name + '__'
		for prop_name in g.keys():
			if not prop_name.startswith(start_str): continue
			orig_name = prop_name
			
			subs = prop_name.split('__')
			prop_name, subs = subs[-1], subs[:-1]
			
			tmp_obj = g
			for sub in subs:
				if not tmp_obj.has_key(sub):
					tmp_obj[sub] = Object()
				tmp_obj = tmp_obj[sub]
			
			tmp_obj[prop_name] = g[orig_name]
	
	
	def get_file_and_line(depth):
		frame = sys._getframe()
		frame = frame.f_back
		while depth:
			depth -= 1
			frame = frame.f_back
		return frame.f_code.co_filename, frame.f_lineno
	
	def get_filename(depth):
		return get_file_and_line(depth + 1)[0]
	def get_numline(depth):
		return get_file_and_line(depth + 1)[1]
	
	def get_stack(depth):
		stack = traceback.format_stack()
		return stack[:-(depth + 1)]
	
	
	def quick_load():
		path = os.path.join(slots.directory, 'quick', '1', 'py_globals')
		if os.path.exists(path):
			slots.load('1', page='quick')
	def quick_save():
		if get_current_mod() != 'main_menu':
			slots.save('1', page='quick')
	
	def show_screen(name, *args, **kwargs):
		return _show_screen(name, args, kwargs)
	
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
	def sign(x):
		return -1 if x < 0 else 1 if x > 0 else 0
	
	def get_image_size(image):
		return get_image_width(image), get_image_height(image)
	def get_stage_size():
		return get_stage_width(), get_stage_height()
	def toggle_fullscreen():
		set_fullscreen(not get_from_hard_config('window_fullscreen', bool))
	
	def get_from_hard_config(param, ret_type):
		res = _get_from_hard_config(str(param))
		if ret_type is bool:
			return res == "True"
		return ret_type(res)
	
	def out_msg(msg, err = ''):
		err = str(err)
		exc_type, exc_value, exc_traceback = sys.exc_info()
		if exc_traceback:
			stack = traceback.format_tb(exc_traceback)
			err += '\n\nException (type=' + type(exc_value).__name__ + '): ' + str(exc_value)
		else:
			stack = get_stack(1)
		err += '\n\nStack:\n'
		for frame in stack:
			err += frame
		
		_out_msg(str(msg), err)
	
	
	def get_k_between(start, end, value, reverse = False):
		"""
		0, 100, 70, True  -> 0.7
		10, 20, 18, False -> 0.2
		"""
		if start == end:
			return 0 if reverse else 1
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
	
	
	def color_to_int(color, alpha = False):
		if color is None or type(color) is int:
			return color
		
		r, g, b, a = [in_bounds(c, 0, 255) for c in renpy.easy.color(color)]
		if alpha:
			return r << 24 | g << 16 | b << 8 | a
		return r << 16 | g << 8 | b
	
	
	def get_md5(s):
		import hashlib
		md5 = hashlib.md5()
		md5.update(s)
		return md5.hexdigest()
	
	
	def get_glob_labels(name):
		if renpy.has_label(name):
			return [name]
		
		cache = get_glob_labels.__dict__
		regexps = cache.get('regexps', None)
		if regexps is None:
			regexps = cache['regexps'] = []
			
			labels = [label for label in renpy.get_all_labels() if ('*' in label) or ('?' in label)]
			labels.sort(key = lambda s: (-len(s), s))
			
			import re
			for label in labels:
				regexp_text = re.escape(label).replace(r'\?', '.').replace(r'\*', '.*?')
				regexp = re.compile('^' + regexp_text + '$')
				regexps.append((label, regexp))
		
		return [label for (label, regexp) in regexps if regexp.match(name)]
	
	def interpolate_tag(tag, kwargs, in_recursion = False):
		if type(tag) is not str or len(tag) < 3 or tag[0] != '[' or tag[-1] != ']':
			out_msg('interpolate_tag', 'invalid tag <' + str(tag) + '>')
			return tag
		tag = tag[1:-1]
		
		ops = ''
		i = tag.find('!')
		if i != -1:
			ops = tag[i+1:]
			tag = tag[:i]
		
		for op in ops:
			if op not in 'tiulc':
				out_msg('interpolate_tag', 'invalid tag operator <' + op + '> in tag <' + tag + '>')
		
		tag = str(eval(tag, kwargs, {}))
		
		if 't' in ops:
			tag = _(tag)
			
		if 'i' in ops and not in_recursion:
			tag = interpolate_tags(tag, kwargs, in_recursion = True)
		
		if 'q' in ops:
			tag = tag.replace('{', '{{')
		if 'u' in ops:
			tag = tag.upper()
		if 'l' in ops:
			tag = tag.lower()
		if 'c' in ops:
			tag = tag.title()
		return tag
	
	def interpolate_tags(text, kwargs = None, in_recursion = False):
		if kwargs is None:
			kwargs = globals()
		
		i = 0
		start = end = None
		opened = 0
		while i < len(text):
			c = text[i]
			
			if c == '[':
				if i < len(text) - 1 and text[i + 1] == '[':
					text = text[:i] + text[i+1:]
					i += 1
					continue
				else:
					if opened == 0:
						start = i
					opened += 1
			elif c == ']':
				opened -= 1
				if opened == 0:
					end = i + 1
					tag = text[start:end]
					defined = interpolate_tag(tag, kwargs)
					text = text[:start] + defined + text[end:]
					i += len(defined) - len(tag)
				elif opened < 0:
					opened = 0
			
			i += 1
		
		return text


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

