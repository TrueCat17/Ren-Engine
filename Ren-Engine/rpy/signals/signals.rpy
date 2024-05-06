init -100001 python:

	class Signals:
		def __init__(self):
			self.funcs = {}
			self.checking = True
			self.queue_for_check = []
		
		def set_check_picklable(self, value):
			self.checking = value
			if value:
				try:
					pickle.dumps(self.queue_for_check) # fast check for all new funcs
				except:
					out_msg('Signals.set_check_picklable(True)', 'Some function is not picklable')
				self.queue_for_check = []
		
		def add(self, event, function, priority = 0, times = -1):
			if times == 0:
				return
			
			if self.checking:
				if not picklable(function):
					out_msg('Signals.add', 'Function <%s> is not picklable' % function)
					return
			else:
				self.queue_for_check.append(function)
			
			if not callable(function):
				out_msg('Signals.add', '<%s> is not callable' % function)
				return
			
			if event not in self.funcs:
				self.funcs[event] = []
			self.funcs[event].append([priority, function, times])
			self.funcs[event].sort(key = lambda l: l[0])
		
		def remove(self, event, function):
			for obj in self.funcs.get(event, ()):
				if obj[1] is function:
					obj[1] = None
					break
		
		def send(self, event, *args, **kwargs):
			if event == 'exit_frame' and not self.checking:
				out_msg('You forgot to call signals.set_check_picklable(True)')
				self.set_check_picklable(True)
			
			funcs = self.funcs.get(event, ())
			
			for priority, function, times in funcs:
				if not function: continue # removed
				
				self.event = event
				try:
					function(*args, **kwargs)
				except:
					func_name = getattr(function, '__name__', str(function))
					out_msg('Signals.send', 'Event = %s, Function = %s' % (event, func_name))
			
			i = 0
			while i < len(funcs):
				func = funcs[i]
				
				func[2] -= 1
				if func[1] and func[2]:
					i += 1
				else:
					funcs.pop(i)
	
	signals = Signals()
	
