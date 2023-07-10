init -10000 python:
	def exec_funcs(funcs):
		if not isinstance(funcs, (list, tuple)):
			funcs = [funcs]
		for func in funcs:
			if func is None: continue
			
			if type(func) is str:
				file_name, num_line = get_file_and_line(1)
				compiled = compile('\n' * (num_line - 1) + func, file_name, 'exec')
				eval(compiled, globals(), globals())
			else:
				func()
	
	
	def If(cond, true, false):
		return true if cond else false
	
	class Function(Object):
		def __init__(self, func, *args, **kwargs):
			Object.__init__(self, func = func, args = args, kwargs = kwargs)
		def __call__(self, *args, **kwargs):
			return self.func(*(self.args + args), **(self.kwargs | kwargs))
	
	
	class EvalObject(Object):
		def __init__(self, code, mode, depth = 0, file_name = None, num_line = None):
			if file_name is None:
				file_name = get_filename(depth + 1)
			if num_line is None:
				num_line = get_numline(depth + 1)
			Object.__init__(self, code = code, mode = mode, file_name = file_name, num_line = num_line)
			self.compile(depth + 1)
		def __call__(self):
			res = None
			if self.compiled:
				try:
					res = eval(self.compiled, globals(), globals())
				except Exception as e:
					msg = get_exception_stack_str(e, 1)
					out_msg('EvalObject.compile', msg, show_stack = False)
			return res
		
		def compile(self, depth):
			try:
				self.compiled = compile('\n' * (self.num_line - 1) + self.code, self.file_name, self.mode)
				self.error = False
			except Exception as e:
				msg = get_exception_stack_str(e, depth + 1)
				out_msg('EvalObject.compile', msg, show_stack = False)
				self.error = True
		
		# for pickle
		def __getstate__(self):
			return (self.file_name, self.num_line, self.code, self.mode)
		def __setstate__(self, params):
			Object.__init__(self)
			self.file_name, self.num_line, self.code, self.mode = params
			self.compile(-1)
	
	# for get value: Eval("2 + 3")() -> 5
	def Eval(code, file_name = None, num_line = None, depth = 0):
		return EvalObject(code, 'eval', depth + 1, file_name, num_line)
	
	# for exec code: Exec("v = f(2, 3)")() -> None
	def Exec(code, file_name = None, num_line = None, depth = 0):
		return EvalObject(code, 'exec', depth + 1, file_name, num_line)
	
	
	class UndefinedType:
		def __eq__(self, other):
			return type(self) is type(other)
	_undefined = UndefinedType()
	
	class SetDict(Object):
		def __init__(self, obj, var_name, value):
			Object.__init__(self, obj = obj, var_name = var_name, value = value)
		def __call__(self, v = _undefined):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] = self.value if v == _undefined else v
	class AddDict(Object):
		def __init__(self, obj, var_name, value):
			Object.__init__(self, obj = obj, var_name = var_name, value = value)
		def __call__(self, v = _undefined):
			obj = self.obj if self.obj is not None else globals()
			obj[self.var_name] += self.value if v == _undefined else v
	class ToggleDict(Object):
		def __init__(self, obj, var_name, true_value = True, false_value = False):
			Object.__init__(self, obj = obj, var_name = var_name, true_value = true_value, false_value = false_value)
		def __call__(self, v = _undefined):
			obj = self.obj if self.obj is not None else globals()
			if v == _undefined:
				v = self.true_value if (obj[self.var_name] != self.true_value) else self.false_value
			obj[self.var_name] = v
	
	
	def getset_attr(attrs_str, value = _undefined, obj = None):
		attrs = attrs_str.split('.')
		if value != _undefined:
			attr = attrs.pop()
		
		if obj is None:
			obj = globals()
		for i in range(len(attrs)):
			name = attrs[i]
			if type(obj) is dict:
				obj = obj[name]
			else:
				obj = getattr(obj, name)
		
		if value == _undefined:
			return obj
		
		if type(obj) is dict:
			obj[attr] = value
		else:
			setattr(obj, attr, value)
	
	def GetSetAttr(attrs_str, value = _undefined, obj = None):
		return Function(getset_attr, attrs_str, value = value, obj = obj)
	
	class SetVariable(Object):
		def __init__(self, attrs_str, value):
			Object.__init__(self, attrs_str = attrs_str, value = value)
		def __call__(self, v = _undefined):
			getset_attr(self.attrs_str, self.value if v == _undefined else v)
	class AddVariable(Object):
		def __init__(self, attrs_str, value):
			Object.__init__(self, attrs_str = attrs_str, value = value)
		def __call__(self, v = _undefined):
			getset_attr(self.attrs_str, getset_attr(self.attrs_str) + (self.value if v == _undefined else v))
	class ToggleVariable(Object):
		def __init__(self, attrs_str, true_value = True, false_value = False):
			Object.__init__(self, attrs_str = attrs_str, true_value = true_value, false_value = false_value)
		def __call__(self, v = _undefined):
			if v == _undefined:
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
	
	
	def QuickLoad():
		return Function(quick_load)
	def QuickSave():
		return Function(quick_save)
	
	def FilePage(page):
		return Function(slots.set_page, page)
	def FileCurrentPage():
		return slots.get_page()
	
	def FilePageName(auto='a', quick='q'):
		if page == 'quick':
			return quick
		if page == 'auto':
			return auto
		return persistent.slot_page
	
	def FileSave(slot = None, page = None):
		return Function(renpy.save, slot, page)
	def FileLoad(slot = None, page = None):
		return Function(renpy.load, slot, page)
	def FileDelete(slot = None, page = None):
		return Function(renpy.unlink_save, slot, page)
	
	def FileTime(slot = None, page = None, empty = ''):
		res = slots.mtime_formatted(slot, page)
		if res is None:
			return empty
		return res
	def FileScreenshot(slot = None, page = None):
		return renpy.screenshot(slot, page)
	def FileLoadable(slot = None, page = None):
		return renpy.can_load(slot, page)
	
	
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
