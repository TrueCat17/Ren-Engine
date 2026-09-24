init -100000 python:
	_interval_id = 0
	_interval_tasks = []
	
	
	def set_interval(function, time_sec):
		if not is_picklable_func('set_interval', function, 'function'):
			return 0
		if not is_number('set_interval', time_sec, 'time_sec'):
			return 0
		
		global _interval_id
		_interval_id += 1
		
		filename, numline = get_file_and_line(1)
		
		task = [_interval_id, function, time_sec, time_sec, filename, numline]
		_interval_tasks.append(task)
		
		return _interval_id
	
	
	def clear_interval(id):
		# just mark, dont remove directly, because clear-func can be called from exec-func
		for task in _interval_tasks:
			if task[0] == id:
				task[1] = None
				break
	
	
	def exec_intervals():
		if not _interval_tasks:
			return
		
		dtime = get_last_tick()
		for task in _interval_tasks:
			id, function, interval_time, before_exec_time, filename, numline = task
			before_exec_time = round(before_exec_time - dtime, 5)
			if function and before_exec_time <= 0: # not cleared and need exec
				before_exec_time = interval_time
				try:
					function()
				except:
					func_name = getattr(function, '__name__', str(function))
					out_msg('exec_intervals',
						'Id = %s, Function = %s (set_interval is called from %s:%s)',
						id, func_name, filename, numline,
					)
			task[3] = before_exec_time
		
		_interval_tasks[:] = [task for task in _interval_tasks if task[1]]
	
	
	signals.add('enter_frame', exec_intervals)
