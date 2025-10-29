init -1000 python:
	def code_coloring__prepare_indents(text):
		lines = text.lstrip('\n').rstrip().split('\n')
		
		indent = 0
		for c in lines[0]:
			if c == ' ':
				indent += 1
			else:
				break
		
		return '\n'.join([line[indent:] for line in lines])
	
	def code_coloring__get_tokens(text):
		text_len = len(text)
		allowed_for_word = alphabet.upper() + alphabet + numbers + '_.'
		def make_step(index):
			if index >= text_len:
				return index
			
			first = text[index]
			index += 1
			
			if first in '"\'':
				# skip to end quote
				prev = first
				while index < text_len:
					if text[index] == '\\':
						index += 2
						continue
					if text[index] == first:
						index += 1
						break
					index += 1
			
			elif first not in code_coloring.operators:
				if first == '#':
					while index < text_len:
						if text[index] == '\n':
							break
						index += 1
				else:
					# skip to disallowed symbol in id/cmd
					while index < text_len:
						if text[index] not in allowed_for_word:
							break
						index += 1
			
			
			# skip spaces
			while index < text_len:
				if not text[index].isspace():
					break
				index += 1
			
			return index
		
		res = []
		index = 0
		while True:
			prev_index = index
			index = make_step(index)
			if prev_index == index:
				break
			res.append(text[prev_index:index])
		return res
	
	def code_coloring__colorize(text):
		cache = code_coloring__colorize.__dict__
		res = cache.get(text)
		if res is not None:
			return res
		
		text = code_coloring.prepare_indents(text)
		tokens = code_coloring.get_tokens(text)
		
		res = ''
		for token in tokens:
			clean_token = token.strip()
			
			if clean_token in code_coloring.keywords:
				token_type = 'keyword'
			elif clean_token in code_coloring.operators:
				token_type = 'operator'
			else:
				first = token[0]
				if first in '"\'':
					token_type = 'quote'
				elif first.isdigit():
					token_type = 'number'
				elif first == '#':
					token_type = 'comment'
				else:
					token_type = None
			
			if token_type:
				res += '{color=%s}%s{/color}' % (code_coloring.colors[token_type], token)
			else:
				res += token
		
		cache[text] = res
		return res
	
	code_coloring = SimpleObject()
	build_object('code_coloring')
	
	
	code_coloring.operators = '+-*/%=!><()[]{}:'
	
	code_coloring.keywords = set('''
		$
		False
		None
		True
		abs
		absolute
		action
		activate_sound
		align
		all
		alpha
		alternate
		anchor
		and
		any
		as
		assert
		at
		behind
		block
		bold
		bool
		break
		button
		call
		callable
		chr
		class
		classmethod
		clipping
		color
		compile
		complex
		contains
		continue
		corner_sizes
		crop
		def
		del
		delattr
		delay
		dict
		dir
		divmod
		elif
		else
		enumerate
		eval
		except
		exec
		expression
		fadein
		fadeout
		filter
		finally
		first_delay
		float
		font
		for
		from
		frozenset
		getattr
		global
		globals
		ground
		has
		hasattr
		hash
		hbox
		hex
		hide
		hotspot
		hover
		hover_bold
		hover_color
		hover_font
		hover_italic
		hover_outlinecolor
		hover_sound
		hover_strikethrough
		hover_text_align
		hover_text_size
		hover_text_valign
		hover_underline
		hovered
		id
		if
		image
		imagebutton
		imagemap
		import
		in
		init
		int
		is
		isinstance
		issubclass
		italic
		iter
		jump
		key
		label
		lambda
		len
		list
		locals
		map
		max
		menu
		min
		modal
		mouse
		not
		null
		nvl
		object
		oct
		open
		or
		ord
		outlinecolor
		parallel
		pass
		pause
		play
		pos
		pow
		print
		property
		python
		queue
		raise
		range
		repeat
		repr
		return
		reversed
		rotate
		round
		save
		scene
		screen
		selected
		self
		set
		setattr
		show
		size
		size_max
		size_min
		skip_mouse
		slice
		sorted
		spacing
		spacing_max
		spacing_min
		staticmethod
		stop
		str
		strikethrough
		style
		sum
		super
		text
		text_align
		text_size
		text_size_max
		text_size_min
		text_valign
		textbutton
		transform
		translate
		try
		tuple
		type
		underline
		unhovered
		use
		vars
		vbox
		while
		window
		with
		xalign
		xanchor
		xpos
		xsize
		xsize_max
		xsize_min
		xzoom
		yalign
		yanchor
		yield
		ypos
		ysize
		ysize_max
		ysize_min
		yzoom
		zip
		zoom
		zorder
	'''.replace(' ', '').strip().split('\n'))
	
	code_coloring.colors = {
		'keyword':  '#FFCF48',
		'operator': '#FF7514',
		'quote':    '#7D7',
		'number':   '#7FC7FF',
		'comment':  '#DCD0FF',
	}
