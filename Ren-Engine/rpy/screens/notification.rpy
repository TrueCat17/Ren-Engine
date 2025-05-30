init -980 python:
	def notification__out(msg, *args):
		if callable(msg):
			if not is_picklable_func('notification.out', msg, 'msg'):
				return
			
			obj = [msg, '', tuple(args), None, 1.0]
		else:
			obj = [None, str(msg) % tuple(args), None, None, 1.0]
		
		show_screen('notification')
		notification.msgs.append(obj)
	
	def notification__update():
		now = get_game_time()
		
		for obj in notification.msgs.copy():
			msg_func, msg, args, start_time, alpha = obj
			
			if msg_func:
				new_msg = msg_func()
				if new_msg is not None:
					obj[1] = str(new_msg) % args
				else:
					start_time = now - notification.show_time
					obj[:] = [None, msg, None, start_time, 1.0]
			else:
				if start_time is None:
					start_time = obj[3] = now
			
			dtime = (now - start_time) if start_time is not None else 0
			
			if dtime < notification.show_time:
				alpha = 1
			elif dtime < notification.show_time + notification.hiding_time:
				alpha = 1 - (dtime - notification.show_time) / notification.hiding_time
			else:
				alpha = 0
			
			if alpha:
				obj[4] = alpha
			else:
				notification.msgs.remove(obj)
		
		if not notification.msgs:
			hide_screen('notification')
	
	
	def notification__remove(index):
		notification.msgs.pop(index)
	
	
	build_object('notification')
	
	notification.align = (0.0, 0.0)
	notification.spacing = 10
	
	notification.msgs = []
	notification.show_time = 4
	notification.hiding_time = 1


init:
	style notification is textbutton:
		ground im.rect('#222')
		hover  im.rect('#222')
		corner_sizes 0
		
		size (0.33, 0.22)
		size_min (300, 111)
		size_max (500, 185)
		
		text_size 0.035
		text_size_min 18
		text_size_max 30
		
		color '#08F'
		hover_color '#4BF'


screen notification:
	zorder 1000000
	ignore_modal True
	
	has vbox
	spacing notification.spacing
	align   notification.align
	
	python:
		notification.update()
		
		screen_tmp = SimpleObject()
		screen_tmp.msgs = notification.msgs.copy()
		screen_tmp.range_iter = iter if notification.align[1] > 0.5 else reversed
	
	for i in screen_tmp.range_iter(range(len(screen_tmp.msgs))):
		textbutton _(screen_tmp.msgs[i][1]):
			style 'notification'
			alpha  screen_tmp.msgs[i][4]
			action notification.remove(i)
