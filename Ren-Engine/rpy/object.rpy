init -100000 python:
	class Object:
		def __init__(self, obj = None, **kwords):
			self.in_persistent = False
			
			if obj is not None:
				for k, v in obj.__dict__.iteritems():
					self.__dict__[k] = v
			for k, v in kwords.iteritems():
				self.__dict__[k] = v
		
		def has_key(self, attr):
			return self.__dict__.has_key(attr)
		def has_attr(self, attr):
			return self.__dict__.has_key(attr)
		
		def __getattr__(self, attr):
			if self.__dict__.has_key(attr):
				return self.__dict__[attr]
			if attr.startswith('__'):
				raise AttributeError(attr)
			return None
		
		def __setattr__(self, attr, value):
			if not self.__dict__.has_key(attr) or self.__dict__[attr] is not value:
				self.__dict__[attr] = value
				
				if self.in_persistent:
					if isinstance(value, Object):
						value.in_persistent = True
					
					global persistent_need_save
					persistent_need_save = True
		
		def __delattr__(self, attr):
			del self.__dict__[attr]
			if self.in_persistent:
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
			return id(self) - id(other)
		
		def get_props(self, get_list = False):
			keys = self.__dict__.keys()
			keys.remove('in_persistent')
			
			return keys if get_list else ' '.join(keys)
		
		
		# for pickle
		def __getstate__(self):
			return self.__dict__
		def __setstate__(self, new_dict):
			self.__dict__.update(new_dict)

