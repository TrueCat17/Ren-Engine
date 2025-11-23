init python:
	
	def sc_info__set_msg(msg):
		sc_info.msg = msg
	
	def sc_info__set_msg_if_prev(msg, prev):
		if sc_info.msg == prev:
			sc_info.msg = msg
	
	
	def sc_info__get_text_size():
		return min(get_stage_height() // 20, 24)
	
	def sc_info__get_indent():
		return max(get_stage_height() // 60, 10)
	
	def sc_info__get_ysize():
		text_size = sc_info.get_text_size()
		indent = sc_info.get_indent()
		return text_size * 4 + indent * 3
	
	
	sc_info = SimpleObject()
	build_object('sc_info')
	
	sc_info.msg = ''
	sc_info.msg_color = '#FFF'
	sc_info.msg_outlinecolor = '#00F'
	
	
	sc_info.back = im.rect('#555')
	
	sc_info.indent = 10
	sc_info.btn_size = 140
	
	sc_info.simple_resources = ('food', 'wood', 'stone', 'coal', 'metal')
	sc_info.resources = sc_info.simple_resources + ('cement', 'steel', 'science')
	
	sc_info.groups = [
		sc_info.resources[0:3],
		sc_info.resources[3:6],
		sc_info.resources[6:] + ('player', ),
	]


screen sc_info:
	python:
		screen_tmp = SimpleObject()
		screen_tmp.ysize     = sc_info.get_ysize()
		screen_tmp.indent    = sc_info.get_indent()
		screen_tmp.text_size = sc_info.get_text_size()
	
	image sc_info.back:
		xsize 1.0
		ysize screen_tmp.ysize
		yalign 1.0
		
		hbox:
			pos     screen_tmp.indent
			spacing screen_tmp.indent
			
			for group in sc_info.groups:
				$ max_len = max(len(_(resource)) for resource in group)
				
				vbox:
					for resource in group:
						python:
							text = _(resource)
							spaces = ' ' * (max_len - len(text))
							
							if resource != 'player':
								count = sc_map.player[resource]
								changing = sc_map.player['change_' + resource]
								changing_color = '#0F0' if changing >= 0 else '#F00'
								text = ('%s:%s {color=#FA0}%s{/color} ({color=%s}%s{/color})' % (text, spaces, count, changing_color, changing))
							else:
								text = '%s: {image=%s}' % (text, sc_map.player.image)
						
						text text:
							font 'Consola'
							text_size screen_tmp.text_size
							color '#FFF'
							outlinecolor 0
		
		text _(sc_info.msg):
			xpos    screen_tmp.indent
			ypos    screen_tmp.ysize - screen_tmp.indent
			yanchor 1.0
			
			color        sc_info.msg_color
			outlinecolor sc_info.msg_outlinecolor
			text_size    screen_tmp.text_size
