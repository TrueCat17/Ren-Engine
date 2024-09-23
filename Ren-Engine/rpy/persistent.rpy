init -100002 python:
	
	setattr(__builtins__, 'NoneType', type(None))
	setattr(__builtins__, 'MethodType', types.MethodType)
	setattr(__builtins__, 'FunctionType', types.FunctionType)
	setattr(__builtins__, 'module', types.ModuleType)
	setattr(__builtins__, 'builtin_function_or_method', types.BuiltinFunctionType)
	
	simple_types = (type(None), bool, int, float, absolute, str, types.BuiltinFunctionType, types.ModuleType)
	collection_types = (list, tuple, set, dict)
	
	
	class GlobalPicklingCanaryException(Exception):
		pass
	class GlobalPicklingCanary:
		def __getstate__(self):
			raise GlobalPicklingCanaryException('Trying to save global dict from globals()')
	_global_pickling_canary = GlobalPicklingCanary()
	
	
	class DontSave(SimpleObject):
		def __reduce__(self):
			return DontSave, ()
	dont_save = DontSave()
	
	
	def picklable(obj, visited = None):
		if obj is globals(): # error
			return False
		
		t = type(obj)
		if t in simple_types:
			return True
		
		if t is not type:
			try:
				getstate = obj.__getstate__
			except:
				pass
			else:
				try:
					state = getstate()
				except:
					return False
				if state is not None:
					obj = state
					t = type(obj)
					if t in simple_types:
						return True
		
		
		# removed function (or lambda)?
		# example:
		#  def f():
		#    pass
		#  f2 = f
		#  del f
		if t is types.FunctionType:
			return getattr(sys.modules.get(obj.__module__), obj.__name__, None) is obj
		
		# instance.method -> class of instance
		if t is types.MethodType:
			obj = obj.__self__
			t = type(obj)
		
		# removed class?
		if t is type:
			return getattr(sys.modules.get(obj.__module__), obj.__name__, None) is obj
		
		
		if visited is None:
			visited = set()
			visited.add(id(obj))
		if t in collection_types:
			for child in (obj.values() if t is dict else obj):
				child_id = id(child)
				if child_id in visited: continue
				
				visited.add(child_id)
				if not picklable(child, visited):
					return False
			return True
		
		try:
			pickle.dumps(obj)
			return True
		except:
			return False
	
	def is_picklable_func(called_from, func, param_name):
		if not callable(func):
			out_msg(called_from, 'Param <%s> is not callable' % (param_name, ))
			return False
		if not picklable(func):
			out_msg(called_from, 'Param <%s> is not picklable' % (param_name, ))
			return False
		return True
	
	
	def _pickle_module(module):
		return _unpickle_module, (module.__name__, )
	def _unpickle_module(module_name):
		return sys.modules[module_name]
	
	def _pickle_builtin_func(func):
		if func.__module__ is None: # builtin method
			return func.__reduce__()
		return _unpickle_builtin_func, (func.__module__, func.__name__)
	def _unpickle_builtin_func(module_name, func_name):
		return getattr(sys.modules.get(module_name), func_name)
	
	def _register_pickle_helpers():
		import copyreg
		copyreg.pickle(types.ModuleType, _pickle_module)
		copyreg.pickle(types.BuiltinFunctionType, _pickle_builtin_func)
	_register_pickle_helpers()


init -10002 python:
	persistent_saving = False
	
	def pickling__load_object(path):
		try:
			if (not os.path.exists(path)) or os.path.getsize(path) == 0:
				res = dict()
				out_msg('pickling.load_object', _('File <%s> is not exists or empty') % path)
			else:
				tmp_file = open(path, 'rb')
				res = pickle.load(tmp_file)
				tmp_file.close()
			return res
		except:
			out_msg('pickling.load_object', _('Error on loading object from file <%s>') % path)
			raise
	
	def pickling__save_object(path, obj):
		try:
			dirname = os.path.dirname(path)
			if dirname and not os.path.isdir(dirname):
				os.mkdir(dirname)
			
			tmp_file = open(path, 'wb')
			pickle.dump(obj, tmp_file, protocol=pickle.HIGHEST_PROTOCOL)
			tmp_file.close()
			return True
		except:
			paths_list = pickling.get_paths_to_unpicklable(obj)
			paths = ''
			for i in paths_list:
				paths += '  ' + i + '\n'
			if paths.endswith('\n'):
				paths = paths[:-1]
			
			out_msg('pickling.save_object', _('Error on saving objects to file <%s>:\n%s') % (path, paths))
			return False
	
	def pickling__get_paths_to_unpicklable_helper(obj, visit_func, visited):
		if obj is globals(): # error
			return [obj] # dict is ok, need return error (not dict)
		
		t = type(obj)
		if t not in collection_types:
			try:
				getstate = obj.__getstate__
			except:
				pass
			else:
				try:
					state = getstate()
				except:
					return obj
				if state is not None:
					obj = state
					t = type(obj)
		
		if t not in collection_types:
			return obj
		
		res = {}
		for i, child in (obj.items() if t is dict else enumerate(obj)):
			if type(child) in simple_types: continue
			
			child_id = id(child)
			if child_id in visited: continue
			visited.add(child_id)
			
			child_res = visit_func(child, visit_func, visited)
			if type(child_res) is dict:
				if not child_res: continue
				
				for child2_path, child2 in child_res.items():
					if not picklable(child2):
						res[(i, ) + child2_path] = child2
			else:
				if not picklable(child):
					res[(i, )] = child
		return res
	
	def pickling__get_paths_to_unpicklable(obj, path = ''):
		visited = set()
		visited.add(id(obj))
		
		visit_func = pickling.get_paths_to_unpicklable_helper
		paths_dict = visit_func(obj, visit_func, visited)
		
		paths = ['.'.join(map(str, path)) for path in paths_dict.keys()]
		paths.sort()
		return paths
	
	
	def pickling__load_global_vars(path):
		obj = pickling.load_object(path)
		g = globals()
		g.update(obj)
	
	def pickling__save_global_vars(path):
		g = globals()
		obj = {}
		
		for k, o in dict(g).items():
			t = type(o)
			
			if t is types.FunctionType:
				# don't save just func names
				if o.__module__ == '__main__' and o.__name__ == k:
					continue
			
			if t is type:
				# don't save just class names
				if o.__module__ == '__main__' and o.__name__ == k:
					continue
			
			# don't save just module names
			if t is types.ModuleType and o.__name__ == k:
				continue
			
			# don't save reference to globals()
			if o is g or o is _global_pickling_canary:
				continue
			
			# skip vars of ScreenLang
			if k.startswith('_SL_'):
				continue
			
			obj[k] = o
		
		return pickling.save_object(path, obj)
	
	build_object('pickling')



init -10001 python:
	persistent_path = '../var/persistent'
	
	try:
		if (not os.path.exists(persistent_path)) or os.path.getsize(persistent_path) == 0:
			persistent = Object()
		else:
			persistent = pickling.load_object(persistent_path)
	except:
		persistent = Object()
		raise


init -10001 python:
	persistent.in_persistent = True
	
	if 'config' not in persistent:
		persistent.config = Object()
	
	for prop in ('_seen_labels', '_seen_images', '_seen_audio'):
		if prop not in persistent:
			persistent[prop] = Object()
	
	if get_current_mod() not in persistent._seen_labels:
		persistent._seen_labels[get_current_mod()] = Object()
	
	
	persistent_need_save = False
	def persistent_save():
		global persistent_need_save, persistent_saving
		if persistent_need_save:
			persistent_need_save = False
			
			persistent_saving = True
			pickling.save_object(persistent_path, persistent)
			persistent_saving = False
	
	signals.add('exit_frame', persistent_save)
