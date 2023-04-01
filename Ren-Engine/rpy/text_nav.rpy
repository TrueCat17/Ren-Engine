init python:
	
	def text_nav__add(text, x, y, s, index, ctrl, shift, paste_processor = None):
		if ctrl:
			if s == 'd':
				lines = text.split('\n')
				lines = lines[0:y] + lines[y+1:]
				y = min(y, len(lines) - 1)
				x = len(lines[y]) if lines else 0
				text = '\n'.join(lines)
				return text, x, y, 0
			if s == 'c':
				set_clipboard_text(text.replace(text_nav.key_tag, '{'))
				return text, x, y, 0
			if s == 'v':
				s = get_clipboard_text()
				s = s.replace('\t', '    ')
				if paste_processor:
					s = paste_processor(s)
		else:
			if shift and s in text_nav.keys:
				s = text_nav.keys_shift[text_nav.keys.index(s)]
		
		s = s.replace('{', text_nav.key_tag)
		return text[:index] + s + text[index:], x, y, len(s)
	
	
	def text_nav__cursor_left(text, index, ctrl):
		if index == 0:
			return index
		index -= 1
		symbol = text[index]
		if ctrl:
			check = text_nav.get_check(symbol)
			while index and check(text[index - 1]):
				index -= 1
		while not utf8.is_first_byte(text[index]):
			index -= 1
		return index
	
	def text_nav__cursor_right(text, index, ctrl):
		if index == len(text):
			return index
		symbol = text[index]
		index += 1
		if hotkeys.ctrl:
			check = text_nav.get_check(symbol)
			while index < len(text) and check(text[index]):
				index += 1
		while index < len(text) and not utf8.is_first_byte(text[index]):
			index += 1
		return index
	
	
	def text_nav__is_space(s):
		return s == ' '
	def text_nav__is_spec(s):
		return s in text_nav.spec_symbols
	def text_nav__is_not_spec(s):
		return s != ' ' and s not in text_nav.spec_symbols
	def text_nav__get_check(s):
		if s == ' ':
			return text_nav.is_space
		if s in text_nav.spec_symbols:
			return text_nav.is_spec
		return text_nav.is_not_spec
	
	
	
	build_object('text_nav')
	
	text_nav.keys = alphabet + list("1234567890-=[]\\;',./`")
	text_nav.keys_shift = [s.upper() for s in alphabet] + list('!@#$%^&*()_+{}|:"<>?~')
	text_nav.key_tag = chr(255)
	
	text_nav.spec_symbols = text_nav.keys[text_nav.keys.index('-'):] + text_nav.keys_shift[text_nav.keys_shift.index('!'):] + [text_nav.key_tag]
	
