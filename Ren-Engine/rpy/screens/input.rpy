init -995 python:
	
	def input__ask_str(callback, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = True, cancel_btn = True):
		if not is_picklable_func('input.ask_str', callback, 'callback'):
			return
		
		db.skip_tab = False
		input.show_time = get_game_time()
		
		input.callback = callback
		input.prompt = prompt
		input.allow = allow
		input.exclude = exclude + input.default_exclude
		input.reset_btn = reset_btn
		input.cancel_btn = cancel_btn
		
		if type(mask) is str and len(mask) > 0:
			input.mask = mask[0] # for <password> mode
		else:
			input.mask = None
		
		if length is None:
			length = input.default_length
		if length <= 0:
			length = 1
		input.length = length
		
		input.confirming = default is None
		default = default or ''
		if len(default) > length:
			default = default[:length]
		input.default = default
		input.reset()
		
		if input.confirming:
			input.btns = [
				('Yes', 'input.res = True;  input.ready()'),
				('No',  'input.res = False; input.ready()'),
			]
		else:
			input.btns = [
				('Ok', input.ready),
			]
			if reset_btn:
				input.btns.append(('Reset', input.reset))
			if cancel_btn:
				input.btns.append(('Cancel', input.close))
		
		if input.reverse_btns:
			input.btns = input.btns[::-1]
		
		show_screen('input')
	
	def input__ask_int(callback, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = True, cancel_btn = True):
		input.ask_str(callback, prompt, default, allow or input.int_allow,   exclude, length, mask, reset_btn, cancel_btn)
	def input__ask_float(callback, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = True, cancel_btn = True):
		input.ask_str(callback, prompt, default, allow or input.float_allow, exclude, length, mask, reset_btn, cancel_btn)
	
	def input__confirm(action, msg):
		input.ask_str(action, msg, default = None, reset_btn = False, cancel_btn = False)
	
	
	def input__get_masked():
		if input.mask:
			return input.mask * len(input.res)
		return input.res
	def input__get_text():
		cursor = '{alpha=%s}%s{/alpha}' % (1 if time.time() % 2 < 1 else 0.01, input.cursor)
		text = input.get_masked()
		text_before = text[:input.index].replace('{', '{{')
		text_after  = text[input.index:].replace('{', '{{')
		return text_before + cursor + text_after
	
	
	def input__close():
		set_timeout(HideScreen('input'), 0.1)
	def input__ready():
		if get_game_time() - input.show_time > 0.5:
			input.close()
			input.callback(input.res)
	def input__reset():
		input.res = input.default
		input.index = len(input.res)
	
	def input__add(s):
		old_index = input.index
		
		input.res, x, _y, add_index = text_nav.add(
			input.res, input.index, 0,
			s, input.index, hotkeys.ctrl, hotkeys.shift,
		)
		
		if add_index:
			input.index += add_index
		else:
			input.index = x
		
		i = input.index - 1
		while old_index <= i:
			s = input.res[i]
			if ((s not in input.allow) if input.allow else (s in input.exclude)) or len(input.res) > input.length:
				input.res = input.res[:i] + input.res[i + 1:]
				input.index -= 1
			i -= 1
	
	
	def input__cursor_home():
		input.index = 0
	def input__cursor_end():
		input.index = len(input.res)
	
	
	def input__cursor_left():
		input.index = text_nav.cursor_left(input.res, input.index, hotkeys.ctrl)
	def input__cursor_right():
		input.index = text_nav.cursor_right(input.res, input.index, hotkeys.ctrl)
	
	
	def input__backspace():
		start_index = input.index
		input.cursor_left()
		index = input.index
		input.res = input.res[:index] + input.res[start_index:]
	def input__delete():
		start_index = input.index
		input.cursor_right()
		index = input.index
		input.res = input.res[:start_index] + input.res[index:]
		input.index = start_index
	
	
	def input__update():
		w, h = get_stage_size()
		spacing = screen_tmp.spacing = get_absolute(input.spacing, h)
		
		input_text_size = style.input_text.get_current('text_size')
		prompt_text_size = style.input_prompt.get_current('text_size')
		
		if input.tf_bg_width:
			screen_tmp.tf_bg_xsize = get_absolute(input.tf_bg_width, w)
		else:
			screen_tmp.tf_bg_xsize = get_text_width('a' * input.length, input_text_size) + input.tf_xindent * 2
		
		if input.tf_bg_height:
			screen_tmp.tf_bg_ysize = get_absolute(input.tf_bg_height, h)
		else:
			screen_tmp.tf_bg_ysize = input_text_size + input.tf_yindent * 2
		
		if input.bg_width:
			screen_tmp.bg_xsize = get_absolute(input.bg_width, w)
		else:
			prompt_xsize = get_text_width(input.prompt, prompt_text_size)
			
			btns = len(input.btns)
			btns_xsize = style.input_button.get_current('xsize') * btns + spacing * (btns - 1)
			
			screen_tmp.bg_xsize = max(prompt_xsize, screen_tmp.tf_bg_xsize, btns_xsize) + spacing * 2
		
		if input.bg_height:
			screen_tmp.bg_ysize = get_absolute(input.bg_height, h)
		else:
			screen_tmp.bg_ysize = style.input_button.get_current('ysize') + spacing * 2
			if not input.confirming:
				screen_tmp.bg_ysize += screen_tmp.tf_bg_ysize + spacing
			if input.prompt:
				screen_tmp.bg_ysize += prompt_text_size + spacing
		
		def fix(name, max_value):
			value = screen_tmp[name]
			
			orig_name = name.replace('xsize', 'width').replace('ysize', 'height')
			
			min_res = input.get(orig_name + '_min')
			if min_res is not None:
				value = max(value, get_absolute(min_res, max_value))
			max_res = input.get(orig_name + '_max')
			if max_res is not None:
				value = min(value, get_absolute(max_res, max_value))
			
			screen_tmp[name] = value
		
		fix('tf_bg_xsize', w)
		fix('tf_bg_ysize', h)
		fix('bg_xsize', w)
		fix('bg_ysize', h)
	
	
	build_object('input')
	
	input.cursor = '|'
	
	input.default_exclude = '\r\n\t'
	input.default_length = 32
	
	input.int_allow = '+-0123456789' # 3, -256, +7
	input.float_allow = input.int_allow + '.eE' # 3.14, 2e5 (200000), 1.5E-2 (0.015)

init:
	$ hotkeys.disable_on_screens.append('input')
	
	
	
	python:
		input.fog = im.rect('#0005')
		input.fog_corner_sizes = -1
		
		input.bg = im.rect('#FFF')
		input.bg_corner_sizes = -1
		input.bg_width  = None # None = auto
		input.bg_height = None # None = auto
		
		input.bg_border = im.rect('#222')
		input.bg_border_corner_sizes = -1
		input.bg_border_size = 4 # 0 - disable
		
		# tf = text field
		input.tf_bg = input.bg
		input.tf_bg_corner_sizes = -1
		input.tf_bg_width  = None # None = auto
		input.tf_bg_height = None # None = auto
		
		input.tf_bg_border = input.bg_border
		input.tf_bg_border_corner_sizes = -1
		input.tf_bg_border_size = 2 # 0 - disable
		
		# indent from tf to tf_border
		input.tf_xindent = 5
		input.tf_yindent = 3
		
		input.spacing = 0.05
		
		input.xalign = 0.5
		input.yalign = 0.5
		
		input.reverse_btns = False # True - <Ok> on the right, False - <Ok> on the left
	
	
	style input_button is textbutton:
		xsize 100
	
	style input_text is text:
		font 'Consola'
		color 0
		text_size 24
		text_align 'center'
	
	style input_prompt is input_text:
		xalign 0.5



screen input:
	modal True
	zorder 1000000
	
	$ screen_tmp = SimpleObject()
	
	if not input.confirming:
		key 'ESCAPE' action (input.close if input.cancel_btn else pause_screen.show)
		
		key 'SPACE' action input.add(' ')
		
		for key in text_nav.keys:
			key key action input.add(key)
		
		key 'HOME'      action input.cursor_home
		key 'END'       action input.cursor_end
		
		$ allow_arrows()
		key 'LEFT'      action input.cursor_left
		key 'RIGHT'     action input.cursor_right
		key 'UP'        action input.cursor_home
		key 'DOWN'      action input.cursor_end
		
		key 'BACKSPACE' action input.backspace
		key 'DELETE'    action input.delete
		
		key 'RETURN'    action input.ready
	
	
	button:
		ground input.fog
		hover  input.fog
		corner_sizes input.fog_corner_sizes
		size 1.0
		
		mouse False
		action input.close if input.cancel_btn else None
	
	$ input.update()
	
	null:
		xsize screen_tmp.bg_xsize + input.bg_border_size * 2
		ysize screen_tmp.bg_ysize + input.bg_border_size * 2
		xalign input.xalign
		yalign input.yalign
		
		if input.bg_border_size:
			image input.bg_border:
				corner_sizes input.bg_border_corner_sizes
				xsize screen_tmp.bg_xsize + input.bg_border_size * 2
				ysize screen_tmp.bg_ysize + input.bg_border_size * 2
		
		image input.bg:
			corner_sizes input.bg_corner_sizes
			xsize screen_tmp.bg_xsize
			ysize screen_tmp.bg_ysize
			align 0.5
			
			use input_content


screen input_content:
	has vbox
	align 0.5
	spacing screen_tmp.spacing
	
	text _(input.prompt):
		style 'input_prompt'
	
	if not input.confirming:
		null:
			xalign 0.5
			xsize screen_tmp.tf_bg_xsize + input.tf_bg_border_size * 2
			ysize screen_tmp.tf_bg_ysize + input.tf_bg_border_size * 2
			
			if input.tf_bg_border_size:
				image input.tf_bg_border:
					corner_sizes input.tf_bg_border_corner_sizes
					xsize screen_tmp.tf_bg_xsize + input.tf_bg_border_size * 2
					ysize screen_tmp.tf_bg_ysize + input.tf_bg_border_size * 2
			
			image input.tf_bg:
				corner_sizes input.tf_bg_corner_sizes
				xsize screen_tmp.tf_bg_xsize
				ysize screen_tmp.tf_bg_ysize
				align 0.5
				
				text input.get_text():
					style 'input_text'
					xpos  input.tf_xindent
					ypos  input.tf_yindent
					xsize screen_tmp.tf_bg_xsize
					ysize screen_tmp.tf_bg_ysize
	
	hbox:
		xalign 0.5
		spacing screen_tmp.spacing
		
		for text, action in input.btns:
			textbutton _(text):
				style 'input_button'
				action action
