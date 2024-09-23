init -10000 python:
	interruptable_cycles_tasks = []
	interruptable_cycles_task_index = 0
	
	
	def interruptable_for(array, func, on_end = None):
		"""
		for i in range(len(array)):
			func(array[i])
		if on_end is not None:
			on_end()
		"""
		
		if not is_picklable_func('interruptable_for', func, 'func'):
			return
		if on_end is not None and not is_picklable_func('interruptable_for', on_end, 'on_end'):
			return
		
		if not picklable(array):
			out_msg('interruptable_for', 'Array is not picklable')
			return
		
		interruptable_cycles_check_subscribe()
		
		task = [array, 0, func, on_end]
		interruptable_cycles_tasks.append(task)
	
	
	def interruptable_while(func):
		"""
		ended = False
		while not ended:
			ended = func()
		"""
		if not is_picklable_func('interruptable_while', func, 'func'):
			return
		
		interruptable_cycles_check_subscribe()
		
		task = func
		interruptable_cycles_tasks.append(task)
	
	
	
	def interruptable_cycles_check_subscribe():
		if not interruptable_cycles_tasks:
			interruptable_cycles_update_pre()
			signals.add('enter_frame', interruptable_cycles_update_pre, priority = -1e9)
			signals.add('exit_frame', interruptable_cycles_update, priority = 1e9)
	
	
	def interruptable_cycles_update_pre():
		global interruptable_cycles_enter_frame_time
		interruptable_cycles_enter_frame_time = time.time()
	
	
	def interruptable_cycles_update():
		global interruptable_cycles_task_index
		
		st = time.time()
		spent_time = st - interruptable_cycles_enter_frame_time
		have_time = max(1 / get_fps() - spent_time, 0.005)
		
		while True:
			if interruptable_cycles_task_index >= len(interruptable_cycles_tasks):
				interruptable_cycles_task_index = 0
				
			done = interruptable_cycles_update_one()
			if done:
				interruptable_cycles_tasks.pop(interruptable_cycles_task_index)
			else:
				interruptable_cycles_task_index += 1
			
			if time.time() - st > have_time or not interruptable_cycles_tasks:
				break
		
		if not interruptable_cycles_tasks:
			signals.remove('enter_frame', interruptable_cycles_update_pre)
			signals.remove('exit_frame', interruptable_cycles_update)
	
	
	def interruptable_cycles_update_one():
		task = interruptable_cycles_tasks[interruptable_cycles_task_index]
		if type(task) is list:
			is_for = True
			array, index, func, on_end = task
		else:
			is_for = False
			func = task
		
		st = time.time()
		while time.time() - st < 0.005:
			if is_for:
				if index >= len(array):
					if on_end is not None:
						on_end()
					return True
				
				func(array[index])
				index += 1
				task[1] = index
			else:
				ended = func()
				if ended:
					return True
		
		return False
