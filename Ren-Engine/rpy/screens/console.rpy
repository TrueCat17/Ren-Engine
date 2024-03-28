init -995 python:
	def console__get_cursor_index():
		lines = console.input.split('\n')
		index = min(console.cursor_x, len(lines[console.cursor_y]))
		for y in range(console.cursor_y):
			index += len(lines[y]) + 1
		return index
	def console__set_cursor_index(index):
		input_slice = console.input[:index]
		console.cursor_y = input_slice.count('\n')
		console.cursor_x = (index - input_slice.rindex('\n') - 1) if console.cursor_y else index
	
	def console__get_input_text():
		index = console.get_cursor_index()
		alpha_cursor = '{alpha=' + ('1' if get_game_time() % 2 < 1 else '0.01') + '}' + console.cursor + '{/alpha}'
		res = console.input[:index] + alpha_cursor + console.input[index:]
		return res.replace(text_nav.key_tag, '{{')
	
	def console__clear():
		persistent.console_commands = []
		persistent.console_text = 'Press Esc to exit, type help for help'
	
	if persistent.console_commands is None or not persistent.console_text:
		console__clear()
	
	
	def console__out(text):
		persistent.console_text += '\n' + str(text).replace('{', '{{')
	def console__out_help():
		console.out(console.help)
	
	
	def console__add(s):
		def remove_empty_lines(s):
			lines = s.split('\n')
			lines = filter(lambda i: i and not i.isspace(), lines)
			return '\n'.join(lines)
		
		index = console.get_cursor_index()
		console.input, console.cursor_x, console.cursor_y, add_index = text_nav.add(
			console.input, console.cursor_x, console.cursor_y,
			s, index, hotkeys.ctrl, hotkeys.shift,
			paste_processor = remove_empty_lines,
		)
		
		if add_index:
			console.set_cursor_index(index + add_index)
		
		if not console.num_command:
			console.input_tmp = console.input
	
	def console__cursor_home():
		console.cursor_x = 0
		if hotkeys.ctrl:
			console.cursor_y = 0
	def console__cursor_end():
		lines = console.input.split('\n')
		if hotkeys.ctrl:
			console.cursor_y = len(lines) - 1
		console.cursor_x = len(lines[console.cursor_y])
	
	
	def console__cursor_left():
		index = console.get_cursor_index()
		index = text_nav.cursor_left(console.input, index, hotkeys.ctrl)
		console.set_cursor_index(index)
	def console__cursor_right():
		index = console.get_cursor_index()
		index = text_nav.cursor_right(console.input, index, hotkeys.ctrl)
		console.set_cursor_index(index)
	
	def console__cursor_up():
		if console.cursor_y > 0:
			console.cursor_y -= 1
		elif console.cursor_y == 0 and (console.cursor_x > 0 and console.input):
			console.cursor_x = 0
		elif console.num_command < len(persistent.console_commands):
			console.num_command += 1
			console.input = persistent.console_commands[-console.num_command]
			lines = console.input.split('\n')
			console.cursor_y = len(lines) - 1
			console.cursor_x = len(lines[-1])
	def console__cursor_down():
		lines = console.input.split('\n')
		if console.cursor_y < len(lines) - 1:
			console.cursor_y += 1
		elif console.cursor_y == len(lines) - 1 and console.cursor_x < len(lines[-1]):
			console.cursor_x = len(lines[-1])
		elif console.num_command > 1:
			console.num_command -= 1
			console.input = persistent.console_commands[-console.num_command]
			console.cursor_y = 0
			console.cursor_x = console.input.index('\n') if '\n' in console.input else len(console.input)
		elif console.input != console.input_tmp:
			console.num_command = 0
			console.cursor_x = console.cursor_y = 0
			console.input = console.input_tmp
	
	def console__backspace():
		start_index = console.get_cursor_index()
		console.cursor_left()
		index = console.get_cursor_index()
		console.input = console.input[:index] + console.input[start_index:]
		if not console.num_command:
			console.input_tmp = console.input
	def console__delete():
		start_index = console.get_cursor_index()
		console.cursor_right()
		index = console.get_cursor_index()
		console.input = console.input[:start_index] + console.input[index:]
		console.set_cursor_index(start_index)
		if not console.num_command:
			console.input_tmp = console.input
	
	def console__on_enter():
		if console.input.isspace():
			console.input = ''
			return
		
		lines = console.input.split('\n')
		cur = lines[console.cursor_y]
		
		cur_indent = 0
		while cur_indent < len(cur) and cur[cur_indent] == ' ':
			cur_indent += 1
		
		if cur.endswith(':') and console.cursor_x == len(cur):
			lines.insert(console.cursor_y + 1, ' ' * (cur_indent + 4))
			console.cursor_y += 1
			console.cursor_x = cur_indent + 4
			console.input = '\n'.join(lines)
		elif console.cursor_y == len(lines) - 1 and (not cur_indent or cur.isspace()):
			console.cursor_x = 0
			console.cursor_y = 0
			console.num_command = 0
			to_exec = console.input
			console.input = console.input_tmp = ''
			if not persistent.console_commands or persistent.console_commands[-1] != to_exec:
				persistent.console_commands += [to_exec]
			console.execute(to_exec.replace(text_nav.key_tag, '{'))
		else:
			index = console.get_cursor_index()
			console.input = console.input[:index] + '\n' + ' ' * cur_indent + console.input[index:]
			console.cursor_y += 1
			console.cursor_x = cur_indent
	
	def console__except_error(to_exec):
		try:
			res = to_exec()
		except Exception as e:
			msg = get_exception_stack_str(e, depth = 2) # cut first (console.execute) and second (console.except_error) funcs
			console.out(msg)
			res = None
		return res
	
	def console__execute(command):
		command = command.strip()
		if not command:
			return
		
		lines = command.split('\n')
		for i in range(len(lines)):
			pre = '... ' if i else '>>> '
			console.out(pre + lines[i])
		
		if ' ' in command:
			index = command.index(' ')
			params = command[index+1:].lstrip()
			command = command[:index]
		else:
			params = ''
		
		if command == 'exit':
			hide_screen('console')
		elif command == 'clear':
			console.clear()
		elif command == 'help':
			console.out_help()
		
		elif command == 'watch':
			console.watch_add(params)
		elif command == 'unwatch':
			console.watch_del(params)
		elif command == 'unwatchall':
			console.watch_clear()
		
		elif command == 'jump':
			renpy.jump(params)
		elif command == 'call':
			renpy.call(params)
		
		elif command in ('scene', 'show', 'hide'):
			args = get_args(params)
			
			if command == 'scene':
				sprites.set_scene(args, [])
			elif command == 'show':
				sprites.show(args, [])
			elif command == 'hide':
				sprites.hide(args)
		
		else:
			code = command + ' ' + params
			try:
				cmpl = compile(code, 'Console', 'eval')
			except:
				to_exec_for_compile = Function(compile, code, 'Console', 'exec')
				cmpl = console.except_error(to_exec_for_compile)
			
			if cmpl is not None:
				to_exec = Function(eval, cmpl, globals(), globals())
				res = console.except_error(to_exec)
				if res is not None:
					if type(res) is str:
						res = repr(res)
					console.out(res)
	
	
	def console__watch_add(code):
		eval_obj = Eval(code, 'Console', 1, depth = 1)
		if not eval_obj.compiled:
			return
		
		if not console.to_watch:
			show_screen('console_watching')
		console.to_watch.append((code, eval_obj))
	
	def console__watch_del(code_to_del):
		for code, cmpl in console.to_watch:
			if code == code_to_del:
				console.to_watch.remove((code, cmpl))
				if not console.to_watch:
					hide_screen('console_watching')
				break
		else:
			console.out('<' + code_to_del + '> not watched')
	
	def console__watch_clear():
		console.to_watch = []
		
		if has_screen('console_watching'):
			hide_screen('console_watching')
	
	
	def console__show():
		if not has_screen('console'):
			db.skip_tab = False
			console.showed_time = get_game_time()
			show_screen('console')
	def console__hide():
		if has_screen('console'):
			hide_screen('console')
			pause_screen.hided_time = get_game_time()
	
	def console__update_watching_text():
		xsize_max_real = get_absolute(console.watching_xsize_max, get_stage_width()) - console.watching_xindent * 2
		text_size = style.console_text.text_size
		
		text = []
		xsize_max = 0
		ysize = 0
		for code, eval_obj in console.to_watch:
			res_start = code + ': '
			try:
				res_end = str(eval_obj()).replace('{', '{{')
			except:
				res_end = 'Eval Failed'
			text.append(res_start + '{color=' + console.watching_calced_color + '}' + res_end + '{/color}')
			
			xsize = get_text_width(res_start + res_end, text_size)
			if xsize > 0:
				ysize += int(math.ceil(xsize / xsize_max_real)) * text_size
			xsize_max = max(xsize, xsize_max)
		
		console.watching_text = text
		console.watching_xsize = min(xsize_max, xsize_max_real)
		console.watching_ysize = ysize
	
	
	build_object('console')
	console.help = 'commands: clear, jump, call, scene, show, hide, watch <expr>, unwatch <expr>, unwatchall or python-expr'
	
	console.to_watch = []
	
	console.background = im.rect('#000')
	console.background_alpha = 0.15
	
	console.input = ''
	console.input_tmp = ''
	
	console.cursor = '|'
	console.cursor_x = 0
	console.cursor_y = 0
	console.num_command = 0

init:
	$ hotkeys.disable_on_screens.append('console')
	
	
	
	style console_text is text:
		font         'Consola'
		color        0xFFFFFF
		outlinecolor 0
		text_size    20
	
	python:
		console.watching_image = im.rect('#0006')
		console.watching_xalign = 1.0
		console.watching_yalign = 0.0
		console.watching_xoffset = -30
		console.watching_yoffset = +30
		console.watching_xindent = 20
		console.watching_yindent = 20
		console.watching_xsize_max = 0.5
		console.watching_calced_color = '#FFFF00'



screen console_watching:
	zorder 10001
	
	image console.watching_image:
		xanchor console.watching_xalign
		yanchor console.watching_yalign
		
		xpos get_absolute(console.watching_xalign, get_stage_width())  + console.watching_xoffset
		ypos get_absolute(console.watching_yalign, get_stage_height()) + console.watching_yoffset
		
		$ console.update_watching_text()
		xsize console.watching_xsize + console.watching_xindent * 2
		ysize console.watching_ysize + console.watching_yindent * 2
		
		vbox:
			align 0.5
			
			for text in console.watching_text:
				text text:
					style 'console_text'
					xsize console.watching_xsize


screen console:
	modal True
	zorder 10002
	
	key 'ESCAPE' action console.hide
	
	key 'SPACE' action console.add(' ')
	
	if get_game_time() - console.showed_time > 0.333:
		for key in text_nav.keys:
			key key action console.add(key)
	
	key 'HOME'      action console.cursor_home
	key 'END'       action console.cursor_end
	
	key 'LEFT'      action console.cursor_left
	key 'RIGHT'     action console.cursor_right
	key 'UP'        action console.cursor_up
	key 'DOWN'      action console.cursor_down
	
	key 'BACKSPACE' action console.backspace
	key 'DELETE'    action console.delete
	
	key 'RETURN'    action console.on_enter
	
	
	python:
		console.input_height = style.console_text.text_size * console.input.count('\n')
		
		console.output_height = get_stage_height() - console.input_height
		console.output_lines = console.output_height // style.console_text.text_size
		
		console.output = '\n'.join(persistent.console_text.split('\n')[-console.output_lines:])
	
	image console.background:
		alpha console.background_alpha
		size 1.0
	
	vbox:
		align (0.5, 1.0)
		
		text console.output:
			style 'console_text'
			xpos  50
			xsize get_stage_width() - 50
		
		hbox:
			text ('...' if '\n' in console.input else '>>>'):
				style 'console_text'
				xsize 50
			
			text console.get_input_text():
				style 'console_text'
				xsize get_stage_width() - 50

