init -1000010 python:
	object_getattribute = object.__getattribute__
	
	class Object:
		def __init__(self, _obj = None, **kwargs):
			d = object_getattribute(self, '__dict__')
			
			if _obj is not None:
				d.update(_obj.__dict__)
			else:
				d['in_persistent'] = False
				d['not_persistent_props'] = set()
			d.update(kwargs)
		
		def set_prop_is_not_persistent(self, prop, not_persistent = True):
			d = object_getattribute(self, '__dict__')
			not_persistent_props = d['not_persistent_props']
			
			changed = False
			if not_persistent:
				changed = prop not in not_persistent_props
				if changed:
					not_persistent_props.add(prop)
			else:
				changed = prop in not_persistent_props
				if changed:
					not_persistent_props.remove(prop)
			
			if changed and d['in_persistent']:
				global persistent_need_save
				persistent_need_save = True
		
		def __contains__(self, key):
			return key in object_getattribute(self, '__dict__')
		
		def get(self, key, default_value = None):
			return object_getattribute(self, '__dict__').get(key, default_value)
		
		def setdefault(self, key, default_value):
			d = object_getattribute(self, '__dict__')
			if key in d:
				return d[key]
			
			if d['in_persistent'] and key not in d['not_persistent_props']:
				if isinstance(default_value, Object):
					_set_object_in_persistent(default_value)
				
				global persistent_need_save
				persistent_need_save = True
			
			d[key] = default_value
			return default_value
		
		def __getattribute__(self, attr):
			d = object_getattribute(self, '__dict__')
			if attr == '__dict__':
				return d
			if attr in d:
				return d[attr]
			
			# need use __getstate__ instead __reduce__?
			if attr == '__reduce__':
				if persistent_saving or not d['in_persistent']:
					raise AttributeError(attr)
			try:
				return object_getattribute(self, attr)
			except:
				return None
		
		def __setattr__(self, attr, value):
			d = object_getattribute(self, '__dict__')
			if attr in d:
				old = d[attr]
				type_old = type(old)
				if type_old in simple_types:
					dont_change = type_old is type(value) and old == value
				else:
					dont_change = old is value
					# 'is', not '==', because code (for example):
					#    obj.prop = []
					#    ...
					#    obj.prop = array = []
					#    array.append(123)
					#    print(obj.prop) # [123], not []
				if dont_change:
					return
			
			d[attr] = value
			
			if d['in_persistent'] and attr not in d['not_persistent_props']:
				if isinstance(value, Object):
					_set_object_in_persistent(value)
				
				global persistent_need_save
				persistent_need_save = True
		
		def __delattr__(self, attr):
			d = object_getattribute(self, '__dict__')
			del d[attr]
			if d['in_persistent'] and attr not in d['not_persistent_props']:
				global persistent_need_save
				persistent_need_save = True
		
		
		def __str__(self):
			return '<instance of %s>' % (self.__class__.__name__, )
		
		def __repr__(self):
			return str(self)
		def __hash__(self):
			return hash(object.__repr__(self))
		def __nonzero__(self):
			return True
		
		def __dir__(self):
			d = object_getattribute(self, '__dict__')
			return filter(_object_keys_filter, d.keys()) # not lambda for ability to pickle
		def __iter__(self):
			return self.__dir__()
		def keys(self):
			return self.__dir__()
		def items(self):
			d = object_getattribute(self, '__dict__')
			return filter(_object_items_filter, d.items())
		def values(self):
			d = object_getattribute(self, '__dict__')
			return map(self.get, filter(_object_keys_filter, d.keys()))
		
		# for pickle persistent objects not in <persistent> file
		#  (see __getattribute__)
		def __reduce__(self):
			path = _get_persistent_object_path(persistent, self)
			return _restore_object_from_persistent, path
		
		# for usual pickle
		def __getstate__(self):
			res = object_getattribute(self, '__dict__').copy()
			not_persistent_props = res['not_persistent_props']
			
			if not res['in_persistent']:
				del res['in_persistent']
			if not res['not_persistent_props']:
				del res['not_persistent_props']
			
			for prop in not_persistent_props:
				res.pop(prop, None)
			return res
	
		def __setstate__(self, new_dict):
			d = object_getattribute(self, '__dict__')
			d.update(new_dict)
			
			d.setdefault('in_persistent', False)
			d.setdefault('not_persistent_props', set())
	
	Object.__getitem__ = Object.__getattribute__
	Object.__setitem__ = Object.__setattr__
	Object.__delitem__ = Object.__delattr__
	
	
	def _restore_object_from_persistent(*path):
		obj = persistent
		for s in path:
			obj = obj[s]
		return obj
	
	def _get_persistent_object_path(parent, obj, cur_path = ()):
		if parent is obj:
			return cur_path
		
		if isinstance(parent, (dict, Object)):
			it = parent.items()
		elif isinstance(parent, (list, tuple)):
			it = enumerate(parent)
		else:
			return None
		
		for k, v in it:
			if isinstance(v, Object):
				res = _get_persistent_object_path(v, obj, cur_path + (k, ))
				if res:
					return res
		return None
	
	def _set_object_in_persistent(obj):
		if isinstance(obj, Object):
			obj = obj.__dict__
			obj['in_persistent'] = True
		
		if isinstance(obj, dict):
			it = obj.values()
		else:
			try:
				it = iter(obj)
			except:
				return
		
		for v in it:
			_set_object_in_persistent(v)
	
	def _object_keys_filter(key):
		return key not in ('in_persistent', 'not_persistent_props')
	def _object_items_filter(item):
		return item[0] not in ('in_persistent', 'not_persistent_props')
