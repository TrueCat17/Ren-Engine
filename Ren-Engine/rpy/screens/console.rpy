init python:
	
	console_font = 'Consola'
	console_text_color = 0xFFFFFF
	console_text_outlinecolor = 0
	
	console_background = im.rect('#000')
	console_background_alpha = 0.15
	
	console_text_size = 20
	
	console_input = ''
	console_input_tmp = ''
	
	console_keys = alphabet + list("1234567890-=[]\\;',./`")
	console_keys_shift = [s.upper() for s in alphabet] + list("!@#$%^&*()_+{}|:\"<>?~")
	console_key_tag = chr(255)
	
	console_spec_symbols = console_keys[console_keys.index('-'):] + console_keys_shift[console_keys_shift.index('!'):] + [console_key_tag]
	
	console_is_spec     = lambda s: s in console_spec_symbols
	console_is_not_spec = lambda s: s != ' ' and s not in console_spec_symbols
	console_get_check   = lambda s: console_is_spec if s in console_spec_symbols else console_is_not_spec
	
	console_cursor = '{color=#FFFFFF}|{/color}'
	console_cursor_x = 0
	console_cursor_y = 0
	console_num_command = 0
	
	console_ctrl = False
	console_shift = False
	
	
	def console_get_cursor_index():
		lines = console_input.split('\n')
		index = min(console_cursor_x, len(lines[console_cursor_y]))
		for y in xrange(console_cursor_y):
			index += len(lines[y]) + 1
		return index
	def console_set_cursor_index(index):
		global console_cursor_x, console_cursor_y
		input_slice = console_input[0:index]
		console_cursor_y = input_slice.count('\n')
		console_cursor_x = (index - input_slice.rindex('\n') - 1) if console_cursor_y else index
	
	def console_clear():
		persistent.console_commands = []
		persistent.console_text = 'Press Esc to exit, type help for help'
	
	if not persistent.console_commands or not persistent.console_text:
		console_clear()
	
	
	def console_print(text):
		persistent.console_text += '\n' + str(text).replace('{', '{{')
	def console_print_help():
		to_print = 'commands: clear, jump, call, scene, show, hide, watch <expr>, unwatch <expr>, unwatchall or python-expr'
		console_print(to_print)
	
	
	def console_add(s):
		global console_cursor_x, console_cursor_y, console_input, console_input_tmp
		if console_ctrl and s == 'd':
			lines = console_input.split('\n')
			lines = lines[0:console_cursor_y] + lines[console_cursor_y+1:]
			console_cursor_x = 0
			console_cursor_y = min(console_cursor_y, len(lines) - 1)
			console_input = '\n'.join(lines)
		else:
			if console_shift and not s.isspace():
				s = console_keys_shift[console_keys.index(s)]
			if s == '{':
				s = console_key_tag
			
			index = console_get_cursor_index()
			console_input = console_input[0:index] + s + console_input[index:]
			console_cursor_right()
		
		if not console_num_command:
			console_input_tmp = console_input
	
	def console_cursor_home():
		global console_cursor_x, console_cursor_y
		console_cursor_x = 0
		if console_ctrl:
			console_cursor_y = 0
	def console_cursor_end():
		global console_cursor_x, console_cursor_y
		lines = console_input.split('\n')
		if console_ctrl:
			console_cursor_y = len(lines) - 1
		cur = lines[console_cursor_y]
		console_cursor_x = len(cur)
	
	
	def console_cursor_left():
		index = console_get_cursor_index()
		if index == 0:
			return
		index -= 1
		symbol = console_input[index]
		check = console_get_check(symbol)
		if console_ctrl:
			if symbol == ' ' and index and console_input[index - 1] == ' ':
				while index and console_input[index - 1] == ' ':
					index -= 1
			else:
				while index and check(console_input[index - 1]):
					index -= 1
		console_set_cursor_index(index)
	def console_cursor_right():
		index = console_get_cursor_index()
		if index == len(console_input):
			return
		symbol = console_input[index]
		check = console_get_check(symbol)
		index += 1
		if console_ctrl:
			while index < len(console_input) and console_input[index] == ' ':
				index += 1
			while index < len(console_input) and check(console_input[index]):
				index += 1
		console_set_cursor_index(index)
	def console_cursor_up():
		global console_cursor_x, console_cursor_y, console_input, console_num_command
		if console_cursor_y > 0:
			console_cursor_y -= 1
		elif console_cursor_y == 0 and (console_cursor_x > 0 and console_input):
			console_cursor_x = 0
		elif console_num_command < len(persistent.console_commands):
			console_num_command += 1
			console_input = persistent.console_commands[-console_num_command]
			lines = console_input.split('\n')
			console_cursor_y = len(lines) - 1
			console_cursor_x = len(lines[-1])
	def console_cursor_down():
		global console_cursor_x, console_cursor_y, console_input, console_num_command
		lines = console_input.split('\n')
		if console_cursor_y < len(lines) - 1:
			console_cursor_y += 1
		elif console_cursor_y == len(lines) - 1 and console_cursor_x < len(lines[-1]):
			console_cursor_x = len(lines[-1])
		elif console_num_command > 1:
			console_num_command -= 1
			console_input = persistent.console_commands[-console_num_command]
			console_cursor_y = 0
			console_cursor_x = console_input.index('\n') if '\n' in console_input else len(console_input)
		else:
			console_num_command = 0
			console_input = console_input_tmp
	
	def console_backspace():
		global console_input, console_input_tmp
		start_index = console_get_cursor_index()
		console_cursor_left()
		index = console_get_cursor_index()
		console_input = console_input[0:index] + console_input[start_index:]
		if not console_num_command:
			console_input_tmp = console_input
	def console_delete():
		global console_input, console_input_tmp
		start_index = console_get_cursor_index()
		console_cursor_right()
		index = console_get_cursor_index()
		console_input = console_input[0:start_index] + console_input[index:]
		console_set_cursor_index(start_index)
		if not console_num_command:
			console_input_tmp = console_input
	
	def console_return():
		global console_cursor_x, console_cursor_y, console_input, console_input_tmp, console_num_command
		if console_input.isspace():
			console_input = ''
			return
		
		lines = console_input.split('\n')
		cur = lines[console_cursor_y]
		
		cur_indent = 0
		while cur_indent < len(cur) and cur[cur_indent] == ' ':
			cur_indent += 1
		
		if cur.endswith(':'):
			lines.insert(console_cursor_y + 1, ' ' * (cur_indent + 4))
			console_cursor_y += 1
			console_cursor_x = cur_indent + 4
			console_input = '\n'.join(lines)
		elif console_cursor_y == len(lines) - 1 and (not cur_indent or cur.isspace()):
			console_cursor_x = 0
			console_cursor_y = 0
			console_num_command = 0
			to_exec = console_input
			console_input = console_input_tmp = ''
			if not persistent.console_commands or persistent.console_commands[-1] != to_exec:
				persistent.console_commands += [to_exec]
			console_exec(to_exec.replace(console_key_tag, '{'))
		else:
			index = console_get_cursor_index()
			console_input = console_input[0:index] + '\n' + ' ' * cur_indent + console_input[index:]
			console_cursor_y += 1
			console_cursor_x = cur_indent
	
	def console_except_error(to_exec):
		try:
			res = to_exec()
		except Exception as e:
			desc = e.args[0]
			if len(e.args) > 1:
				file_name, line_num, symbol_num, error_line = e.args[1]
				console_print('File <' + file_name + '>, line ' + str(line_num) + ': ' + desc + '\n' +
					          '  ' + error_line + '\n' +
					          '  ' + ' ' * (symbol_num - 1) + '^')
			else:
				console_print("KeyError: " + str(desc))
			res = None
		return res
	
	def console_exec(command):
		command = command.strip()
		if not command:
			return
		
		lines = command.split('\n')
		for i in xrange(len(lines)):
			pre = '... ' if i else '>>> '
			console_print(pre + lines[i])
		
		if ' ' in command:
			index = command.index(' ')
			params = command[index+1:]
			command = command[0:index]
		else:
			params = ''
		
		if command == 'exit':
			hide_screen('console')
		elif command == 'clear':
			console_clear()
		elif command == 'help':
			console_print_help()
		
		elif command == 'watch':
			console_watch_add(params)
		elif command == 'unwatch':
			console_watch_del(params)
		elif command == 'unwatchall':
			console_watch_clear()
		
		elif command == 'jump':
			renpy.jump(params)
		elif command == 'call':
			renpy.call(params)
		
		elif command in ('scene', 'show', 'hide'):
			args = get_args(params)
			
			if command == 'scene':
				set_scene(args, [])
			elif command == 'show':
				show_sprite(args, [])
			elif command == 'hide':
				hide_sprite(args)
		
		else:
			code = command + ' ' + params
			to_exec = None
			try:
				cmpl = compile(code, 'Console', 'eval')
				to_exec = Function(eval, cmpl, globals(), globals())
			except:
				to_exec_for_compile = Function(compile, code, 'Console', 'exec')
				cmpl = console_except_error(to_exec_for_compile)
				if cmpl is not None:
					to_exec = Function(eval, cmpl, globals(), globals())
			
			if to_exec is not None:
				res = console_except_error(to_exec)
				if res is not None:
					if type(res) is str:
						res = repr(res)
					console_print(res)
	
	
	console_to_watch = []
	def console_watch_add(code):
		try:
			cmpl = compile(code, 'Console', 'eval')
		except Exception as e:
			console_print('Compilation Failed: ' + str(e))
		
		if not console_to_watch:
			show_screen('console_watching')
		console_to_watch.append((code, cmpl))
	
	def console_watch_del(code_to_del):
		for code, cmpl in console_to_watch:
			if code == code_to_del:
				console_to_watch.remove((code, cmpl))
				if not console_to_watch:
					hide_screen('console_watching')
				break
		else:
			console_print('<' + code_to_del + '> not watched')
	
	def console_watch_clear():
		global console_to_watch
		console_to_watch = []
		
		if has_screen('console_watching'):
			hide_screen('console_watching')
	
	console_showed_time = 0
	def show_console():
		global console_showed_time
		if not has_screen('console'):
			show_screen('console')
			console_showed_time = time.time()


screen console_watching:
	zorder 10001
	
	image im.rect('#0006'):
		align (1.0, 0.0)
		size  (0.4, 0.3)
		
		python:
			console_watching_text = ''
			for code, cmpl in console_to_watch:
				try:
					res = str(eval(cmpl)).replace('{', '{{')
				except:
					res = 'Eval Failed'
				console_watching_text += code + ': ' + res + '\n'
		
		text console_watching_text:
			align (0.5, 0.5)
			size  (0.35, 0.25)
			text_size 20


screen console:
	modal True
	zorder 10002
	
	$ db_skip_tab = False
	
	key 'ESCAPE' action [Hide('console'), SetVariable('pause_hided_time', time.time())]
	
	$ console_ctrl  = False
	key 'LEFT CTRL'   action SetVariable('console_ctrl', True) first_delay 0
	key 'RIGHT CTRL'  action SetVariable('console_ctrl', True) first_delay 0
	$ console_shift = False
	key 'LEFT SHIFT'  action SetVariable('console_shift', True) first_delay 0
	key 'RIGHT SHIFT' action SetVariable('console_shift', True) first_delay 0
	
	key 'SPACE' action console_add(' ')
	
	if time.time() - console_showed_time > 0.333:
		for key in console_keys:
			key key action console_add(key)
	
	key 'HOME'      action console_cursor_home
	key 'END'       action console_cursor_end
	
	key 'LEFT'      action console_cursor_left
	key 'RIGHT'     action console_cursor_right
	key 'UP'        action console_cursor_up
	key 'DOWN'      action console_cursor_down
	
	key 'BACKSPACE' action console_backspace
	key 'DELETE'    action console_delete
	
	key 'RETURN'    action console_return
	
	
	python:
		console_input_height = console_text_size * console_input.count('\n')
		
		console_output_height = get_stage_height() - console_input_height
		console_output_lines = console_output_height / console_text_size
		
		console_output = '\n'.join(persistent.console_text.split('\n')[-console_output_lines:])
	
	image console_background:
		alpha console_background_alpha
		size (1.0, 1.0)
	
	vbox:
		align (0.5, 1.0)
		
		hbox:
			null xsize 50
			
			text console_output:
				font         console_font
				color        console_text_color
				outlinecolor console_text_outlinecolor
				text_size    console_text_size
				xsize        get_stage_width() - 50
		
		hbox:
			text ('...' if '\n' in console_input else '>>>'):
				font         console_font
				color        console_text_color
				outlinecolor console_text_outlinecolor
				text_size    console_text_size
				xsize        50
			
			python:
				index = console_get_cursor_index()
				alpha_cursor = '{alpha=' + str(1 if time.time() % 2 < 1 else 0) + '}' + console_cursor + '{/alpha}'
				console_input_with_cursor = console_input[0:index] + alpha_cursor + console_input[index:]
			
			text console_input_with_cursor.replace(console_key_tag, '{{'):
				font         console_font
				color        console_text_color
				outlinecolor console_text_outlinecolor
				text_size    console_text_size
				xsize     get_stage_width() - 50

