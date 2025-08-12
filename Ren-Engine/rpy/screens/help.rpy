init -500 python:
	
	def help__set_file(path_dir = None, path_file = 'readme.txt', make_vars = dict):
		if not path_dir:
			path_dir = 'mods/' + get_current_mod()
		help.path_dir = make_sure_dir(path_dir)
		
		help.path_file = path_file
		help.make_vars = make_vars
	
	
	def help__init():
		with open(help.path_dir + _(help.path_file), 'rb') as f:
			content = f.read().decode('utf-8')
			content = content.strip().replace('\\\n', '').replace('\t', '  ')
		
		help.groups = []
		groups = content.split('\n\n\n')
		data = help.make_vars()
		for group in groups:
			group = group.strip()
			if group.startswith('['):
				i = group.index('\n') if '\n' in group else 0
				name = group[:i]
				if name.startswith('[') and name.endswith(']'):
					name = name[1:-1]
				text = group[i+1:] + '\n'
			else:
				name = ''
				text = group + '\n'
			
			end = -1
			while True:
				start = text.find('${', end + 1)
				if start == -1: break
				
				end = text.find('}', start, text.find('\n', start))
				if end == -1:
					out_msg('help.init, section %s' % name, 'Close-tag not found')
					break
				
				var_name = text[start + 2 : end]
				if var_name in data:
					text = text[:start] + data[var_name] + text[end + 1:]
					end += len(data[var_name]) - (end + 1 - start)
				else:
					out_msg('help.init, section %s' % name, 'var name <%s> is undefined', var_name)
					break
			
			help.groups.append((name, text.strip()))
		
		help.on_resize()
	
	
	def help__on_resize():
		if not help.groups: # not inited
			return
		
		sw, sh = get_stage_size()
		
		help.button_width = 0
		text_size = style.help_button.get_current('text_size')
		for name, text in help.groups:
			width = get_text_width(name, text_size) + 10
			help.button_width = max(help.button_width, width)
		
		arrow_size = help.button_spacing * 2 + style.help_button.get_current('ysize')
		max_width = get_absolute(help.width, sw) - arrow_size * 2
		help.button_count = min(len(help.groups), int(max_width / (help.button_width + help.button_spacing)))
		
		help.button_start_index = 0
		help.button_max_index = len(help.groups) - help.button_count
		
		
		help._xsize = get_absolute(help.width,  sw)
		help._ysize = get_absolute(help.height, sh)
		
		help._xindent = get_absolute(help.xindent, sw)
		help._indent = get_absolute(help.indent, sh)
		help._indent_for_buttons = get_absolute(help.indent_for_buttons, sh)
		
		help._viewport_xsize = help._xsize - help._xindent * 3 - help.slider_width
		help._viewport_ysize = help._ysize - help._indent - help._indent_for_buttons
		
		help.select_group(help.group_index)
	
	def help__on_show(screen_name):
		if screen_name == 'help':
			help.init()
	
	def help__select_group(index):
		help.group_index = index
		help.text = help.groups[index][1]
		
		strs = help.text.split('\n')
		text_size = style.help_text.get_current('text_size')
		count = 0
		for s in strs:
			width = get_text_width(s, text_size)
			count += math.ceil(width / help._viewport_xsize)
		height = (count + 3) * text_size
		slider_size = max(help._viewport_ysize / height, 0.1)
		slider_v.change('help', slider_size = slider_size)
	
	
	build_object('help')
	help.set_file()
	help.group_index = 0
	
	signals.add('show_screen', help.on_show)
	signals.add('resized_stage', help.on_resize)
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('help')
	
	
	help.width = 0.8
	help.height = 0.8
	
	# only in pixels
	help.border_size = 5
	help.button_spacing = 15
	help.slider_width = 25
	
	help.background_image = im.rect('#FFF')
	help.border_image = im.rect('#000')
	
	# None - auto
	help.slider_ground = None
	help.slider_hover  = None
	help.slider_button_style = None
	
	# only relative sizes here
	help.indent = 0.03
	help.xindent = help.indent / get_from_hard_config('window_w_div_h', float)
	help.indent_for_buttons = 0.1
	help.viewport_height = help.height - help.indent_for_buttons - help.indent
	
	def help__init_slider():
		slider_v.init(
			'help', help.viewport_height,
			ground = help.slider_ground,
			hover  = help.slider_hover,
			button_size  = help.slider_width,
			button_style = help.slider_button_style
		)
	signals.add('inited', help__init_slider, times = 1)

init:
	style help_background_button is button:
		corner_sizes 0
		ground im.rect('#000')
		hover  im.rect('#000')
		size 1.0
		alpha 0.3
		mouse False
	
	style help_button is menu_button
	
	style help_text is text:
		color 0


screen help:
	modal True
	zorder 1000001
	
	key 'ESCAPE' action hide_screen('help')
	button:
		style 'help_background_button'
		action    hide_screen('help')
		alternate hide_screen('help')
	
	image help.border_image:
		align 0.5
		xsize help._xsize + help.border_size * 2
		ysize help._ysize + help.border_size * 2
		corner_sizes help.border_size
	
	image help.background_image:
		align 0.5
		xsize help.width
		ysize help.height
		
		hbox:
			spacing help.button_spacing
			xalign 0.5
			xsize help.width
			ysize help.indent_for_buttons
			
			textbutton '←':
				style 'help_button'
				yalign 0.5
				xsize  style.help_button.get_current('ysize')
				alpha  1 if help.button_start_index != 0 else 0
				action 'help.button_start_index = max(0, help.button_start_index - 1)'
			
			for i in range(help.button_start_index, help.button_start_index + help.button_count):
				$ name = help.groups[i][0]
				textbutton name:
					style 'help_button'
					yalign 0.5
					xsize  help.button_width
					alpha  1 if name or len(help.groups) > 1 else 0
					action [help.select_group(i), slider_v.set_value('help', 0)]
			
			textbutton '→':
				style 'help_button'
				yalign 0.5
				xsize  style.help_button.get_current('ysize')
				alpha  1 if help.button_start_index != help.button_max_index else 0
				action 'help.button_start_index = min(help.button_start_index + 1, help.button_max_index)'
		
		null:
			xpos help._xindent
			ypos help._indent_for_buttons
			
			xsize help._viewport_xsize
			ysize help._viewport_ysize
			
			null:
				clipping True
				
				xsize help._viewport_xsize
				ysize help._viewport_ysize
				
				text help.text:
					style 'help_text'
					xsize help._viewport_xsize
					ysize_min help._viewport_ysize
					yalign slider_v.get_value('help')
			
			null:
				xpos help._xindent + help._viewport_xsize
				use slider_v('help')
