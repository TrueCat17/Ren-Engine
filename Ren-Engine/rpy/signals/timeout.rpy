init -100000 python:
	_timeout_id = 0
	_timeout_tasks = []
	
	
	def set_timeout(function, time_sec):
		if not is_picklable_func('set_timeout', function, 'function'):
			return 0
		if not is_number('set_timeout', time_sec, 'time_sec'):
			return 0
		
		global _timeout_id
		_timeout_id += 1
		
		filename, numline = get_file_and_line(1)
		
		task = [_timeout_id, function, time_sec, filename, numline]
		_timeout_tasks.append(task)
		
		return _timeout_id
	
	
	def clear_timeout(id):
		# just mark, dont remove directly, because clear-func can be called from exec-func
		for task in _timeout_tasks:
			if task[0] == id:
				task[1] = None
				break
	
	
	def exec_timeouts():
		if not _timeout_tasks:
			return
		
		dtime = get_last_tick()
		for task in _timeout_tasks:
			id, function, before_exec_time, filename, numline = task
			task[2] = before_exec_time = round(before_exec_time - dtime, 5)
			if function and before_exec_time <= 0: # not cleared and need exec
				task[1] = None # clear
				try:
					function()
				except:
					func_name = getattr(function, '__name__', str(function))
					out_msg('exec_timeouts',
						'Id = %s, Function = %s (set_timeout is called from %s:%s)',
						id, func_name, filename, numline,
					)
		
		_timeout_tasks[:] = [task for task in _timeout_tasks if task[1]]
	
	
	signals.add('enter_frame', exec_timeouts)
