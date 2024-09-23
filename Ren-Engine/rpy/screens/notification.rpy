init -980 python:
	def notification__out(msg):
		obj = [str(msg), None, 1.0]
		
		show_screen('notification')
		notification.msgs.append(obj)
		
		signals.add('enter_frame', SetDictFuncRes(obj, 1, get_game_time), times = 1)
	
	def notification__update():
		i = 0
		while i < len(notification.msgs):
			msg, start_time, alpha = notification.msgs[i]
			if start_time is None:
				i += 1
				continue
			dtime = get_game_time() - start_time
			
			if dtime < notification.show_time:
				alpha = 1
			elif dtime < notification.show_time + notification.hiding_time:
				alpha = 1 - (dtime - notification.show_time) / notification.hiding_time
			else:
				alpha = 0
			
			if alpha:
				notification.msgs[i][2] = alpha
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
		
		$ msg, start_time, alpha = notification.msgs[i]
		
		textbutton _(msg):
			style 'notification'
			alpha  alpha
			action [notification.remove(i), AddVariable('i', correcting_i)]
		
		$ i += di

