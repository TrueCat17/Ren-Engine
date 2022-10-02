init -100002 python:
	
	for t in 'None', 'Type', 'Class', 'Instance', 'Method', 'Function':
		t = getattr(types, t + 'Type')
		setattr(__builtins__, t.__name__, t)
	del t
	
	safe_types = (list, tuple, set, dict, types.TypeType, types.ClassType, types.InstanceType, types.MethodType, types.FunctionType)
	strong_safe_types = safe_types[-5:]
	
	simple_types = (type(None), bool, int, long, float, absolute, str)
	collection_types = (list, tuple, set, dict)
	
	
	def is_lambda(obj):
		return type(obj) is types.FunctionType and obj.func_name == '<lambda>'
	
	def picklable(obj, visited = None, is_persistent = False):
		if obj is globals(): # error
			return False
		if not is_persistent and isinstance(obj, Object) and obj.in_persistent:
			# don't save persistent-object in usual saving, it's error
			return False
		
		t = type(obj)
		if t not in (types.TypeType, types.ClassType) and hasattr(obj, '__getstate__'):
			try:
				obj = obj.__getstate__()
			except:
				return False
			t = type(obj)
		
		if t in simple_types:
			return True
		
		
		if t in strong_safe_types:
			if is_lambda(obj):
				return False
			
			# removed function?
			# example:
			#  def f():
			#    pass
			#  f2 = f
			#  del f
			if t is types.FunctionType and getattr(sys.modules[obj.__module__], obj.func_name) is not obj:
				return False
			
			# instance -> class
			if t is types.InstanceType:
				obj = obj.__class__
				t = type(obj)
			
			# instance.method -> class of instance
			if t is types.MethodType:
				obj = obj.im_class
				t = type(obj)
			
			# removed class?
			if t is types.ClassType and getattr(sys.modules[obj.__module__], obj.__name__) is not obj:
				return False
			
			return True
		
		if visited is None:
			visited = set()
		visited.add(id(obj))
		if t in collection_types:
			for i, child in (obj.iteritems() if t is dict else enumerate(obj)):
				child_id = id(child)
				if child_id in visited: continue
				
				visited.add(child_id)
				if not picklable(child, visited, is_persistent):
					return False
			return True
		
		try:
			pickle.dumps(obj)
			return True
		except:
			return False
	
	
	def _pickle_method(method):
		func_name = method.im_func.__name__
		obj = method.im_self
		cls = method.im_class
		return _unpickle_method, (func_name, obj, cls)
	
	def _unpickle_method(func_name, obj, cls):
		if hasattr(cls, 'mro'):
			for cls in cls.mro():
				if cls.__dict__.has_key(func_name):
					func = cls.__dict__[func_name]
					break
		else:
			func = cls.__dict__[func_name]
		return func.__get__(obj, cls)
	
	def register_instancemethod_for_pickle():
		import copy_reg
		copy_reg.pickle(types.MethodType, _pickle_method)
	register_instancemethod_for_pickle()


init -10002 python:
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
		except:
			paths_list = pickling.get_paths_to_unpicklable(obj, is_persistent = obj is persistent)
			paths = ''
			for i in paths_list:
				paths += '  ' + i + '\n'
			if paths.endswith('\n'):
				paths = paths[:-1]
			
			out_msg('pickling.save_object', _('Error on saving objects to file <%s>:\n%s') % (obj_path, paths))
			raise
	
	def pickling__get_paths_to_objects(obj, visited, path = '', is_persistent = False):
		visited.add(id(obj))
		
		if obj is globals(): # error
			return [obj]
		if not is_persistent and isinstance(obj, Object) and obj.in_persistent: # error
			return obj
		
		t = type(obj)
		if t not in (types.TypeType, types.ClassType) and hasattr(obj, '__getstate__'):
			try:
				obj = obj.__getstate__()
			except:
				return obj
			t = type(obj)
		
		if t not in collection_types:
			return obj
		
		res = {}
		for i, child in (obj.iteritems() if t is dict else enumerate(obj)):
			if type(child) in simple_types: continue
			
			child_id = id(child)
			if child_id in visited: continue
			visited.add(child_id)
			
			child_path = path + ('.' if path else '') + str(i)
			child_res = pickling.get_paths_to_objects(child, visited, child_path, is_persistent)
			if type(child_res) is dict:
				for child2_path, child2 in child_res.iteritems():
					res[child2_path] = child2
			else:
				res[child_path] = child
		return res
	
	def pickling__get_paths_to_unpicklable(obj, path = '', is_persistent = False):
		paths_dict = pickling.get_paths_to_objects(obj, set(), is_persistent = is_persistent)
		paths = paths_dict.keys()
		paths.sort()
		
		res = []
		for path in paths:
			if not picklable(paths_dict[path], is_persistent = is_persistent):
				res.append(path)
		return res
	
	
	def pickling__load_global_vars(path):
		obj = pickling.load_object(path)
		g = globals()
		g.update(obj)
	
	def pickling__save_global_vars(path):
		g = globals()
		obj = {}
		
		for k, o in g.iteritems():
			t = type(o)
			
			if t is types.FunctionType:
				# lambda is not picklable
				if is_lambda(o):
					continue
				# don't save just func names
				if o.__module__ == '__main__' and o.func_name == k:
					continue
			
			if t is types.ClassType:
				if t.__module__ == '__main__' and t.__name__ == k:
					continue
			
			# don't save persistent
			if isinstance(o, Object) and o.in_persistent:
				continue
			
			# renpy contains module <random>, modules can't be saved
			# reference to globals() are not saved too
			if o is renpy or o is g:
				continue
			
			# skip vars of ScreenLang
			if k.startswith('_SL_'):
				continue
			
			if t in simple_types or t in safe_types:
				obj[k] = o
		
		paths_list = pickling.get_paths_to_unpicklable(obj)
		if paths_list:
			paths = ''
			for i in paths_list:
				paths += '  ' + i + '\n'
			if paths.endswith('\n'):
				paths = paths[:-1]
			
			out_msg('pickling.save_global_vars', _('Unpicklable object paths:\n%s') % paths)
			return False
		
		pickling.save_object(path, obj)
		return True
	
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
	
	if not persistent.has_attr('config'):
		persistent.config = Object()
	
	for prop in ('_seen_labels', '_seen_images', '_seen_audio'):
		if not persistent.has_attr(prop):
			persistent[prop] = Object()
	
	if not persistent._seen_labels.has_key(get_current_mod()):
		persistent._seen_labels[get_current_mod()] = Object()
	
	persistent_need_save = False
	def persistent_save():
		global persistent_need_save
		if persistent_need_save:
			persistent_need_save = False
			pickling.save_object(persistent_path, persistent)
	
	signals.add('exit_frame', persistent_save)
