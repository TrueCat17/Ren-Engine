init -995 python:
	
	def input__ask_str(callback, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = True, cancel_btn = True):
		db.skip_tab = False
		input.show_time = get_game_time()
		
		input.callback = callback
		input.prompt = prompt
		input.allow = allow
		input.exclude = exclude + input.default_exclude
		input.reset_btn = reset_btn
		input.cancel_btn = cancel_btn
		
		if type(mask) is str and len(mask) > 0:
			input.mask = mask[0] # for like password
			if input.mask == '{':
				input.mask = text_nav.key_tag
		else:
			input.mask = None
		
		if length is None:
			length = input.default_length
		if length <= 0:
			length = 1
		input.length = length
		if len(default) > length:
			default = default[:length]
		input.default = default
		input.reset()
		
		input.btns = [('Ok', input.ready)]
		if reset_btn:
			input.btns += [('Reset', input.reset)]
		if cancel_btn:
			input.btns += [('Cancel', input.close)]
		
		if input.reverse_btns:
			input.btns = input.btns[::-1]
		
		show_screen('input')
	
	def input__ask_int(callback, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = True, cancel_btn = True):
		input.ask_str(callback, prompt, default, allow or input.int_allow,   exclude, length, mask, reset_btn, cancel_btn)
	def input__ask_float(callback, prompt, default = '', allow = None, exclude = '', length = None, mask = None, reset_btn = True, cancel_btn = True):
		input.ask_str(callback, prompt, default, allow or input.float_allow, exclude, length, mask, reset_btn, cancel_btn)
	
	
	def input__get_masked():
		if input.mask:
			return input.mask * len(input.res)
		return input.res
	def input__get_text():
		cursor = '{alpha=' + ('1' if time.time() % 2 < 1 else '0.01') + '}' + input.cursor + '{/alpha}'
		text = input.get_masked()
		res = text[:input.index] + cursor + text[input.index:]
		return res.replace(text_nav.key_tag, '{{')
	
	
	def input__close():
		hide_screen('input')
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
		spacing = input.real_spacing = get_absolute(input.spacing, h)
		
		if input.tf_bg_width:
			input.tf_bg_xsize = get_absolute(input.tf_bg_width, w)
		else:
			input.tf_bg_xsize = utf8.width('a' * input.length, style.input_text.text_size) + input.tf_side_indent * 2
		
		if input.tf_bg_height:
			input.tf_bg_ysize = get_absolute(input.tf_bg_height, h)
		else:
			input.tf_bg_ysize = style.input_text.text_size
		
		if input.bg_width:
			input.bg_xsize = get_absolute(input.bg_width, w)
		else:
			prompt_xsize = utf8.width(input.prompt, style.input_prompt.text_size)
			
			btns = len(input.btns)
			btns_xsize = get_absolute(style.input_button.xsize, w) * btns + spacing * (btns - 1)
			
			input.bg_xsize = max(prompt_xsize, input.tf_bg_xsize, btns_xsize) + spacing * 2
		
		if input.bg_height:
			input.bg_ysize = get_absolute(input.bg_height, h)
		else:
			input.bg_ysize = input.tf_bg_ysize + get_absolute(style.input_button.ysize, h) + spacing * 3
			if input.prompt:
				input.bg_ysize += style.input_prompt.text_size + spacing
	
	
	build_object('input')
	
	input.cursor = '|'
	
	input.default_exclude = '\r\n\t'
	input.default_length = 32
	
	input.int_allow = '+-0123456789' # 3, -256, +7
	input.float_allow = input.int_allow + '.eE' # 3.14, 2e5 (200000), 1.5E-2 (0.015)

init:
	$ hotkeys.disable_on_screens.append('input')
	
	
	
	python:
		input.fog = im.rect('#000')
		input.fog_alpha = 0.3
		
		
		input.bg = im.rect('#FFF')
		input.bg_width = None # None = auto
		input.bg_height = None # None = auto
		
		input.bg_border = im.rect('#222')
		input.bg_border_size = 4 # 0 - disable
		
		# tf = text field
		input.tf_bg = input.bg
		input.tf_bg_width = None # None = auto
		input.tf_bg_height = None # None = auto
		
		input.tf_bg_border = input.bg_border
		input.tf_bg_border_size = 2 # 0 - disable
		
		input.tf_side_indent = 5 # from left and right sides to border
		
		input.spacing = 0.05
		
		input.reverse_btns = False # True - <Ok> on the right, False - <Ok> on the left
	
	
	style input_button is textbutton:
		size (100, 25)
	
	style input_text is text:
		font 'Consola'
		color 0x000000
		text_size 20
		text_align 'center'
	
	style input_prompt is input_text:
		xalign 0.5



screen input:
	modal True
	zorder 100
	
	
	key 'ESCAPE' action (input.close if input.cancel_btn else pause_screen.show)
	
	key 'SPACE' action input.add(' ')
	
	for key in text_nav.keys:
		key key action input.add(key)
	
	key 'HOME'      action input.cursor_home
	key 'END'       action input.cursor_end
	
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
		
		alpha input.fog_alpha
		size 1.0
		
		mouse False
		action input.close if input.cancel_btn else None
	
	$ input.update()
	vbox:
		align 0.5
		
		image input.bg_border:
			xsize input.bg_xsize + input.bg_border_size * 2
			ysize input.bg_border_size
		
		hbox:
			image input.bg_border:
				xsize input.bg_border_size
				ysize input.bg_ysize
			
			image input.bg:
				xsize input.bg_xsize
				ysize input.bg_ysize
				
				use input_content
			
			image input.bg_border:
				xsize input.bg_border_size
				ysize input.bg_ysize
		
		image input.bg_border:
			xsize input.bg_xsize + input.bg_border_size * 2
			ysize input.bg_border_size


screen input_content:
	has vbox
	align 0.5
	spacing input.real_spacing
	
	text _(input.prompt):
		style 'input_prompt'
	
	vbox:
		xalign 0.5
		
		image input.tf_bg_border:
			xsize input.tf_bg_xsize + input.tf_bg_border_size * 2
			ysize input.tf_bg_border_size
		
		hbox:
			image input.tf_bg_border:
				xsize input.tf_bg_border_size
				ysize input.tf_bg_ysize
			
			image input.tf_bg:
				xsize input.tf_bg_xsize
				ysize input.tf_bg_ysize
				
				text input.get_text():
					style 'input_text'
					xpos input.tf_side_indent
					xsize input.tf_bg_xsize
					ysize input.tf_bg_ysize
			
			image input.tf_bg_border:
				xsize input.tf_bg_border_size
				ysize input.tf_bg_ysize
		
		image input.tf_bg_border:
			xsize input.tf_bg_xsize + input.tf_bg_border_size * 2
			ysize input.tf_bg_border_size
	
	hbox:
		xalign 0.5
		spacing input.real_spacing
		
		for text, action in input.btns:
			textbutton _(text):
				style 'input_button'
				action action
