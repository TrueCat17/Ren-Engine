init -980 python:
	def notification__out(msg):
		if callable(msg):
			if not is_picklable_func('notification.out', msg, 'msg'):
				return
			
			obj = [msg, '', None, 1.0]
		else:
			obj = [None, str(msg), None, 1.0]
		
		show_screen('notification')
		notification.msgs.append(obj)
	
	def notification__update():
		now = get_game_time()
		
		i = 0
		while i < len(notification.msgs):
			msg_func, msg, start_time, alpha = obj = notification.msgs[i]
			if msg_func:
				new_msg = msg_func()
				if new_msg is not None:
					obj[1] = str(new_msg)
				else:
					start_time = now - notification.show_time
					obj[:] = [None, msg, start_time, 1.0]
			else:
				if start_time is None:
					start_time = obj[2] = now
			
			dtime = (now - start_time) if start_time is not None else 0
			
			if dtime < notification.show_time:
				alpha = 1
			elif dtime < notification.show_time + notification.hiding_time:
				alpha = 1 - (dtime - notification.show_time) / notification.hiding_time
			else:
				alpha = 0
			
			if alpha:
				obj[3] = alpha
				i += 1
			else:
				notification.msgs.pop(i)
	
	def notification__remove(index):
		notification.msgs.pop(index)
	
	
	build_object('notification')
	
	notification.align = (0.0, 0.0)
	
	notification.msgs = []
	notification.show_time = 4
	notification.hiding_time = 1


init:
	style notification is textbutton:
		ground im.rect('#222')
		hover  im.rect('#222')
		corner_sizes 0
		size (400, 150)
		text_size 24
		color 0x0080FF


screen notification:
	zorder 1000000
	
	has vbox
	spacing 10
	align notification.align
	
	python:
		notification.update()
		
		if notification.align[1] > 0.5:
			i = 0
			di = 1
			correcting_i = -1
		else:
			i = len(notification.msgs) - 1
			di = -1
			correcting_i = 0
	
	while True:
		if di > 0:
			if i >= len(notification.msgs):
				break
		else:
			if i < 0:
				break
		
		$ msg, start_time, alpha = notification.msgs[i][1:]
		
		textbutton _(msg):
			style 'notification'
			alpha  alpha
			action [notification.remove(i), AddVariable('i', correcting_i)]
		
		$ i += di

