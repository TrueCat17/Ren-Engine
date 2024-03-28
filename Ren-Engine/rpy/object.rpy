init -100000 python:
	class Object:
		def __init__(self, _obj = None, **kwords):
			self.in_persistent = False
			self.not_persistent_props = set()
			
			if _obj is not None:
				for k, v in _obj.__dict__.items():
					self.__dict__[k] = v
			for k, v in kwords.items():
				self.__dict__[k] = v
		
		def set_prop_is_not_persistent(self, prop, not_persistent = True):
			changed = False
			if not_persistent:
				changed = prop not in self.not_persistent_props
				if changed:
					self.not_persistent_props.add(prop)
			else:
				changed = prop in self.not_persistent_props
				if changed:
					self.not_persistent_props.remove(prop)
			
			if changed and self.in_persistent:
				global persistent_need_save
				persistent_need_save = True
		
		def __contains__(self, key):
			return key in self.__dict__
		
		def get(self, attr, default_value = None):
			return self.__dict__.get(attr, default_value)
		
		def __getattribute__(self, attr):
			d = object.__getattribute__(self, '__dict__')
			if attr == '__dict__':
				return d
			if attr in d:
				return d[attr]
			
			# need use __getstate__ instead __reduce__?
			if attr == '__reduce__':
				if persistent_saving or not d['in_persistent']:
					raise AttributeError(attr)
			try:
				return object.__getattribute__(self, attr)
			except:
				return None
		
		def __setattr__(self, attr, value):
			d = self.__dict__
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
			
			if self.in_persistent and attr not in self.not_persistent_props:
				if isinstance(value, Object):
					_set_object_in_persistent(value)
				
				global persistent_need_save
				persistent_need_save = True
		
		def __delattr__(self, attr):
			del self.__dict__[attr]
			if self.in_persistent and attr not in self.not_persistent_props:
				global persistent_need_save
				persistent_need_save = True
		
		def __getitem__(self, item):
			return self.__getattribute__(item)
		def __setitem__(self, item, value):
			self.__setattr__(item, value)
		def __delitem__(self, item):
			self.__delattr__(item)
		
		
		def __str__(self):
			return '<instance of ' + self.__class__.__name__ + '>'
		
		def __repr__(self):
			return str(self)
		def __hash__(self):
			return hash(object.__repr__(self))
		def __nonzero__(self):
			return True
		
		def __dir__(self):
			return filter(_object_keys_filter, self.__dict__.keys()) # not lambda for ability to pickle
		def __iter__(self):
			return self.__dir__()
		def keys(self):
			return self.__dir__()
		def items(self):
			return filter(_object_items_filter, self.__dict__.items())
		def values(self):
			return map(self.get, filter(_object_keys_filter, self.__dict__.keys()))
		
		# for pickle persistent objects not in <persistent> file
		#  (see __getattribute__)
		def __reduce__(self):
			path = _get_persistent_object_path(persistent, self)
			return _restore_object_from_persistent, path
		
		# for usual pickle
		def __getstate__(self):
			res = dict(self.__dict__)
			if not res['in_persistent']:
				del res['in_persistent']
			if not res['not_persistent_props']:
				del res['not_persistent_props']
			
			for prop in self.not_persistent_props:
				if prop in res:
					del res[prop]
			return res
	
		def __setstate__(self, new_dict):
			if 'in_persistent' not in new_dict:
				new_dict['in_persistent'] = False
			if 'not_persistent_props' not in new_dict:
				new_dict['not_persistent_props'] = set()
			self.__dict__.update(new_dict)
	
	
	def _restore_object_from_persistent(*path):
		obj = persistent
		for s in path:
			obj = obj[s]
		return obj
	
	def _get_persistent_object_path(parent, obj, cur_path = ()):
		if parent is obj:
			return cur_path
		
		if isinstance(parent, Object):
			parent = parent.__dict__
		if isinstance(parent, dict):
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
			obj.in_persistent = True
			obj = obj.__dict__
		
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
