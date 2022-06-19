init -10000 python:
	def exec_funcs(funcs):
		if not isinstance(funcs, (list, tuple)):
			funcs = [funcs]
		for func in funcs:
			if func is not None:
				func()
	
	
	def If(cond, true, false):
		return true if cond else false
	
	class Function(Object):
		def __init__(self, func, *args, **kwargs):
			Object.__init__(self)
			self.func, self.args, self.kwargs = func, args, kwargs
		def __call__(self):
			return apply(self.func, self.args, self.kwargs)
	
	
	_undefined = object()
	
	class SetDict(Object):
		def __init__(self, obj, var_name, value):
			Object.__init__(self, obj = obj, var_name = var_name, value = value)
		def __call__(self, v = _undefined):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] = self.value if v is _undefined else v
	class AddDict(Object):
		def __init__(self, obj, var_name, value):
			Object.__init__(self, obj = obj, var_name = var_name, value = value)
		def __call__(self, v = _undefined):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] += self.value if v is _undefined else v
	class ToggleDict(Object):
		def __init__(self, obj, var_name, true_value = True, false_value = False):
			Object.__init__(self, obj = obj, var_name = var_name, true_value = true_value, false_value = false_value)
		def __call__(self, v = _undefined):
			obj = self.obj if self.obj is not None else globals()
			if v is _undefined:
				v = self.true_value if (obj[self.var_name] != self.true_value) else self.false_value
			obj[self.var_name] = v
	
	
	def getset_attr(attrs_str, value = _undefined, obj = None):
		attrs = attrs_str.split('.')
		if value is not _undefined:
			attr = attrs.pop()
		
		if obj is None:
			obj = globals()
		for i in xrange(len(attrs)):
			name = attrs[i]
			if type(obj) is dict:
				obj = obj[name]
			else:
				obj = getattr(obj, name)
		
		if value is _undefined:
			return obj
		
		if type(obj) is dict:
			obj[attr] = value
		else:
			setattr(obj, attr, value)
	
	class SetVariable(Object):
		def __init__(self, attrs_str, value):
			Object.__init__(self, attrs_str = attrs_str, value = value)
		def __call__(self, v = _undefined):
			getset_attr(self.attrs_str, self.value if v is _undefined else v)
	class AddVariable(Object):
		def __init__(self, attrs_str, value):
			Object.__init__(self, attrs_str = attrs_str, value = value)
		def __call__(self, v = _undefined):
			getset_attr(self.attrs_str, getset_attr(self.attrs_str) + (self.value if v is _undefined else v))
	class ToggleVariable(Object):
		def __init__(self, attrs_str, true_value = True, false_value = False):
			Object.__init__(self, attrs_str = attrs_str, true_value = true_value, false_value = false_value)
		def __call__(self, v = _undefined):
			if v is _undefined:
				v = self.true_value if getset_attr(self.attrs_str) != self.true_value else self.false_value
			getset_attr(self.attrs_str, v)
	
	
	def Jump(label):
		return Function(renpy.jump, label)
	def Call(label):
		return Function(renpy.call, label)
	
	def Play(file_name, channel):
		return Function(renpy.play, file_name, channel)
	def Stop(channel):
		return Function(renpy.stop, channel)
	
	def Show(name):
		return Function(show_screen, name)
	def Hide(name):
		return Function(hide_screen, name)
	ShowMenu = ShowScreen = Show
	HideMenu = HideScreen = Hide
	
	def Notify(msg):
		return Function(notification.out, str(msg))
	
	def Language(lang):
		return Function(renpy.change_language, lang)
	
	# Return -> call_screen.rpy
	
	
	class SetDictFuncRes(Object):
		def __init__(self, obj, var_name, func):
			Object.__init__(self, obj = obj, var_name = var_name, func = func)
		def __call__(self):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] = self.func()
	
