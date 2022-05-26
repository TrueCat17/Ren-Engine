init -980 python:
	def notification__out(msg):
		show_screen('notification')
		notification.msgs.append([str(msg), None, 1.0])
	
	def notification__update():
		i = 0
		while i < len(notification.msgs):
			msg, start_time, alpha = notification.msgs[i]
			if not start_time:
				start_time = notification.msgs[i][1] = get_game_time()
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
	
	notification.size = (400, 150)
	notification.align = (0.0, 0.0)
	
	notification.back = im.rect('#222')
	notification.text_color = 0x0080FF
	notification.text_size = 24
	notification.text_align = 'center'
	notification.text_valign = 'center'
	
	notification.msgs = []
	notification.show_time = 4
	notification.hiding_time = 1


screen notification:
	zorder 1000
	
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
		
		image notification.back:
			size  notification.size
			alpha alpha
			
			text _(msg):
				size      notification.size
				color     notification.text_color
				text_size notification.text_size
				text_align  notification.text_align
				text_valign notification.text_valign
			button:
				ground notification.back
				hover  notification.back
				
				size   notification.size
				alpha  0.01
				action [notification.remove(i), SetVariable('i', i + correcting_i)]
		
		$ i += di

