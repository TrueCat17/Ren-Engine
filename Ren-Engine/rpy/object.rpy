init -100000 python:
	class Object:
		def __init__(self, _obj = None, **kwords):
			self.in_persistent = False
			self.not_persistent_props = set()
			
			if _obj is not None:
				for k, v in _obj.__dict__.iteritems():
					self.__dict__[k] = v
			for k, v in kwords.iteritems():
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
		
		def __getattr__(self, attr):
			if attr in self.__dict__:
				return self.__dict__[attr]
			if attr.startswith('__'):
				raise AttributeError(attr)
			return None
		
		def __setattr__(self, attr, value):
			if attr in self.__dict__:
				old = self.__dict__[attr]
				if type(old) in simple_types:
					dont_change = type(old) is type(value) and old == value
				else:
					dont_change = old is value
					# 'is', not '==', because code (for example):
					#    obj.prop = []
					#    ...
					#    obj.prop = array = []
					#    array.append(123)
					#    print obj.prop # [123], not []
				if dont_change:
					return
			
			self.__dict__[attr] = value
			
			if self.in_persistent and attr not in self.not_persistent_props:
				if isinstance(value, Object):
					value.in_persistent = True
				
				global persistent_need_save
				persistent_need_save = True
		
		def __delattr__(self, attr):
			del self.__dict__[attr]
			if self.in_persistent and attr not in self.not_persistent_props:
				global persistent_need_save
				persistent_need_save = True
		
		def __getitem__(self, item):
			return self.__getattr__(item)
		def __setitem__(self, item, value):
			self.__setattr__(item, value)
		def __delitem__(self, item):
			self.__delattr__(item)
		
		
		def __str__(self):
			self_class = str(self.__class__)
			index = self_class.rfind('.')
			if index != -1:
				self_class = self_class[index + 1:]
			
			return '<instance of ' + self_class + '>'
		
		def __repr__(self):
			return str(self)
		def __hash__(self):
			return hash(object.__repr__(self))
		def __nonzero__(self):
			return True
		
		def __cmp__(self, other):
			# not `return id(self) - id(other)`
			#   because __cmp__ must return int
			#   but ptr_diff is unsigned int, mb int overflow and returning long (that error)
			if self is other: return 0
			if id(self) < id(other): return -1
			return 1
		
		def get_props(self, get_list = False):
			keys = self.__dict__.keys()
			keys.remove('in_persistent')
			keys.remove('not_persistent_props')
			
			return keys if get_list else ' '.join(keys)
		
		
		# for pickle
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

