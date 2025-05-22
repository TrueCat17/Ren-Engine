init -1000000 python:
	class SimpleObject:
		def __init__(self, obj = None):
			if obj:
				self.__dict__ = obj.__dict__.copy()
		
		def __str__(self):
			return '<instance of %s>' % (self.__class__.__name__, )
		def __repr__(self):
			return str(self)
		
		def __hash__(self):
			return hash(object.__repr__(self))
		
		def __contains__(self, prop):
			return prop in self.__dict__
		
		def get(self, prop, default_value = None):
			return self.__dict__.get(prop, default_value)
		
		def setdefault(self, prop, default_value):
			return self.__dict__.setdefault(prop, default_value)
		
		def __dir__(self):
			return self.__dict__.__dir__()
		def __iter__(self):
			return self.__dict__.__iter__()
		def keys(self):
			return self.__dict__.keys()
		def values(self):
			return self.__dict__.values()
		def items(self):
			return self.__dict__.items()
	
	SimpleObject.__getitem__ = object.__getattribute__
	SimpleObject.__setitem__ = object.__setattr__
	SimpleObject.__delitem__ = object.__delattr__
