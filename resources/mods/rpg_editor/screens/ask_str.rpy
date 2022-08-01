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
	ask_empty_is_ok = False
	callback_ask_str = None
	def ask_str(func, default_res = '', empty_is_ok = False):
		global callback_ask_str, ask_res, ask_cursor, ask_empty_is_ok
		callback_ask_str = func
		ask_res = default_res
		ask_cursor = len(ask_res)
		ask_empty_is_ok = empty_is_ok
		
		show_screen('ask_str')
	
	def ask_exit():
		hide_screen('ask_str')
	def ask_ready():
		if ask_res or ask_empty_is_ok:
			ask_exit()
			callback_ask_str(ask_res)
	
	def ask_add(s):
		global ask_res, ask_cursor
		if s == '-' and ask_shift:
			s = '_'
		
		if ask_ctrl:
			if s == 'c':
				set_clipboard_text(ask_res)
				return
			if s == 'v':
				s = get_clipboard_text()
				i = 0
				while i < len(s):
					if s[i] not in ask_keys:
						s = s[0:i] + s[i+1:]
					else:
						i += 1
			if s == 'd':
				ask_res = ''
				ask_cursor = 0
				return
		
		ask_res = ask_res[0:ask_cursor] + s + ask_res[ask_cursor:]
		ask_cursor += len(s)
	
	def ask_cursor_left():
		global ask_cursor
		if ask_cursor == 0:
			return
		ask_cursor -= 1
		if ask_ctrl:
			while ask_cursor > 0 and ask_res[ask_cursor - 1].isalnum():
				ask_cursor -= 1
	def ask_cursor_right():
		global ask_cursor
		if ask_cursor == len(ask_res):
			return
		ask_cursor += 1
		if ask_ctrl:
			while ask_cursor < len(ask_res) and ask_res[ask_cursor].isalnum():
				ask_cursor += 1
	
	def ask_cursor_home():
		global ask_cursor
		ask_cursor = 0
	def ask_cursor_end():
		global ask_cursor
		ask_cursor = len(ask_res)
	
	def ask_backspace():
		prev_cursor = ask_cursor
		ask_cursor_left()
		global ask_res
		ask_res = ask_res[0:ask_cursor] + ask_res[prev_cursor:]
	def ask_delete():
		global ask_cursor
		prev_cursor = ask_cursor
		ask_cursor_right()
		global ask_res
		ask_res = ask_res[0:prev_cursor] + ask_res[ask_cursor:]
		ask_cursor = prev_cursor
	
	hotkeys.disable_on_screens.append('ask_str')


screen ask_str:
	key 'ESCAPE' action ask_exit
	
	$ ask_ctrl  = False
	key 'LEFT CTRL'   action SetVariable('ask_ctrl', True) first_delay 0
	key 'RIGHT CTRL'  action SetVariable('ask_ctrl', True) first_delay 0
	$ ask_shift = False
	key 'LEFT SHIFT'  first_delay 0.01 action SetVariable('ask_shift', True)
	key 'RIGHT SHIFT' first_delay 0.01 action SetVariable('ask_shift', True)
	
	if len(ask_res) < ask_max_len:
		for key in ask_keys:
			key key action ask_add(key)
	
	key 'HOME'      action ask_cursor_home
	key 'END'       action ask_cursor_end
	
	key 'LEFT'      action ask_cursor_left
	key 'RIGHT'     action ask_cursor_right
	key 'UP'        action ask_cursor_home
	key 'DOWN'      action ask_cursor_end
	
	key 'BACKSPACE' action ask_backspace
	key 'DELETE'    action ask_delete
	
	key 'RETURN'    action ask_ready
	
	
	modal True
	zorder 10000
	
	button:
		ground 'images/bg/black.jpg'
		hover  'images/bg/black.jpg'
		
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
					
					$ cursor = '{alpha=' + ('1' if time.time() % 2 < 1 else '0') + '}|{/alpha}'
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

