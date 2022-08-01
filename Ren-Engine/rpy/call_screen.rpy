init -1000 python:
	call_screen_ready = True
	def call_screen_ready_func():
		return call_screen_ready
	can_exec_next_check_funcs.append(call_screen_ready_func)
	
	call_screen_name, call_ret_name = None, None
	
	
	class Return(Object):
		def __init__(self, value):
			Object.__init__(self)
			self.value = value
		def __call__(self):
			global call_screen_ready, call_screen_name, call_ret_name
			
			g = globals()
			g[call_ret_name] = self.value
			
			call_screen_ready = True
			hide_screen(call_screen_name)
			
			call_screen_name, call_ret_name = None, None
