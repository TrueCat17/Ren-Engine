init -10000 python:
	def exec_funcs(funcs):
		if isinstance(funcs, (list, tuple)):
			for func in funcs:
				if func is not None:
					func()
		else: # if get only 1 func
			if funcs is not None:
				funcs()
	
	
	def If(cond, true, false):
		return true if cond else false
	
	class Function(Object):
		def __init__(self, func, *args, **kwargs):
			Object.__init__(self)
			self.func, self.args, self.kwargs = func, args, kwargs
		def __call__(self):
			return apply(self.func, self.args, self.kwargs)
	
	class AddDict(Object):
		def __init__(self, obj, var_name, value):
			Object.__init__(self)
			self.obj, self.var_name, self.value = obj, var_name, value
		def __call__(self):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] += self.value
	class SetDict(Object):
		def __init__(self, obj, var_name, value):
			Object.__init__(self)
			self.obj, self.var_name, self.value = obj, var_name, value
		def __call__(self):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] = self.value
	
	def AddVariable(var_name, value):
		return AddDict(None, var_name, value)
	def SetVariable(var_name, value):
		return SetDict(None, var_name, value)
	
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
	
	
	# Return -> call_screen.rpy
