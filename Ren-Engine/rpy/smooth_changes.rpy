init -10000 python:
	def smooth_changes__start(name, start_props, end_props, res_props, change_time, on_end):
		if type(end_props) not in (tuple, list):
			end_props = (end_props, )
		
		if type(change_time) not in (tuple, list):
			change_time = (change_time, ) * len(end_props)
		elif len(change_time) != len(end_props):
			out_msg('smooth_changes.start', 'len(change_time) != len(end_props) (%s != %s)', len(change_time), len(end_props))
			return
		
		if type(on_end) not in (tuple, list):
			on_end = (None, ) * (len(end_props) - 1) + (on_end, )
		elif len(on_end) != len(end_props):
			out_msg('smooth_changes.start', 'len(on_end) != len(end_props) (%s != %s)', len(on_end), len(end_props))
			return
		
		start_keys = sorted([prop for prop in start_props.keys() if not prop.endswith('_time_func')])
		for tmp_props in end_props:
			end_keys = sorted([prop for prop in tmp_props.keys() if not prop.endswith('_time_func')])
			if start_keys != end_keys:
				out_msg('smooth_changes.start', 'Keys of <start_props> != keys of <end_props> (%s != %s)', start_keys, end_keys)
				return
		
		for props in type(end_props)((start_props, )) + end_props:
			for prop in props:
				if prop.endswith('_time_func') and prop[:-len('_time_func')] not in props:
					out_msg('smooth_changes.start', 'Time func for unknown prop: %s', prop)
		
		
		params = smooth_changes.params[name] = {
			'res_props': res_props,
			'start_time': 0,
			'real_start_time': get_game_time(),
			'index': 0,
		}
		
		time_points = params['time_points'] = []
		for i in range(len(end_props)):
			props = []
			time_points.append((props, max(change_time[i], 0.001), on_end[i]))
			
			prev_props = start_props if i == 0 else end_props[i - 1]
			next_props = end_props[i]
			
			for prop in start_keys:
				time_func = next_props.get(prop + '_time_func', linear)
				props.append((prop, prev_props[prop], next_props[prop], time_func))
	
	
	def smooth_changes__update(name):
		params = smooth_changes.params.get(name)
		if not params: return
		
		props, change_time, on_end = params['time_points'][params['index']]
		
		if not params['start_time']:
			dtime = 0
			if params['real_start_time'] != get_game_time():
				params['start_time'] = get_game_time() # set start_time in next frame relatively calling start()
		else:
			dtime = get_game_time() - params['start_time']
		k = min(dtime / change_time, 1)
		
		res_props = params['res_props']
		for prop, prev_value, next_value, time_func in props:
			res_props[prop] = interpolate(prev_value, next_value, time_func(k))
		
		if k == 1:
			params['index'] += 1
			if params['index'] == len(params['time_points']):
				del smooth_changes.params[name]
			
			if on_end:
				on_end()
			params['start_time'] = 0
	
	def smooth_changes__ended(name):
		return name not in smooth_changes.params
	
	
	build_object('smooth_changes')
	smooth_changes.params = {}
