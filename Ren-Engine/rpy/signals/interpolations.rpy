init -100000 python:
	_interpolation_id = 0
	_interpolation_tasks = []
	
	
	def set_interpolation(function, start_value, end_value, time_sec, time_func = None, on_end = None):
		"""
		<on enter frame>:
			k = time_func(dtime / time_sec)
			interpolated_value = ...
			function(interpolated_value)
			
			if dtime >= time_sec:
				if on_end is not None:
					on_end()
				<break>
		"""
		
		if not is_picklable_func('set_interpolation', function, 'function'):
			return 0
		
		if not is_number('set_interpolation', start_value, 'start_value'):
			return 0
		if not is_number('set_interpolation', end_value, 'end_value'):
			return 0
		if not is_number('set_interpolation', time_sec, 'time_sec'):
			return 0
		
		if time_func is not None and not is_picklable_func('set_interpolation', time_func, 'time_func'):
			return 0
		if on_end is not None and not is_picklable_func('set_interpolation', on_end, 'on_end'):
			return 0
		
		if not _interpolation_tasks:
			signals.add('enter_frame', exec_interpolations)
		
		global _interpolation_id
		_interpolation_id += 1
		
		filename, numline = get_file_and_line(1)
		
		task = [_interpolation_id, function, start_value, end_value, get_game_time(), max(time_sec, 0.001), time_func, on_end, filename, numline]
		_interpolation_tasks.append(task)
		
		return _interpolation_id
	
	
	def clear_interpolation(id):
		# just mark, dont remove directly, because clear-func can be called from exec-func
		for task in _interpolation_tasks:
			if task[0] == id:
				task[1] = None
				break
	
	
	def exec_interpolations():
		for task in _interpolation_tasks:
			id, function, start_value, end_value, start_time, time_sec, time_func, on_end, filename, numline = task
			if function is None:
				continue
			
			dtime = get_game_time() - start_time
			k = min(dtime / time_sec, 1.0)
			if time_func is not None:
				k = time_func(k)
			v = start_value + (end_value - start_value) * k
			
			try:
				function(v)
			except:
				func_name = getattr(function, '__name__', str(function))
				out_msg('exec_interpolations',
					'Id = %s, Function = %s (set_interpolation is called from %s:%s)',
					id, func_name, filename, numline,
				)
			
			if dtime >= time_sec:
				if on_end is not None:
					try:
						on_end()
					except:
						func_name = getattr(on_end, '__name__', str(on_end))
						out_msg('exec_interpolations',
							'Id = %s, Function <on_end> = %s (set_interpolation is called from %s:%s)',
							id, func_name, filename, numline,
						)
				
				task[1] = None # clear
		
		_interpolation_tasks[:] = [task for task in _interpolation_tasks if task[1]]
		if not _interpolation_tasks:
			signals.remove('enter_frame', exec_interpolations)
