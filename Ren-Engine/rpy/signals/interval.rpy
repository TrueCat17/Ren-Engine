init -100000 python:
	
	_interval_id = 0
	_interval_funcs = []
	
	def set_interval(function, time_sec):
		if not is_picklable_func('set_interval', function, 'function'):
			return 0
		
		global _interval_id
		_interval_id += 1
		_interval_funcs.append([_interval_id, function, time_sec, time_sec])
		return _interval_id
	
	def clear_interval(id):
		if id <= 0:
			out_msg('clear_interval', 'Invalid id <%s>', id)
			return
		
		i = 0
		while i < len(_interval_funcs):
			if _interval_funcs[i][0] == id:
				_interval_funcs[i][1] = None
				break
			i += 1
	
	def exec_intervals():
		dtime = get_last_tick()
		for interval_obj in _interval_funcs:
			id, function, interval_time, before_exec_time = interval_obj
			before_exec_time = round(before_exec_time - dtime, 5)
			if function and before_exec_time <= 0: # not cleared and need exec
				before_exec_time = interval_time
				try:
					function()
				except:
					func_name = getattr(function, '__name__', str(function))
					out_msg('exec_intervals', 'Id = %s, Function = %s', id, func_name)
			interval_obj[3] = before_exec_time
		
		i = 0
		while i < len(_interval_funcs):
			if _interval_funcs[i][1]: # not cleared
				i += 1
			else:
				_interval_funcs.pop(i)
	
	signals.add('enter_frame', exec_intervals)
