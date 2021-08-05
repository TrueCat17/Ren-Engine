init -10001 python:

	class Signals:
		def __init__(self):
			self.funcs = {}
		
		def add(self, event, function, priority = 0):
			if not picklable(function):
				out_msg('Signals.add', 'Function <' + str(function) + '> is not picklable')
				return
			
			if event not in self.funcs:
				self.funcs[event] = []
			self.funcs[event].append([priority, function])
			self.funcs[event].sort(key = lambda l: l[0])
		
		def remove(self, event, function):
			for obj in self.funcs.get(event, ()):
				if obj[1] is function:
					obj[1] = None
					break
		
		def send(self, event, *args, **kwargs):
			funcs = self.funcs.get(event, ())
			
			for priority, function in funcs:
				if not function: continue # removed
				
				self.event = event
				try:
					function(*args, **kwargs)
				except:
					out_msg('Signals.send, event=' + str(event) + ', function=' + str(function))
			
			i = 0
			while i < len(funcs):
				if funcs[i][1]:
					i += 1
				else:
					funcs.pop(i)
	
	signals = Signals()
	