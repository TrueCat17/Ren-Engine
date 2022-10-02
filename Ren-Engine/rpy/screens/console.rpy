init -995 python:
	def console__get_cursor_index():
		lines = console.input.split('\n')
		index = min(console.cursor_x, len(lines[console.cursor_y]))
		for y in xrange(console.cursor_y):
			index += len(lines[y]) + 1
		return index
	def console__set_cursor_index(index):
		input_slice = console.input[0:index]
		console.cursor_y = input_slice.count('\n')
		console.cursor_x = (index - input_slice.rindex('\n') - 1) if console.cursor_y else index
	
	def console__clear():
		persistent.console_commands = []
		persistent.console_text = 'Press Esc to exit, type help for help'
	
	if persistent.console_commands is None or not persistent.console_text:
		console__clear()
	
	
	def console__out(text):
		persistent.console_text += '\n' + str(text).replace('{', '{{')
	def console__out_help():
		to_out = 'commands: clear, jump, call, scene, show, hide, watch <expr>, unwatch <expr>, unwatchall or python-expr'
		console.out(to_out)
	
	
	def console__add(s):
		if hotkeys.ctrl:
			if s == 'c':
				set_clipboard_text(console.input)
				return
			if s == 'v':
				s = get_clipboard_text()
				s = s.replace('\t', '    ')
				lines = s.split('\n')
				i = 0
				while i < len(lines):
					if not lines[i] or lines[i].isspace():
						lines.pop(i)
					else:
						i += 1
				s = '\n'.join(lines)
			if s == 'd':
				lines = console.input.split('\n')
				lines = lines[0:console.cursor_y] + lines[console.cursor_y+1:]
				console.cursor_x = 0
				console.cursor_y = min(console.cursor_y, len(lines) - 1)
				console.input = '\n'.join(lines)
				return
		
		if hotkeys.shift and s in console.keys:
			s = console.keys_shift[console.keys.index(s)]
		if s == '{':
			s = console.key_tag
		
		index = console.get_cursor_index()
		console.input = console.input[0:index] + s + console.input[index:]
		index += len(s)
		console.set_cursor_index(index)
		
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
		cur = lines[console.cursor_y]
		console.cursor_x = len(cur)
	
	
	def console__cursor_left():
		index = console.get_cursor_index()
		if index == 0:
			return
		index -= 1
		symbol = console.input[index]
		check = console.get_check(symbol)
		if hotkeys.ctrl:
			if symbol == ' ' and index and console.input[index - 1] == ' ':
				while index and console.input[index - 1] == ' ':
					index -= 1
			else:
				while index and check(console.input[index - 1]):
					index -= 1
		console.set_cursor_index(index)
	def console__cursor_right():
		index = console.get_cursor_index()
		if index == len(console.input):
			return
		symbol = console.input[index]
		check = console.get_check(symbol)
		index += 1
		if hotkeys.ctrl:
			while index < len(console.input) and console.input[index] == ' ':
				index += 1
			while index < len(console.input) and check(console.input[index]):
				index += 1
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
		console.input = console.input[0:index] + console.input[start_index:]
		if not console.num_command:
			console.input_tmp = console.input
	def console__delete():
		start_index = console.get_cursor_index()
		console.cursor_right()
		index = console.get_cursor_index()
		console.input = console.input[0:start_index] + console.input[index:]
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
			console.execute(to_exec.replace(console.key_tag, '{'))
		else:
			index = console.get_cursor_index()
			console.input = console.input[0:index] + '\n' + ' ' * cur_indent + console.input[index:]
			console.cursor_y += 1
			console.cursor_x = cur_indent
	
	def console__except_error(to_exec):
		try:
			res = to_exec()
		except Exception as e:
			if isinstance(e, SyntaxError):
				file_name, line_num, symbol_num, error_line = e.args[1]
				msg = 'File <%s>, line %s: invalid syntax' % (file_name, line_num)
				if error_line:
					msg += '\n  ' + str(error_line)
					if symbol_num:
						msg += '\n  ' + ' ' * (symbol_num - 1) + '^'
				console.out(msg)
			else:
				console.out(e.__class__.__name__ + ': ' + str(e.args[0]))
			res = None
		return res
	
	def console__execute(command):
		command = command.strip()
		if not command:
			return
		
		lines = command.split('\n')
		for i in xrange(len(lines)):
			pre = '... ' if i else '>>> '
			console.out(pre + lines[i])
		
		if ' ' in command:
			index = command.index(' ')
			params = command[index+1:]
			command = command[0:index]
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
		try:
			eval_obj = Eval(code, 'Console', 1)
		except Exception as e:
			console.out('Compilation Failed: ' + str(e))
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
	
	
	def console__is_spec(s):
		return s in console.spec_symbols
	def console__is_not_spec(s):
		return s != ' ' and s not in console.spec_symbols
	def console__get_check(s):
		return console.is_spec if s in console.spec_symbols else console.is_not_spec
	
	
	def console__show():
		if not has_screen('console'):
			console.showed_time = get_game_time()
			show_screen('console')
	
	
	build_object('console')
	
	console.to_watch = []
	
	console.background = im.rect('#000')
	console.background_alpha = 0.15
	
	console.input = ''
	console.input_tmp = ''
	
	console.keys = alphabet + list("1234567890-=[]\\;',./`")
	console.keys_shift = [s.upper() for s in alphabet] + list("!@#$%^&*()_+{}|:\"<>?~")
	console.key_tag = chr(255)
	
	console.spec_symbols = console.keys[console.keys.index('-'):] + console.keys_shift[console.keys_shift.index('!'):] + [console.key_tag]
	
	console.cursor = '{color=#FFFFFF}|{/color}'
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
		
		python:
			console.watching_xsize_max_real = get_absolute(console.watching_xsize_max, get_stage_width()) - console.watching_xindent * 2
			
			console.watching_text = []
			console.watching_xsize = 0
			console.watching_ysize = 0
			for code, eval_obj in console.to_watch:
				res_start = code + ': '
				try:
					res_end = str(eval_obj()).replace('{', '{{')
				except:
					res_end = 'Eval Failed'
				console.watching_text.append(res_start + '{color=' + console.watching_calced_color + '}' + res_end + '{/color}')
				console.watching_xsize = max(console.watching_xsize, utf8.width(res_start + res_end, style.console_text.text_size))
				if console.watching_xsize > 0:
					console.watching_ysize += int(math.ceil(console.watching_xsize / console.watching_xsize_max_real)) * style.console_text.text_size
			
			console.watching_xsize = min(console.watching_xsize, console.watching_xsize_max_real)
		
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
	
	$ db.skip_tab = False
	
	key 'ESCAPE' action [Hide('console'), SetDict(pause_screen, 'hided_time', get_game_time())]
	
	key 'SPACE' action console.add(' ')
	
	if get_game_time() - console.showed_time > 0.333:
		for key in console.keys:
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
		console.output_lines = console.output_height / style.console_text.text_size
		
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
			
			python:
				index = console.get_cursor_index()
				alpha_cursor = '{alpha=' + str(1 if get_game_time() % 2 < 1 else 0) + '}' + console.cursor + '{/alpha}'
				console.input_with_cursor = console.input[0:index] + alpha_cursor + console.input[index:]
			
			text console.input_with_cursor.replace(console.key_tag, '{{'):
				style 'console_text'
				xsize get_stage_width() - 50

