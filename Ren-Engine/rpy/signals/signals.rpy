init -10001 python:

	class Signals:
		def __init__(self):
			self.funcs = {}
		
		def add(self, event, function, priority = 0, times = -1):
			if times == 0:
				return
			if not picklable(function):
				out_msg('Signals.add', 'Function <%s> is not picklable' % function)
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
			funcs = self.funcs.get(event, ())
			
			for priority, function, times in funcs:
				if not function: continue # removed
				
				self.event = event
				try:
					function(*args, **kwargs)
				except:
					out_msg('Signals.send', 'Event=%s, Function=%s' % (event, function))
			
			i = 0
			while i < len(funcs):
				func = funcs[i]
				
				func[2] -= 1
				if func[1] and func[2]:
					i += 1
				else:
					funcs.pop(i)
	
	signals = Signals()
	
