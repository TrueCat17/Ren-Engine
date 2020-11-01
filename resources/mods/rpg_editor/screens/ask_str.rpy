init python:
	ask_str_borders = im.Rect('#222')
	ask_str_bg = im.Rect('#FFF')
	
	ask_str_borders_size = (350, 150)
	ask_str_bg_size = (340, 140)
	
	ask_str_tf_border_size = (250, 25)
	ask_str_tf_bg_size = (246, 20)
	
	ask_max_len = 25
	ask_keys = alphabet + [str(i) for i in xrange(10)] + ['-']
	
	ask_cursor = 0
	ask_shift = False
	
	ask_res = ''
	callback_ask_str = None
	def ask_str(func, default_res = ''):
		global callback_ask_str, ask_res, ask_cursor
		callback_ask_str = func
		ask_res = default_res
		ask_cursor = len(ask_res)
		
		show_screen('ask_str')
	
	def ask_exit():
		hide_screen('ask_str')
	def ask_ready():
		if ask_res:
			ask_exit()
			callback_ask_str(ask_res)
	
	def ask_add(s):
		global ask_res
		if s == '-' and ask_shift:
			s = '_'
		ask_res = ask_res[0:ask_cursor] + s + ask_res[ask_cursor:]
		ask_cursor_right()
	
	def ask_cursor_left():
		global ask_cursor
		ask_cursor = max(ask_cursor - 1, 0)
	def ask_cursor_right():
		global ask_cursor
		ask_cursor = min(ask_cursor + 1, len(ask_res))
	
	def ask_backspace():
		if ask_cursor:
			global ask_res
			ask_res = ask_res[0:ask_cursor - 1] + ask_res[ask_cursor:]
			ask_cursor_left()
	def ask_delete():
		global ask_res
		ask_res = ask_res[0:ask_cursor] + ask_res[ask_cursor+1:]


screen ask_str:
	key 'ESCAPE' action ask_exit
	
	$ ask_shift = False
	key 'LEFT SHIFT'  first_delay 0.01 action SetVariable('ask_shift', True)
	key 'RIGHT SHIFT' first_delay 0.01 action SetVariable('ask_shift', True)
	
	if len(ask_res) < ask_max_len:
		for key in ask_keys:
			key key action ask_add(key)
	
	key 'LEFT'      action ask_cursor_left
	key 'RIGHT'     action ask_cursor_right
	
	key 'BACKSPACE' action ask_backspace
	key 'DELETE'    action ask_delete
	
	key 'RETURN'    action ask_ready
	
	
	modal True
	zorder 10000
	
	button:
		ground 'images/bg/black.jpg'
		hover 'images/bg/black.jpg'
		
		size (1.0, 1.0)
		alpha 0.3
		mouse False
		
		action ask_exit
	
	image ask_str_borders:
		align (0.5, 0.5)
		size ask_str_borders_size
	
	image ask_str_bg:
		align (0.5, 0.5)
		size ask_str_bg_size
		
		vbox:
			align (0.5, 0.5)
			spacing 10
			
			image ask_str_borders:
				size ask_str_tf_border_size
				
				image ask_str_bg:
					align (0.5, 0.5)
					size ask_str_tf_bg_size
					
					$ cursor = '{color=' + ('#000000' if time.time() % 2 < 1 else '#FFFFFF') + '}|{/color}'
					text (ask_res[0:ask_cursor] + cursor + ask_res[ask_cursor:]):
						color 0
						text_size 20
						align (0.5, 0.5)
			
			hbox:
				spacing 10
				xalign 0.5
				
				textbutton 'Ok':
					size (100, 25)
					text_size 20
					action ask_ready
				textbutton _('Cancel'):
					size (100, 25)
					text_size 20
					action ask_exit

