init -990 python:
	
	def info__set_msg(msg, extra = ''):
		info.msg = [msg, extra]
	
	def info__set_msg_if_prev(msg, prev, msg_extra = '', prev_extra = ''):
		if info.msg == [prev, prev_extra]:
			info.msg = [msg, msg_extra]
	
	
	build_object('info')
	
	info.msg = ['', '']
	info.msg_color = 0xFFFFFF
	info.msg_outlinecolor = 0x0000FF
	
	
	info.ysize = 100
	info.back = im.rect('#555')
	
	info.indent = 10
	info.text_size = 20
	info.btn_size = 140
	
	info.simple_resources = ('food', 'wood', 'stone', 'coal', 'metal')
	info.resources = info.simple_resources + ('cement', 'steel', 'science')
	info.group_size = 3
	
	info.groups = []
	info.count_groups = int(math.ceil(len(info.resources) / info.group_size))
	for i in range(info.count_groups):
		group = []
		info.groups.append(group)
		for j in range(info.group_size):
			index = i * info.group_size + j
			if index < len(info.resources):
				group.append(info.resources[index])


screen info:
	python:
		info.ysize = max(get_stage_height() // 5, 100)
		info.indent = min(get_stage_height() // 60, 20)
		info.text_size = min(get_stage_height() // 20, 22)
		info.btn_size = min(int(get_stage_height() * 0.4), 250)
	
	image info.back:
		xsize 1.0
		ysize info.ysize
		yalign 1.0
		
		hbox:
			pos     info.indent
			spacing info.indent
			
			for group in info.groups:
				$ max_len = max(len(_(resource)) for resource in group)
				vbox:
					for resource in group:
						python:
							spaces = ' ' * (max_len - len(_(resource)))
							count = sc_map.player[resource]
							changing = sc_map.player['change_' + resource]
							changing_color = '0x00FF00' if changing >= 0 else '#FF0000'
						
						text ('%s:%s {color=0xFFAA00}%s{/color} ({color=%s}%s{/color})' % (_(resource), spaces, count, changing_color, changing)):
							text_size info.text_size
							color 0xFFFFFF
							outlinecolor 0
							font 'Consola'
		
		python:
			text = _(info.msg[0])
			if info.msg[1]:
				text += ' (%s)' % _(info.msg[1])
		text text:
			xpos    info.indent
			ypos    info.ysize - info.indent
			yanchor 1.0
			xsize   get_stage_width() - info.btn_size - 2 * info.indent
			
			color        info.msg_color
			outlinecolor info.msg_outlinecolor
			text_size    info.text_size
		
		vbox:
			xpos    get_stage_width() - info.indent
			xanchor 1.0
			yalign  0.5
			spacing info.indent
			
			textbutton _('Research'):
				style     'btn'
				xsize     info.btn_size
				text_size info.text_size
				hovered   info.set_msg('F2')
				unhovered info.set_msg_if_prev('', 'F2')
				action show_screen('research')
			key 'F2' action show_screen('research')
			
			textbutton _('Help'):
				style     'btn'
				xsize     info.btn_size
				text_size info.text_size
				hovered   info.set_msg('F1')
				unhovered info.set_msg_if_prev('', 'F1')
				action    show_screen('help')
			key 'F1' action show_screen('help')
			
			textbutton _('Pause'):
				style     'btn'
				xsize     info.btn_size
				text_size info.text_size
				hovered   info.set_msg('Esc')
				unhovered info.set_msg_if_prev('', 'Esc')
				action pause_screen.show()
