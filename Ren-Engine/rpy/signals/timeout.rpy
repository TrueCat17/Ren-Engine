init -10000 python:
	
	_timeout_id = 0
	_timeout_funcs = []
	
	def set_timeout(function, time_sec):
		if not callable(function):
			out_msg('set_timeout', '<' + str(function) + '> is not callable')
			return 0
		global _timeout_id
		_timeout_id += 1
		_timeout_funcs.append([_timeout_id, function, time_sec])
		return _timeout_id
	
	def clear_timeout(id):
		if id <= 0:
			out_msg('clear_timeout', 'Invalid id <' + str(id) + '>')
			return
		i = 0
		while i < len(_timeout_funcs):
			if _timeout_funcs[i][0] == id:
				_timeout_funcs[i][1] = None
				break
			i += 1
	
	def exec_timeouts():
		dtime = get_last_tick()
		for timeout_obj in _timeout_funcs:
			id, function, before_exec_time = timeout_obj
			timeout_obj[2] = before_exec_time = round(before_exec_time - dtime, 5)
			if function and before_exec_time <= 0: # not cleared and need exec
				timeout_obj[1] = None # clear
				try:
					function()
				except Exception as e:
					out_msg('exec_timeouts, id=' + str(id) + ', function=' + str(function), str(e))
		
		i = 0
		while i < len(_timeout_funcs):
			if _timeout_funcs[i][1]: # not cleared
				i += 1
			else:
				_timeout_funcs.pop(i)
	
	signals.add('enter_frame', exec_timeouts)

