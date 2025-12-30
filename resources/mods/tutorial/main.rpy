init python:
	config.has_autosave = False
	
	rn_moving = easein
	
	# selected text: start and end
	sts = '{color=#0B4}'
	ste = '{/color}'
	
	def text_code(code):
		return sts + code.replace(' ', chr(160)) + ste # space -> no-break-space
	
	def in_tags(expr, real_tags = True):
		res = ''
		
		for text in expr.split('|'):
			if '=' in text:
				tag = text[:text.index('=')]
			else:
				tag = text
			
			if tag.startswith('/'):
				tag = tag[1:]
				is_end = True
			else:
				is_end = False
			
			is_tag = tag in ('i', 'b', 'u', 's', 'plain', 'color', 'outlinecolor', 'font', 'size', 'alpha', 'image')
			
			if not is_tag:
				res += text.replace('{', '{{')
				continue
			
			text = '{%s}' % text
			if is_end and real_tags:
				res += text
			res += sts + text.replace('{', '{{') + ste
			if not is_end and real_tags:
				res += text
		
		return res


label start:
	window hide
	scene
	$ lessons.cur_label = None
	$ tutorial_menu.start(from_middle = True)
	
	$ code.hide()
	$ current_part.hide()
	
	while True:
		pause 1
