init -1000001 python:
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
	
	def make_sure_dir(path):
		if path.endswith('/'):
			return path
		return path + '/'
	
	# <path> relative game root dir
	def get_directory_name(path):
		path = make_sure_dir(path)
		
		cache = get_directory_name.__dict__
		if path in cache:
			return cache[path]
		
		if path == '/':
			name = get_from_hard_config('window_title', str)
			cache[path] = name
			return name
		
		name_path = get_root_dir() + path + 'name'
		if os.path.exists(name_path):
			name = None
			with open(name_path, 'rb') as f:
				for s in f:
					s = s.decode('utf-8').strip()
					if name is None:
						name = s
					else:
						if s.count('=') == 1:
							lang, translated_name = s.split('=')
							if lang == config.language:
								name = translated_name
								break
		else:
			name = path
		
		cache[path] = name
		return name
	
	def get_root_dir():
		cache = get_root_dir.__dict__
		path = cache.get('path')
		if path:
			return path
		
		path = os.path.abspath(get_filename(0)).split('/')
		path = path[:-3] # 3 = len([paths.rpy, rpy, Ren-Engine])
		cache['path'] = '/'.join(path) + '/'
		return cache['path']
	
	
	def get_stack(depth):
		stack = traceback.format_stack()
		return stack[:-(depth + 1)]
	
	
	def get_exception_stack_str(e, depth):
		if isinstance(e, SyntaxError):
			msg = '%s (%s, line %s)' % (e.msg, e.filename, e.lineno)
			text = getattr(e, 'text', None)
			if text:
				if '\n' in text:
					text = text[:text.index('\n')]
				msg += '\n  ' + text
				if e.offset:
					end_offset = getattr(e, 'end_offset', len(text))
					msg += '\n  ' + ' ' * (e.offset - 1) + '^' * max(1, end_offset - e.offset)
		else:
			exc_type, exc_value, exc_traceback = sys.exc_info()
			msg = 'Exception (type = ' + type(exc_value).__name__ + '): ' + str(exc_value)
			if exc_traceback:
				stack = traceback.format_tb(exc_traceback)[depth:]
				if stack:
					msg += '\nStack:\n'
					for frame in stack:
						msg += frame
		return msg
	
	def out_msg(msg, err = '', show_stack = True):
		err = str(err)
		if show_stack:
			exc_type, exc_value, exc_traceback = sys.exc_info()
			if exc_traceback:
				stack = traceback.format_tb(exc_traceback)
				err += '\n\nException (type = ' + type(exc_value).__name__ + '): ' + str(exc_value)
			else:
				stack = get_stack(1)
			err += '\n\nStack:\n'
			for frame in stack:
				err += frame
		
		_out_msg(str(msg), err)
	
	
	def get_name_from_file(path):
		cache = get_name_from_file.__dict__
		
		key1 = (path, config.language)
		if key1 in cache:
			return cache[key1]
		
		key2 = (path, None)
		if key2 in cache:
			return cache[key2]
		
		key3 = (path, 'path is loaded', '?')
		if key3 not in cache:
			cache[key3] = True
			
			with open(path, 'rb') as f:
				content = f.read().decode('utf-8')
			
			for s in content.split('\n'):
				if not s: continue
				
				if key2 not in cache:
					cache[key2] = s.strip()
				else:
					i = s.index('=')
					lang = s[:i].strip()
					name = s[i+1:].strip()
					cache[(path, lang)] = name
			
			if key1 in cache:
				return cache[key1]
			
			if key2 in cache:
				return cache[key2]
		
		out_msg('get_name_from_file', 'File <%s> is incorrect')
		return 'NoName'
